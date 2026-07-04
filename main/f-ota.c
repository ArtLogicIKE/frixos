#include "frixos.h"
#include "f-ota.h"
#include "f-display.h"
#include "f-wifi.h"
#include "f-membuffer.h"
#include "f-integrations.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_spiffs.h"
#include "esp_http_client.h"
#include "esp_task_wdt.h"
#include "esp_app_format.h"

#include "cJSON.h"
#include <dirent.h>
#include <sys/stat.h>

#include "f-manifest.h"

static const char *TAG = "f-ota";
static update_progress_callback_t progress_callback = NULL;
static time_t last_check_time = 0;

// OTA update thread control
TaskHandle_t ota_update_task_handle = NULL;
SemaphoreHandle_t ota_update_semaphore = NULL;
static volatile bool ota_reinstall_mode = false;

// Internal function declarations
static esp_err_t download_file(const char *url, const char *dest_path, int *progress,
                               const uint8_t *expected_sha256, uint32_t expected_size);
static void update_progress(int progress, const char *message);
static void ota_update_task(void *pvParameters);
static void log_partition_info(const esp_partition_t *partition, const char *prefix);
static void ota_handle_failure(const char *error_msg, update_status_t status, bool release_mutex);
static void ota_handle_failure_with_cleanup(const char *error_msg, update_status_t status, esp_http_client_handle_t client, esp_ota_handle_t ota_handle, bool release_mutex);
static void cleanup_update_files(void);
static void f_ota_do_update(int version);
static esp_err_t ota_fetch_manifest(int version, manifest_t *m);
static esp_err_t ota_reconcile_files(const manifest_t *m, int progress_lo, int progress_hi, bool show_progress);
static void ota_self_heal(void);
static void ota_quiet_refresh(void);

// Add this function before f_ota_init
static void log_partition_info(const esp_partition_t *partition, const char *prefix)
{
    if (partition == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "%s: NULL partition", prefix);
        return;
    }
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "%s: label=%s, type=%d, subtype=%d, address=0x%lx, size=0x%lx",
                prefix, partition->label, partition->type, partition->subtype,
                partition->address, partition->size);
}

void f_ota_verify(void)
{
    last_check_time = 0;
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Initializing OTA, checking partitions");

    // Get all relevant partitions
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *boot = esp_ota_get_boot_partition();
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

    // Log detailed partition information
    log_partition_info(running, "Running");
    log_partition_info(boot, "Boot");
    log_partition_info(next, "Next");

    // Get otadata partition
    const esp_partition_t *otadata = esp_partition_find_first(ESP_PARTITION_TYPE_DATA,
                                                              ESP_PARTITION_SUBTYPE_DATA_OTA,
                                                              NULL);
    log_partition_info(otadata, "OTAData");

    esp_ota_img_states_t ota_state;
    esp_ota_get_state_partition(running, &ota_state);

    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA image is pending verification");
        // Confirm the app is okay
        esp_ota_mark_app_valid_cancel_rollback();
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA image verified");
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA image already verified");
    }
}

void f_ota_set_progress_callback(update_progress_callback_t callback)
{
    progress_callback = callback;
}

static void update_progress(int progress, const char *message)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Update progress: %d%%, message: %s", progress, message);
    if (progress_callback)
    {
        progress_callback(progress, message);
    }
}

// Unified failure handling function
static void ota_handle_failure(const char *error_msg, update_status_t status, bool release_mutex)
{
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA update failed: %s", error_msg);
    update_progress(100, "Update failed"); // Signal completion to restore normal message
    ota_update_in_progress = false;
    ota_reinstall_in_progress = false;
    ota_updating_message = false;
    ota_start_time = 0;

    // SPIFFS writes are over; restore the normal boot-loop rescue threshold.
    manifest_set_ota_in_progress(false);

    // Clean up any leftover .update files
    cleanup_update_files();

    // Give the HTTP mutex back only if we actually took it
    if (release_mutex)
    {
        xSemaphoreGive(http_mutex);
    }

    // Restore web server first so UI is back even if report below blocks (e.g. update server unreachable)
    extern void stop_webserver(void);
    extern esp_err_t start_webserver(void);

    stop_webserver();
    vTaskDelay(pdMS_TO_TICKS(500)); // Give time for cleanup
    start_webserver();

    // Small delay to allow any pending operations to complete
    vTaskDelay(pdMS_TO_TICKS(100));

    // Report status to update server after UI is restored (report may block on timeout)
    f_ota_report_status(status, error_msg);

    last_check_time = time(NULL) - (UPDATE_CHECK_INTERVAL / 2); // Reset to half interval ago so device can try again in half the time
}

// Unified failure handling function with cleanup
static void cleanup_update_files(void)
{
    DIR *dir;
    struct dirent *ent;

    dir = opendir("/spiffs");
    if (dir == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open SPIFFS directory for cleanup");
        return;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (strstr(ent->d_name, ".update") != NULL)
        {
            char filepath[256];
            snprintf(filepath, sizeof(filepath), "/spiffs/%.200s", ent->d_name);
            if (remove(filepath) == 0)
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Cleaned up update file: %s", ent->d_name);
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to remove update file: %s", ent->d_name);
            }
        }
    }

    closedir(dir);
}

