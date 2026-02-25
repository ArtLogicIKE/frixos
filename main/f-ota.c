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

static const char *TAG = "f-ota";
static update_progress_callback_t progress_callback = NULL;
static time_t last_check_time = 0;

// OTA update thread control
TaskHandle_t ota_update_task_handle = NULL;
SemaphoreHandle_t ota_update_semaphore = NULL;

// Internal function declarations
static esp_err_t download_file(const char *url, const char *dest_path, int *progress);
static void update_progress(int progress, const char *message);
static void ota_update_task(void *pvParameters);
static void log_partition_info(const esp_partition_t *partition, const char *prefix);
static void ota_handle_failure(const char *error_msg, update_status_t status, bool release_mutex);
static void ota_handle_failure_with_cleanup(const char *error_msg, update_status_t status, esp_http_client_handle_t client, esp_ota_handle_t ota_handle, bool release_mutex);
static void cleanup_update_files(void);

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
    ota_updating_message = false;
    ota_start_time = 0;

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

static esp_err_t download_file(const char *url, const char *dest_path, int *progress)
{
    // Create update path with .update extension
    char update_path[512];
    snprintf(update_path, sizeof(update_path), "%s.update", dest_path);

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .buffer_size = HTTP_BUFFER_SIZE,
        .buffer_size_tx = HTTP_BUFFER_SIZE,
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

    FILE *file = fopen(update_path, "wb");
    if (file == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open update file for writing: %s", update_path);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    int total_read = 0;
    char *buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "download_file");
    int read_len;
    bool download_success = false;

    while (total_read < content_length && (read_len = esp_http_client_read(client, buffer, 1024)) > 0)
    {
        fwrite(buffer, 1, read_len, file);
        total_read += read_len;
        taskYIELD();
        if (progress)
        {
            *progress = (total_read * 100) / content_length;
        }
    }
    release_shared_buffer(buffer);
    fclose(file);
    esp_http_client_cleanup(client);

    // Check if download was successful
    if (total_read == content_length)
    {
        remove(dest_path); // Remove the original file
        // Download successful, replace original file with update file
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
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Download incomplete: %d of %d bytes", total_read, content_length);
        // Clean up the partial update file
        remove(update_path);
    }

    return download_success ? ESP_OK : ESP_FAIL;
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

    esp_http_client_config_t config = {
        .url = update_result_url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .event_handler = http_event_handler, // Use the shared event handler
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    // Use HTTP GET instead of POST
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
        esp_http_client_cleanup(client);
        return;
    }
}