static void ota_handle_failure_with_cleanup(const char *error_msg, update_status_t status, esp_http_client_handle_t client, esp_ota_handle_t ota_handle, bool release_mutex)
{
    if (client != NULL)
    {
        esp_http_client_cleanup(client);
    }
    if (ota_handle != 0)
    {
        esp_ota_abort(ota_handle);
    }

    // Clean up any leftover .update files
    cleanup_update_files();

    ota_handle_failure(error_msg, status, release_mutex);
}

/* Download url into dest_path via a temporary "<dest>.update" file.
 *
 * When expected_sha256 is non-NULL the payload is SHA-256-hashed while it
 * streams in and the temp file only replaces dest_path if size AND hash
 * match the manifest. Every write is checked: a failed fwrite/fclose (the
 * silent-corruption bug that produced truncated files in the field, 2026-07)
 * fails the download instead of installing a short file. */
static esp_err_t download_file(const char *url, const char *dest_path, int *progress,
                               const uint8_t *expected_sha256, uint32_t expected_size)
{
    // Create update path with .update extension
    char update_path[512];
    snprintf(update_path, sizeof(update_path), "%s.update", dest_path);

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        // Small buffers: we read in 1 KB chunks; large rx/tx buffers only
        // inflate peak heap and made client init fail in the low-heap boot
        // window (observed at 21 KB free on first-boot quiet refresh).
        .buffer_size = 2048,
        .buffer_size_tx = 512,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP client fetch headers failed");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    if (expected_size > 0 && (uint32_t)content_length != expected_size)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "%s: server sends %d bytes, manifest says %lu",
                    url, content_length, (unsigned long)expected_size);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    FILE *file = fopen(update_path, "wb");
    if (file == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open update file for writing: %s", update_path);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    manifest_hash_t sha_ctx;
    if (expected_sha256 && manifest_hash_begin(&sha_ctx) != ESP_OK)
    {
        fclose(file);
        remove(update_path);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    int total_read = 0;
    char *buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "download_file");
    int read_len;
    bool write_failed = false;

    while (total_read < content_length && (read_len = esp_http_client_read(client, buffer, 1024)) > 0)
    {
        if (fwrite(buffer, 1, read_len, file) != (size_t)read_len)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SPIFFS write failed at %d/%d bytes of %s",
                        total_read, content_length, update_path);
            write_failed = true;
            break;
        }
        if (expected_sha256)
        {
            manifest_hash_update(&sha_ctx, buffer, read_len);
        }
        total_read += read_len;
        taskYIELD();
        if (progress)
        {
            *progress = (total_read * 100) / content_length;
        }
    }
    release_shared_buffer(buffer);
    if (fclose(file) != 0)
    {
        // Buffered data may not have reached flash - the file cannot be trusted.
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SPIFFS close failed for %s", update_path);
        write_failed = true;
    }
    esp_http_client_cleanup(client);

    uint8_t actual_sha[MANIFEST_SHA256_LEN];
    bool sha_ok = true;
    if (expected_sha256)
    {
        sha_ok = (manifest_hash_end(&sha_ctx, actual_sha) == ESP_OK) &&
                 (memcmp(actual_sha, expected_sha256, MANIFEST_SHA256_LEN) == 0);
    }

    bool download_success = false;
    if (write_failed || total_read != content_length)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Download incomplete: %d of %d bytes", total_read, content_length);
        remove(update_path);
        sha_ok = false; // hash context result is meaningless for a partial payload
    }
    else if (!sha_ok)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Hash mismatch for %s - discarding download", dest_path);
        remove(update_path);
    }
    else
    {
        remove(dest_path); // Remove the original file
        // Download successful and verified, replace original file with update file
        if (rename(update_path, dest_path) == 0)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Successfully updated file: %s", dest_path);
            download_success = true;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to replace file %s with update", dest_path);
            // Clean up the update file
            remove(update_path);
        }
    }

    return download_success ? ESP_OK : ESP_FAIL;
}

/* Defer (don't fail) OTA work while heap is critically low - the boot storm
 * (weather + CGM TLS + geolocation) can leave <10 KB free for the first
 * minute, which starves HTTP client allocs and socket reads. The internal
 * esp_http_client rx/tx buffers cannot come from the f-membuffer pool (the
 * IDF API only takes sizes), so waiting is the reliable fix. */
static void ota_wait_for_heap(const char *who)
{
    const uint32_t needed = 35 * 1024;
    for (int i = 0; i < 18; i++) // up to 90 s
    {
        uint32_t free_heap = esp_get_free_heap_size();
        if (free_heap >= needed)
        {
            return;
        }
        if (i == 0)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "%s: heap low (%lu), waiting for boot tasks to settle",
                        who, (unsigned long)free_heap);
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "%s: proceeding despite low heap", who);
}

/* Make room on SPIFFS before writing `needed` bytes. SPIFFS garbage
 * collection otherwise runs synchronously inside fwrite when free pages run
 * out - a long stall mid-download that has watchdog-reset devices in the
 * field. Collecting deliberately between files keeps the stall bounded and
 * away from open sockets. */
static void ota_prepare_space(uint32_t needed)
{
    size_t total = 0, used = 0;
    if (esp_spiffs_info(NULL, &total, &used) != ESP_OK)
    {
        return;
    }
    // The download needs room for the .update copy; keep generous headroom.
    uint32_t want = needed + 64 * 1024;
    if (total - used >= want)
    {
        return;
    }
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "SPIFFS gc: %u free, want %lu",
                (unsigned)(total - used), (unsigned long)want);
    esp_err_t err = esp_spiffs_gc(NULL, want - (total - used));
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "SPIFFS gc failed: %s", esp_err_to_name(err));
    }
}

/* Fetch <base>/<version>/manifest.txt, verify its signature and downgrade
 * stamp, and install it as MANIFEST_PATH. Fills *m from the accepted file.
 * ESP_ERR_INVALID_RESPONSE = authenticity problem (report as SIGNATURE). */
static esp_err_t ota_fetch_manifest(int version, manifest_t *m)
{
    char url[512];
    char temp_path[64];
    snprintf(url, sizeof(url), "%s/%d/manifest.txt", UPDATE_SERVER_BASE, version);
    snprintf(temp_path, sizeof(temp_path), "%s.update", MANIFEST_PATH);

    if (download_file(url, temp_path, NULL, NULL, 0) != ESP_OK)
    {
        // download_file only renames onto temp_path on success, so nothing to clean
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest download failed: %s", url);
        return ESP_FAIL;
    }

    esp_err_t err = manifest_load_and_verify(temp_path, m);
    if (err != ESP_OK)
    {
        remove(temp_path);
        // Only an actual signature/format verdict is "rejected"; transient
        // failures (alloc, I/O) must stay retryable, not read as tampering.
        return (err == ESP_ERR_INVALID_RESPONSE) ? ESP_ERR_INVALID_RESPONSE : ESP_FAIL;
    }

    uint32_t applied = manifest_get_applied_generation();
    if (m->generated < applied)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest generation %lu older than applied %lu - replay rejected",
                    (unsigned long)m->generated, (unsigned long)applied);
        remove(temp_path);
        return ESP_ERR_INVALID_RESPONSE;
    }

    remove(MANIFEST_PATH);
    if (rename(temp_path, MANIFEST_PATH) != 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to install verified manifest");
        remove(temp_path);
        return ESP_FAIL;
    }
    return ESP_OK;
}

/* Bring SPIFFS in line with a verified manifest: hash every listed file
 * locally, skip the ones that already match, download the rest (verified,
 * up to 3 attempts each). Only changed files are ever written, which keeps
 * both flash wear and crash exposure minimal. Entries stream from the
 * manifest file on disk - nothing sizeable is held in RAM. */
typedef struct
{
    int version;
    int entry_count;
    int progress_lo, progress_hi;
    bool show_progress;
    int downloaded, skipped;
} reconcile_ctx_t;

static esp_err_t reconcile_entry_cb(const manifest_entry_t *e, int index, void *arg)
{
    reconcile_ctx_t *ctx = (reconcile_ctx_t *)arg;
    char url[512];
    char dest_path[128 + 8];
    uint8_t local_sha[MANIFEST_SHA256_LEN];

    snprintf(dest_path, sizeof(dest_path), "/spiffs/%s", e->path);

    struct stat st;
    bool matches = (stat(dest_path, &st) == 0 && (uint32_t)st.st_size == e->size &&
                    manifest_sha256_file(dest_path, local_sha) == ESP_OK &&
                    memcmp(local_sha, e->sha256, MANIFEST_SHA256_LEN) == 0);
    if (matches)
    {
        ctx->skipped++;
    }
    else
    {
        snprintf(url, sizeof(url), "%s/%d/%s", UPDATE_SERVER_BASE, ctx->version, e->path);
        esp_err_t err = ESP_FAIL;
        for (int attempt = 1; attempt <= 3 && err != ESP_OK; attempt++)
        {
            if (attempt > 1)
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Retrying %s (attempt %d/3)", e->path, attempt);
                vTaskDelay(pdMS_TO_TICKS(1000 * attempt));
            }
            ota_prepare_space(e->size);
            err = download_file(url, dest_path, NULL, e->sha256, e->size);
        }
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Giving up on %s after 3 attempts", e->path);
            return ESP_FAIL;
        }
        ctx->downloaded++;
        // Breather between writes: let httpd/WiFi/display drain their queues.
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    if (ctx->show_progress && ctx->entry_count > 0)
    {
        int p = ctx->progress_lo +
                ((index + 1) * (ctx->progress_hi - ctx->progress_lo)) / ctx->entry_count;
        update_progress(p, "Updating files...");
    }
    taskYIELD();
    return ESP_OK;
}