void f_ota_check_update(void)
{
    ESP_LOGI_STACK(TAG, "OTA Check Update");

    if (!wifi_connected)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi not connected, skipping OTA check");
        return;
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

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = UPDATE_TIMEOUT_MS,
        .event_handler = http_event_handler,
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
    if (err != ESP_OK)
    {
        esp_http_client_cleanup(client);
        ota_handle_failure("OTA Latest FW GET request failed", UPDATE_ERROR_DOWNLOAD, false);
        return;
    }

    int new_version = 0;
    bool version_parsed = false;

    if (strlen(wifi_http_buffer) < 5)
    {
        // Clean the response buffer by removing whitespace and newlines
        char *clean_response = wifi_http_buffer;
        while (*clean_response == ' ' || *clean_response == '\t' || *clean_response == '\r' || *clean_response == '\n')
        {
            clean_response++;
        }

        // Find the end of the number and null-terminate
        char *end = clean_response;
        while (*end >= '0' && *end <= '9')
        {
            end++;
        }
        *end = '\0';

        new_version = atoi(clean_response);
        version_parsed = true;
    }
    else
    {
        cJSON *root = cJSON_Parse(wifi_http_buffer);
        if (root == NULL)
        {
            esp_http_client_cleanup(client);
            ota_handle_failure("Failed to parse JSON response", UPDATE_ERROR_DOWNLOAD, false);
            return;
        }

        cJSON *latest_id = cJSON_GetObjectItem(root, "id");
        if (cJSON_IsNumber(latest_id))
        {
            new_version = latest_id->valueint;
            version_parsed = true;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid response format");
        }
        cJSON_Delete(root);
    }

    esp_http_client_cleanup(client);

    if (!version_parsed)
    {
        char escaped[64];
        size_t j = 0;
        const char *resp = (wifi_http_buffer != NULL) ? wifi_http_buffer : "";
        for (size_t i = 0; i < 10 && resp[i] != '\0' && j < sizeof(escaped) - 6; i++)
        {
            char c = resp[i];
            if (c == '&')
            {
                const char *e = "&amp;";
                while (*e)
                    escaped[j++] = *e++;
            }
            else if (c == '<')
            {
                const char *e = "&lt;";
                while (*e)
                    escaped[j++] = *e++;
            }
            else if (c == '>')
            {
                const char *e = "&gt;";
                while (*e)
                    escaped[j++] = *e++;
            }
            else if (c == '"')
            {
                const char *e = "&quot;";
                while (*e)
                    escaped[j++] = *e++;
            }
            else if (c == '\'')
            {
                const char *e = "&#39;";
                while (*e)
                    escaped[j++] = *e++;
            }
            else
                escaped[j++] = c;
        }
        escaped[j] = '\0';
        char msg[80];
        snprintf(msg, sizeof(msg), "Invalid FW:%s", escaped);
        ota_handle_failure(msg, UPDATE_ERROR_DOWNLOAD, false);
        return;
    }

    if (new_version <= fwversion)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No update available, latest %d, current %d", new_version, fwversion);
        return;
    }

    // Start update process
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Starting update process to version %d", new_version);
    xSemaphoreTake(http_mutex, portMAX_DELAY);
    update_progress(0, "Starting update...");
    ota_update_in_progress = true;
    ota_updating_message = false;
    ota_start_time = 0;

    // Download SPIFFS files
    char update_dir[96];
    snprintf(update_dir, sizeof(update_dir), "%s/%d", UPDATE_SERVER_BASE, new_version);

    // First download the files.txt manifest; reuse url variable
    snprintf(url, sizeof(url), "%s/files.txt", update_dir);

    char manifest_path[300];
    snprintf(manifest_path, sizeof(manifest_path), "/spiffs/files.txt");

    if (download_file(url, manifest_path, NULL) != ESP_OK)
    {
        ota_handle_failure("Failed to download files.txt manifest", UPDATE_ERROR_DOWNLOAD, true);
        return;
    }

    // Read the manifest file
    FILE *manifest = fopen(manifest_path, "r");
    if (manifest == NULL)
    {
        ota_handle_failure("Failed to open files.txt manifest", UPDATE_ERROR_DOWNLOAD, true);
        return;
    }

    // Count total files in manifest
    char line[256];
    int file_count = 0;
    while (fgets(line, sizeof(line), manifest))
    {
        if (line[0] != '\n' && line[0] != '#')
        {
            file_count++;
        }
    }
    rewind(manifest);

    // Download each file listed in the manifest
    int current_file = 0;
    char dest_path[300];
    while (fgets(line, sizeof(line), manifest))
    {
        if (line[0] == '\n' || line[0] == '#')
        {
            continue;
        }

        if (strstr(line, "files.txt") == NULL)
        {
            line[strcspn(line, "\r\n")] = 0;
            snprintf(url, sizeof(url), "%s/%s", update_dir, line);
            snprintf(dest_path, sizeof(dest_path), "/spiffs/%s", line);

            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Downloading file %i of %i: %s to %s", current_file + 1, file_count, url, dest_path);
            int progress = 0;
            if (download_file(url, dest_path, &progress) != ESP_OK)
            {
                fclose(manifest);
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), "Failed to download file: %s/%.128s", update_dir, line);
                url_encode_string(error_msg, error_msg);
                ota_handle_failure(error_msg, UPDATE_ERROR_DOWNLOAD, true);
                return;
            }
        }

        current_file++;
        // Use floating point for more accurate progress
        float overall_progress = ((float)current_file / (float)file_count) * 50.0f;
        update_progress((int)overall_progress, "Updating files...");
        taskYIELD();
    }
    fclose(manifest);

    // Clean up the manifest file with error checking
    if (remove("/spiffs/files.txt") != 0)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to remove manifest file: %s", strerror(errno));
    }

    // ****************
    // Download and install firmware
    // ****************
    snprintf(url, sizeof(url), "%s/revE%d.bin", UPDATE_SERVER_BASE, new_version);

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

    int total_read = 0;
    char fw_buffer[1024] __attribute__((aligned(4)));
    int fw_read_len;
    int last_progress = 50;
    bool first_chunk = true;
    uint32_t checksum = 0;

    while (total_read < fw_content_length && (fw_read_len = esp_http_client_read(fw_client, fw_buffer, sizeof(fw_buffer))) > 0)
    {
        // Verify magic byte in first chunk
        if (first_chunk)
        {
            if (fw_buffer[0] != 0xE9)
            {
                ota_handle_failure_with_cleanup("Invalid firmware image: magic byte mismatch", UPDATE_ERROR_VERIFY, fw_client, ota_handle, true);
                return;
            }
            first_chunk = false;
        }

        // Calculate checksum
        for (int i = 0; i < fw_read_len; i++)
        {
            checksum += (uint32_t)fw_buffer[i];
        }

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

    // Verify checksum
    if (total_read != fw_content_length)
    {
        ota_handle_failure_with_cleanup("Firmware download incomplete", UPDATE_ERROR_DOWNLOAD, NULL, ota_handle, true);
        return;
    }

    if (esp_ota_end(ota_handle) != ESP_OK)
    {
        ota_handle_failure_with_cleanup("esp_ota_end failed", UPDATE_ERROR_INSTALL, NULL, ota_handle, true);
        return;
    }

    // Verify the new firmware
    /*
    esp_err_t verify_err = esp_ota_check_rollback_is_possible();
    if (verify_err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Firmware verification failed: %s", esp_err_to_name(verify_err));
        f_ota_report_status(UPDATE_ERROR_VERIFY);
        return;
    }
    */

    // Set boot partition and verify
    err = esp_ota_set_boot_partition(next);
    if (err != ESP_OK)
    {
        ota_handle_failure("esp_ota_set_boot_partition failed", UPDATE_ERROR_INSTALL, true);
        return;
    }

    update_progress(100, "Update complete, rebooting...");
    f_ota_report_status(UPDATE_SUCCESS, "OK");

    // Small delay to ensure logs are written
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}

// OTA update thread function
static void ota_update_task(void *pvParameters)
{
    while (1)
    {
        // Wait for the semaphore to be given
        if (xSemaphoreTake(ota_update_semaphore, portMAX_DELAY) == pdTRUE)
        {
            if (wifi_connected)
            {
                // Log stack high water mark before update
                UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update starting - Stack high water mark: %d", uxHighWaterMark);
                f_ota_check_update();

                // Log stack high water mark after update
                uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
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

    // Create OTA update task
    BaseType_t xReturned = xTaskCreatePinnedToCore(
        ota_update_task,
        "ota_update_task",
        8192, // Stack size
        NULL, // Task parameters
        3,    // Priority
        &ota_update_task_handle,
        0 // ping to the APP core
    );

    if (xReturned != pdPASS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create OTA update task");
        vSemaphoreDelete(ota_update_semaphore);
        ota_update_semaphore = NULL;
        return;
    }

    // Enable stack monitoring for the OTA update task
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(ota_update_task_handle);
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update task created with initial stack high water mark: %d", uxHighWaterMark);
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
    // Signal the OTA update thread
    if (ota_update_semaphore != NULL)
    {
        xSemaphoreGive(ota_update_semaphore);
    }
}