static esp_err_t ota_reconcile_files(const manifest_t *m, int progress_lo, int progress_hi, bool show_progress)
{
    reconcile_ctx_t ctx = {
        .version = m->version,
        .entry_count = m->entry_count,
        .progress_lo = progress_lo,
        .progress_hi = progress_hi,
        .show_progress = show_progress,
    };
    esp_err_t err = manifest_for_each_entry(MANIFEST_PATH, reconcile_entry_cb, &ctx);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Reconcile done: %d downloaded, %d already current",
                ctx.downloaded, ctx.skipped);
    return err;
}

/* First boot after an OTA (or first boot of the first signed firmware, whose
 * files arrived through the legacy unverified downloader): check every
 * SPIFFS file against the stored, signature-checked manifest and re-download
 * whatever does not match. Heals interrupted updates and any file corrupted
 * in transit. Clears the raised boot-rescue threshold once the set is clean. */
static void ota_self_heal(void)
{
    manifest_t m;
    esp_err_t err = manifest_load_and_verify(MANIFEST_PATH, &m);
    if (err == ESP_ERR_NOT_FOUND)
    {
        // Pre-signed-era SPIFFS: nothing to verify against; a quiet refresh
        // will fetch a manifest for this version soon.
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Self-heal: no stored manifest, skipping");
        manifest_set_self_heal_pending(false);
        manifest_set_ota_in_progress(false);
        return;
    }
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Self-heal: stored manifest failed verification, discarding it");
        f_ota_report_status(UPDATE_ERROR_SIGNATURE, "Self-heal: stored manifest invalid");
        remove(MANIFEST_PATH);
        manifest_set_self_heal_pending(false);
        manifest_set_ota_in_progress(false);
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Self-heal: verifying %d files against release %d",
                m.entry_count, m.version);
    xSemaphoreTake(http_mutex, portMAX_DELAY);
    err = ota_reconcile_files(&m, 0, 0, false);
    xSemaphoreGive(http_mutex);

    if (err == ESP_OK)
    {
        manifest_set_applied_generation(m.generated);
        manifest_set_self_heal_pending(false);
        manifest_set_ota_in_progress(false);
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Self-heal complete");
    }
    else
    {
        // Keep the pending flag; retried on the next update check.
        f_ota_report_status(UPDATE_ERROR_VERIFY, "Self-heal: reconcile failed");
    }
}

/* Re-fetch the current release's manifest so web-file-only fixes published
 * with `push.py --files-only` reach the fleet without a version bump, flash
 * write, or reboot. Files are only touched when the publisher stamped a NEWER
 * generation than what this device applied - so a user's locally customized
 * files are never reverted by a mere re-check, only by actual publishes
 * (matching what full updates have always done). */
static void ota_quiet_refresh(void)
{
    uint32_t applied = manifest_get_applied_generation();

    xSemaphoreTake(http_mutex, portMAX_DELAY);
    manifest_t m;
    esp_err_t err = ESP_FAIL;
    // A few spaced attempts: the first-boot refresh lands in the boot
    // congestion window where a transient alloc/connect failure is likely.
    for (int attempt = 1; attempt <= 3; attempt++)
    {
        if (attempt > 1)
        {
            vTaskDelay(pdMS_TO_TICKS(60 * 1000));
        }
        err = ota_fetch_manifest(fwversion, &m);
        if (err != ESP_FAIL) // OK or a definitive signature/downgrade verdict
        {
            break;
        }
    }
    if (err != ESP_OK)
    {
        xSemaphoreGive(http_mutex);
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Quiet refresh: manifest fetch/verify failed (%s)",
                    esp_err_to_name(err));
        if (err == ESP_ERR_INVALID_RESPONSE)
        {
            f_ota_report_status(UPDATE_ERROR_SIGNATURE, "Quiet refresh: manifest rejected");
        }
        return;
    }

    if (m.generated <= applied && applied != 0)
    {
        // Nothing new published since we last applied - leave files alone.
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Quiet refresh: generation %lu already applied",
                    (unsigned long)m.generated);
        xSemaphoreGive(http_mutex);
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Quiet refresh: applying generation %lu (files only)",
                (unsigned long)m.generated);
    manifest_set_ota_in_progress(true);
    err = ota_reconcile_files(&m, 0, 0, false);
    xSemaphoreGive(http_mutex);

    if (err == ESP_OK)
    {
        manifest_set_applied_generation(m.generated);
        f_ota_report_status(UPDATE_SUCCESS, "Quiet file refresh applied");
    }
    else
    {
        f_ota_report_status(UPDATE_ERROR_VERIFY, "Quiet refresh: reconcile failed");
    }
    manifest_set_ota_in_progress(false);
}

void f_ota_report_status(update_status_t status, const char *error_msg)
{
    char update_result_url[512];
    char mac_str[18];
    char encoded_error_msg[256]; // Buffer for URL-encoded error message

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(mac_str, sizeof(mac_str), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // URL encode the error message
    url_encode_string(error_msg ? error_msg : "", encoded_error_msg);

    // Format URL with parameters
    snprintf(update_result_url, sizeof(update_result_url),
             "%s/update_result?status=%s&code=%d&fw=%d&mac=%s&reason=%s",
             UPDATE_SERVER_BASE,
             status == UPDATE_SUCCESS ? "success" : "failure",
             status,
             fwversion,
             mac_str,
             encoded_error_msg);

    // Fire-and-forget telemetry: we don't read the response, so use NO event
    // handler. (Reusing f-wifi.c's shared http_event_handler/wifi_http_buffer
    // here would race the weather fetch on wifi_task — the same shared-state
    // bug that broke the version check.)
    esp_http_client_config_t config = {
        .url = update_result_url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .event_handler = NULL,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
        return;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA Report Status GET request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

// Dedicated response context for the version check. The OTA check runs on its
// own task (ota_update_task) but historically reused f-wifi.c's global
// http_event_handler + wifi_http_buffer + response_len + the met.no/forecast
// flags, which the weather fetch owns on wifi_task. When the two overlapped,
// the version check parsed met.no weather JSON (its entries are marked
// `{"time":"..."`), producing the spurious "Invalid FW:{\"time\":\"2..." error.
// Using a local buffer passed via user_data keeps the check fully isolated.
typedef struct
{
    char *buf;
    int len;
    int cap;
} ota_check_ctx_t;

static esp_err_t ota_check_event_handler(esp_http_client_event_t *evt)
{
    if (evt->event_id != HTTP_EVENT_ON_DATA)
        return ESP_OK;
    ota_check_ctx_t *ctx = (ota_check_ctx_t *)evt->user_data;
    if (!ctx || !ctx->buf || evt->data_len <= 0)
        return ESP_OK;
    int space = ctx->cap - 1 - ctx->len;
    if (space <= 0)
        return ESP_OK; // a version reply is tiny; ignore any excess
    int take = evt->data_len < space ? evt->data_len : space;
    memcpy(ctx->buf + ctx->len, evt->data, take);
    ctx->len += take;
    ctx->buf[ctx->len] = '\0';
    return ESP_OK;
}

void f_ota_check_update(void)
{
    ESP_LOGI_STACK(TAG, "OTA Check Update");

    if (!wifi_connected)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi not connected, skipping OTA check");
        return;
    }

    // First boot after an update - or first boot of the first signed
    // firmware, whose files arrived via the legacy path - verify the SPIFFS
    // file set against the stored manifest before doing anything else.
    if (manifest_get_self_heal_pending())
    {
        ota_self_heal();
    }
    else if (manifest_get_applied_generation() == 0)
    {
        struct stat heal_st;
        if (stat(MANIFEST_PATH, &heal_st) == 0)
        {
            ota_self_heal();
        }
    }

    // Thread-safe time check
    static portMUX_TYPE time_check_mutex = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&time_check_mutex);
    time_t current_time = time(NULL);
    if (current_time - last_check_time < UPDATE_CHECK_INTERVAL)
    {
        portEXIT_CRITICAL(&time_check_mutex);
        return;
    }
    last_check_time = current_time;
    portEXIT_CRITICAL(&time_check_mutex);

    if (!eeprom_update_firmware)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Firmware updates are disabled");
        return;
    }

    // Use dynamic allocation for query params to avoid buffer overflow
    char url[512] = "";

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    uint16_t flash = flash_size / 1024; // Convert to KB

    size_t total_size = 0;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL)
    {
        const esp_partition_t *part = esp_partition_get(it);
        total_size = part->size;
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    uint16_t app_size = total_size / 1024; // Convert to KB

    // Build integration string (A=HA, B=Stock, C=Dexcom, D=Freestyle)
    char integrations[5] = ""; // Max 4 letters + null terminator
    int int_idx = 0;
    if (integration_active[INTEGRATION_HA])
    {
        integrations[int_idx++] = 'A';
    }
    if (integration_active[INTEGRATION_STOCK])
    {
        integrations[int_idx++] = 'B';
    }
    if (integration_active[INTEGRATION_DEXCOM])
    {
        integrations[int_idx++] = 'C';
    }
    if (integration_active[INTEGRATION_FREESTYLE])
    {
        integrations[int_idx++] = 'D';
    }
    if (integration_active[INTEGRATION_NIGHTSCOUT])
    {
        integrations[int_idx++] = 'E';
    }
    integrations[int_idx] = '\0';

    char escaped_hostname[64];
    url_encode_string(eeprom_hostname, escaped_hostname);

    snprintf(url, 512,
             "%s/latest?host=%s&fw=%d&mac=%02X%02X%02X%02X%02X%02X&rev=%s&ver=%s&fla=%d&app=%d&poh=%lu&int=%s",
             UPDATE_SERVER_BASE, escaped_hostname, fwversion,
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], revision, version,
             flash, app_size, current_poh, integrations);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Checking for updates at %s", url);

    // Isolated response buffer (see ota_check_ctx_t). Not the shared
    // wifi_http_buffer — that belongs to the weather fetch on another task.
    char resp_buf[256];
    resp_buf[0] = '\0';
    ota_check_ctx_t ctx = {.buf = resp_buf, .len = 0, .cap = (int)sizeof(resp_buf)};

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .event_handler = ota_check_event_handler,
        .user_data = &ctx,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ota_handle_failure("Failed to initialize HTTP client", UPDATE_ERROR_DOWNLOAD, false);
        return;
    }

    // Add content type validation
    esp_http_client_set_header(client, "Accept", "application/json, text/plain");

    esp_err_t err = esp_http_client_perform(client);
    int status_code = (err == ESP_OK) ? esp_http_client_get_status_code(client) : -1;
    esp_http_client_cleanup(client); // client no longer needed; parse the local buffer
    if (err != ESP_OK)
    {
        ota_handle_failure("OTA Latest FW GET request failed", UPDATE_ERROR_DOWNLOAD, false);
        return;
    }

    // --- Hardened parse (#3) -------------------------------------------------
    // Skip leading whitespace; the buffer is always a valid local array.
    const char *resp = resp_buf;
    while (*resp == ' ' || *resp == '\t' || *resp == '\r' || *resp == '\n')
        resp++;

    // An empty / non-200 / unrecognised response must NOT fail the device — just
    // skip this cycle and retry later. (Previously any id-less JSON, e.g. stale
    // weather data, was logged as a hard "Invalid FW" failure.)
    if (status_code != 200 || *resp == '\0')
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Update check: status %d, %d bytes; skipping",
                    status_code, ctx.len);
        return;
    }

    int new_version = 0;
    bool version_parsed = false;

    if (resp[0] >= '0' && resp[0] <= '9')
    {
        new_version = atoi(resp);
        version_parsed = true;
    }
    else if (resp[0] == '{' || resp[0] == '[')
    {
        cJSON *root = cJSON_Parse(resp);
        if (root)
        {
            cJSON *id = cJSON_GetObjectItem(root, "id");
            if (!cJSON_IsNumber(id))
                id = cJSON_GetObjectItem(root, "version");
            if (!cJSON_IsNumber(id))
                id = cJSON_GetObjectItem(root, "latest");
            if (cJSON_IsNumber(id))
            {
                new_version = id->valueint;
                version_parsed = true;
            }
            cJSON_Delete(root);
        }
    }

    if (!version_parsed)
    {
        // Unrecognised body (stale weather JSON, an HTML error page, etc.).
        // Log a short preview and skip — do not mark a firmware failure.
        char preview[20];
        size_t p = 0;
        for (size_t i = 0; resp[i] != '\0' && i < 16 && p < sizeof(preview) - 1; i++)
            preview[p++] = (resp[i] >= 32 && resp[i] < 127) ? resp[i] : '.';
        preview[p] = '\0';
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Update check: unrecognised response '%s', skipping", preview);
        return;
    }

    if (new_version <= fwversion)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No update available, latest %d, current %d", new_version, fwversion);

        // Every QUIET_REFRESH_EVERY_CHECKS idle checks (or right away while no
        // release has ever been applied), pick up web-file-only publishes.
        static int quiet_refresh_counter = 0;
        quiet_refresh_counter++;
        if (quiet_refresh_counter >= QUIET_REFRESH_EVERY_CHECKS ||
            manifest_get_applied_generation() == 0)
        {
            quiet_refresh_counter = 0;
            ota_quiet_refresh();
        }
        return;
    }

    ota_reinstall_in_progress = false;
    f_ota_do_update(new_version);
}

static void f_ota_do_update(int version)
{
    char url[512] = "";

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Starting update process to version %d", version);
    ota_wait_for_heap("update");
    xSemaphoreTake(http_mutex, portMAX_DELAY);
    update_progress(0, "Starting update...");
    ota_update_in_progress = true;
    ota_updating_message = false;
    ota_start_time = 0;

    esp_err_t err;

    // Fetch and authenticate the release manifest FIRST - nothing is written
    // until the release provably came from our signing key.
    update_progress(2, "Verifying release...");
    manifest_t m;
    err = ota_fetch_manifest(version, &m);
    if (err == ESP_ERR_INVALID_RESPONSE)
    {
        ota_handle_failure("Release manifest failed signature/downgrade check", UPDATE_ERROR_SIGNATURE, true);
        return;
    }
    if (err != ESP_OK)
    {
        ota_handle_failure("Failed to download release manifest", UPDATE_ERROR_DOWNLOAD, true);
        return;
    }

    // Raise the boot-loop rescue threshold while SPIFFS writes are in flight:
    // an interrupted update resumes safely via self-heal, so crash-reboots
    // here must not factory-reset the user's settings.
    manifest_set_ota_in_progress(true);

    // Bring SPIFFS files in line with the manifest (unchanged files are skipped).
    if (ota_reconcile_files(&m, 5, 50, true) != ESP_OK)
    {
        ota_handle_failure("File reconcile failed verification after retries", UPDATE_ERROR_VERIFY, true);
        return;
    }

    // Keep what the firmware step needs, then release the entry table.
    uint8_t fw_expected_sha[MANIFEST_SHA256_LEN];
    memcpy(fw_expected_sha, m.fw_sha256, sizeof(fw_expected_sha));
    uint32_t fw_expected_size = m.fw_size;
    uint32_t manifest_generation = m.generated;

    // ****************
    // Download and install firmware
    // ****************
    snprintf(url, sizeof(url), "%s/revE%d.bin", UPDATE_SERVER_BASE, version);

    // Get all partitions and verify their state
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

    if (next == NULL)
    {
        ota_handle_failure("Failed to get next update partitions", UPDATE_ERROR_INSTALL, true);
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Downloading firmware %s...", url);
    update_progress(50, "Downloading firmware...");

    esp_ota_handle_t ota_handle;
    err = esp_ota_begin(next, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        ota_handle_failure("esp_ota_begin failed", UPDATE_ERROR_INSTALL, true);
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "OTA update started for partition: %s", next->label);

    esp_http_client_config_t fw_config = {
        .url = url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .buffer_size = 512,
        .buffer_size_tx = 512,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t fw_client = esp_http_client_init(&fw_config);
    if (fw_client == NULL)
    {
        ota_handle_failure_with_cleanup("Failed to initialize HTTP client for firmware", UPDATE_ERROR_DOWNLOAD, NULL, ota_handle, true);
        return;
    }

    err = esp_http_client_open(fw_client, 0);
    if (err != ESP_OK)
    {
        ota_handle_failure_with_cleanup("Failed to open HTTP connection for firmware", UPDATE_ERROR_DOWNLOAD, fw_client, ota_handle, true);
        return;
    }

    int fw_content_length = esp_http_client_fetch_headers(fw_client);
    if (fw_content_length < 0)
    {
        ota_handle_failure_with_cleanup("HTTP client fetch headers failed for firmware", UPDATE_ERROR_DOWNLOAD, fw_client, ota_handle, true);
        return;
    }

    // Validate firmware size
    if (fw_content_length > next->size)
    {
        ota_handle_failure_with_cleanup("Firmware too large for partition", UPDATE_ERROR_VERIFY, fw_client, ota_handle, true);
        return;
    }
    if ((uint32_t)fw_content_length != fw_expected_size)
    {
        ota_handle_failure_with_cleanup("Firmware size disagrees with signed manifest", UPDATE_ERROR_VERIFY, fw_client, ota_handle, true);
        return;
    }

    int total_read = 0;
    char fw_buffer[1024] __attribute__((aligned(4)));
    int fw_read_len;
    int last_progress = 50;
    bool first_chunk = true;

    // Stream-hash the image; compared against the signed manifest below.
    // (esp_ota_end() checks integrity via the image's embedded digest; the
    // manifest comparison is what proves the image came from our servers.)
    manifest_hash_t fw_sha_ctx;
    if (manifest_hash_begin(&fw_sha_ctx) != ESP_OK)
    {
        ota_handle_failure_with_cleanup("Hash init failed", UPDATE_ERROR_INSTALL, fw_client, ota_handle, true);
        return;
    }

    while (total_read < fw_content_length && (fw_read_len = esp_http_client_read(fw_client, fw_buffer, sizeof(fw_buffer))) > 0)
    {
        // Verify magic byte in first chunk
        if (first_chunk)
        {
            if (fw_buffer[0] != 0xE9)
            {
                manifest_hash_abort(&fw_sha_ctx);
                ota_handle_failure_with_cleanup("Invalid firmware image: magic byte mismatch", UPDATE_ERROR_VERIFY, fw_client, ota_handle, true);
                return;
            }
            first_chunk = false;
        }

        manifest_hash_update(&fw_sha_ctx, fw_buffer, fw_read_len);

        if (esp_ota_write(ota_handle, fw_buffer, fw_read_len) != ESP_OK)
        {
            ota_handle_failure_with_cleanup("esp_ota_write failed", UPDATE_ERROR_INSTALL, fw_client, ota_handle, true);
            return;
        }
        total_read += fw_read_len;
        float progress = 50.0f + ((float)total_read / (float)fw_content_length) * 50.0f;
        taskYIELD();
        if ((int)progress - last_progress >= 5)
        {
            update_progress((int)progress, "Updating firmware...");
            last_progress = (int)progress;
        }
    }
    esp_http_client_cleanup(fw_client);
    fw_client = NULL; /* client already cleaned up; failure paths must not use it */

    uint8_t fw_actual_sha[MANIFEST_SHA256_LEN];
    esp_err_t sha_ret = manifest_hash_end(&fw_sha_ctx, fw_actual_sha);

    if (total_read != fw_content_length)
    {
        ota_handle_failure_with_cleanup("Firmware download incomplete", UPDATE_ERROR_DOWNLOAD, NULL, ota_handle, true);
        return;
    }

    // Origin check: the streamed image must be the one the signed manifest names.
    if (sha_ret != ESP_OK || memcmp(fw_actual_sha, fw_expected_sha, MANIFEST_SHA256_LEN) != 0)
    {
        ota_handle_failure_with_cleanup("Firmware hash disagrees with signed manifest", UPDATE_ERROR_SIGNATURE, NULL, ota_handle, true);
        return;
    }

    if (esp_ota_end(ota_handle) != ESP_OK)
    {
        ota_handle_failure_with_cleanup("esp_ota_end failed", UPDATE_ERROR_INSTALL, NULL, ota_handle, true);
        return;
    }

    // Set boot partition and verify
    err = esp_ota_set_boot_partition(next);
    if (err != ESP_OK)
    {
        ota_handle_failure("esp_ota_set_boot_partition failed", UPDATE_ERROR_INSTALL, true);
        return;
    }

    // Record the applied release and ask the new firmware to verify all
    // SPIFFS files on its first boot (self-heal). ota_in_progress stays set
    // on purpose: the raised rescue threshold covers the first-boot window
    // and self-heal clears it once the file set checks out.
    manifest_set_applied_generation(manifest_generation);
    manifest_set_self_heal_pending(true);

    update_progress(100, "Update complete, rebooting...");
    f_ota_report_status(UPDATE_SUCCESS, "OK");

    // Small delay to ensure logs are written
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

void f_ota_reinstall_current(void)
{
    ESP_LOGI_STACK(TAG, "OTA Reinstall Current");

    if (!wifi_connected)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi not connected, skipping OTA reinstall");
        return;
    }

    if (ota_update_in_progress)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "OTA update already in progress");
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Reinstalling current firmware version %d", fwversion);
    ota_reinstall_in_progress = true;
    f_ota_do_update(fwversion);
}

// OTA update thread function
static void ota_update_task(void *pvParameters)
{
    while (1)
    {
        // Wait for the semaphore to be given
        if (xSemaphoreTake(ota_update_semaphore, portMAX_DELAY) == pdTRUE)
        {
            bool reinstall = ota_reinstall_mode;
            ota_reinstall_mode = false;

            if (wifi_connected)
            {
                // Log stack high water mark before update
                size_t stack_free_bytes = uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update starting - Stack free: %u bytes", (unsigned)stack_free_bytes);
                if (reinstall)
                {
                    f_ota_reinstall_current();
                }
                else
                {
                    f_ota_check_update();
                }

                // Log stack high water mark after update
                stack_free_bytes = uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update finished - Stack free: %u bytes", (unsigned)stack_free_bytes);
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "OTA update skipped - WiFi not connected");
            }
        }
    }
}

void f_ota_start_update_thread(void)
{
    // Create OTA update semaphore
    ota_update_semaphore = xSemaphoreCreateBinary();
    if (ota_update_semaphore == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create OTA update semaphore");
        return;
    }

    // Create OTA update task on APP_CPU (core 1). Same rationale as wifi_task /
    // integration_update_task: keep heavy HTTP / mbedtls work off PRO_CPU so
    // WiFi, lwIP and httpd remain responsive.
    // 10240: manifest ECDSA verification (mbedtls_pk_verify) runs on this
    // task and needs several KB of stack on top of the HTTP/download paths.
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        ota_update_task,
        "ota_update_task",
        10240,
        NULL,
        3,
        &ota_update_task_handle,
        1); // APP_CPU

    if (xReturned != pdPASS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create OTA update task");
        vSemaphoreDelete(ota_update_semaphore);
        ota_update_semaphore = NULL;
        return;
    }

    // Enable stack monitoring for the OTA update task
    size_t stack_free_bytes = uxTaskGetStackHighWaterMark(ota_update_task_handle) * sizeof(StackType_t);
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update task created with initial stack free: %u bytes", (unsigned)stack_free_bytes);
}

void f_ota_stop_update_thread(void)
{
    // Cleanup OTA update task and semaphore
    if (ota_update_task_handle != NULL)
    {
        vTaskDelete(ota_update_task_handle);
        ota_update_task_handle = NULL;
    }
    if (ota_update_semaphore != NULL)
    {
        vSemaphoreDelete(ota_update_semaphore);
        ota_update_semaphore = NULL;
    }
}

void f_ota_trigger_update(void)
{
    ota_reinstall_mode = false;
    if (ota_update_semaphore != NULL)
    {
        xSemaphoreGive(ota_update_semaphore);
    }
}

void f_ota_trigger_reinstall(void)
{
    ota_reinstall_mode = true;
    if (ota_update_semaphore != NULL)
    {
        xSemaphoreGive(ota_update_semaphore);
    }
}