#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "psa/crypto.h"
#include "f-manifest.h"
#include "f-ota-pubkey.h"
#include "frixos.h"

static const char *TAG = "f-manifest";

#define MANIFEST_MAGIC "frixos-manifest-v1"
#define MANIFEST_MAX_SIZE (64 * 1024)
#define MANIFEST_SIG_RAW_LEN 64 // ECDSA P-256 r||s
#define MANIFEST_NVS_NAMESPACE "frixos"
#define NVS_KEY_GENERATION "manifest_gen"
#define NVS_KEY_OTA_IN_PROGRESS "ota_in_prog"
#define NVS_KEY_SELF_HEAL "self_heal"

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

static bool parse_hex(const char *hex, uint8_t *out, int out_len)
{
    for (int i = 0; i < out_len; i++)
    {
        int hi = hex_nibble(hex[i * 2]);
        int lo = hex_nibble(hex[i * 2 + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return true;
}

/* Parse one "f <sha256hex> <size> <path>" line (no trailing newline). */
static bool parse_entry_line(const char *line, manifest_entry_t *e)
{
    const char *hex = line + 2;
    const char *sp1 = strchr(hex, ' ');
    const char *sp2 = sp1 ? strchr(sp1 + 1, ' ') : NULL;
    if (sp1 == NULL || sp2 == NULL || (sp1 - hex) != MANIFEST_SHA256_LEN * 2 ||
        strlen(sp2 + 1) == 0 || strlen(sp2 + 1) >= MANIFEST_MAX_PATH)
    {
        return false;
    }
    if (!parse_hex(hex, e->sha256, MANIFEST_SHA256_LEN))
    {
        return false;
    }
    e->size = (uint32_t)strtoul(sp1 + 1, NULL, 10);
    strlcpy(e->path, sp2 + 1, sizeof(e->path));
    return true;
}

static esp_err_t verify_signature_hash(const uint8_t hash[MANIFEST_SHA256_LEN],
                                       const uint8_t sig[MANIFEST_SIG_RAW_LEN])
{
    psa_status_t status = psa_crypto_init(); // idempotent
    if (status != PSA_SUCCESS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "psa_crypto_init failed: %d", (int)status);
        return ESP_FAIL;
    }

    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    psa_key_id_t key = 0;
    status = psa_import_key(&attr, OTA_MANIFEST_PUBKEY_P256, OTA_MANIFEST_PUBKEY_P256_LEN, &key);
    if (status != PSA_SUCCESS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Embedded public key rejected by PSA: %d", (int)status);
        return ESP_FAIL;
    }

    status = psa_verify_hash(key, PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                             hash, MANIFEST_SHA256_LEN, sig, MANIFEST_SIG_RAW_LEN);
    psa_destroy_key(key);
    if (status != PSA_SUCCESS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest signature verification FAILED: %d", (int)status);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

/* Fully streaming (no heap, ~1 KB stack): the device cannot rely on having
 * a manifest-sized buffer free - a 12 KB malloc here failed in the field
 * during the boot window. Layout knowledge used: the "sig" line is the LAST
 * line of the file, so the signed region is everything before it. */
esp_err_t manifest_load_and_verify(const char *path, manifest_t *out)
{
    memset(out, 0, sizeof(*out));

    FILE *f = fopen(path, "rb");
    if (f == NULL)
    {
        return ESP_ERR_NOT_FOUND;
    }
    struct stat st;
    if (stat(path, &st) != 0 || st.st_size <= 0 || st.st_size > MANIFEST_MAX_SIZE)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest missing or implausible size");
        fclose(f);
        return ESP_ERR_INVALID_RESPONSE;
    }

    // --- Pass 1: read the tail to find the signature line ---
    // "sig " + 128 hex + '\n' = 133 bytes; read a bit more for safety.
    char tail[192];
    long tail_off = st.st_size > (long)(sizeof(tail) - 1) ? st.st_size - (long)(sizeof(tail) - 1) : 0;
    fseek(f, tail_off, SEEK_SET);
    size_t tail_len = fread(tail, 1, sizeof(tail) - 1, f);
    tail[tail_len] = '\0';

    char *sig_line = NULL;
    for (char *p = tail; (p = strstr(p, "\nsig ")) != NULL; p++)
    {
        sig_line = p; // keep the LAST occurrence
    }
    if (sig_line == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest has no signature line");
        fclose(f);
        return ESP_ERR_INVALID_RESPONSE;
    }
    size_t region_len = (size_t)tail_off + (sig_line - tail) + 1; // include the '\n'
    char *sig_hex = sig_line + 5;
    char *sig_end = strpbrk(sig_hex, "\r\n");
    if (sig_end)
        *sig_end = '\0';

    uint8_t sig_raw[MANIFEST_SIG_RAW_LEN];
    if (strlen(sig_hex) != MANIFEST_SIG_RAW_LEN * 2 ||
        !parse_hex(sig_hex, sig_raw, MANIFEST_SIG_RAW_LEN))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest signature not valid hex");
        fclose(f);
        return ESP_ERR_INVALID_RESPONSE;
    }

    // --- Pass 2: stream-hash the signed region and verify the signature ---
    if (psa_crypto_init() != PSA_SUCCESS)
    {
        fclose(f);
        return ESP_FAIL;
    }
    manifest_hash_t hctx;
    if (manifest_hash_begin(&hctx) != ESP_OK)
    {
        fclose(f);
        return ESP_FAIL;
    }
    fseek(f, 0, SEEK_SET);
    uint8_t chunk[512];
    size_t remaining = region_len;
    bool magic_checked = false, io_error = false;
    while (remaining > 0)
    {
        size_t want = remaining < sizeof(chunk) ? remaining : sizeof(chunk);
        size_t got = fread(chunk, 1, want, f);
        if (got != want)
        {
            io_error = true;
            break;
        }
        if (!magic_checked)
        {
            if (got < strlen(MANIFEST_MAGIC) ||
                memcmp(chunk, MANIFEST_MAGIC, strlen(MANIFEST_MAGIC)) != 0)
            {
                io_error = true; // not our format; treated below
                break;
            }
            magic_checked = true;
        }
        if (manifest_hash_update(&hctx, chunk, got) != ESP_OK)
        {
            io_error = true;
            break;
        }
        remaining -= got;
    }
    if (io_error)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest read/magic failed while hashing");
        manifest_hash_abort(&hctx);
        fclose(f);
        return ESP_ERR_INVALID_RESPONSE;
    }

    uint8_t hash[MANIFEST_SHA256_LEN];
    if (manifest_hash_end(&hctx, hash) != ESP_OK)
    {
        fclose(f);
        return ESP_FAIL;
    }
    esp_err_t err = verify_signature_hash(hash, sig_raw);
    if (err != ESP_OK)
    {
        fclose(f);
        return err;
    }

    // --- Pass 3: signature is good; parse header + validate entries ---
    fseek(f, 0, SEEK_SET);
    char line[MANIFEST_MAX_PATH + MANIFEST_SHA256_LEN * 2 + 24];
    long consumed = 0;
    bool have_fw = false, malformed = false;
    while (consumed < (long)region_len && fgets(line, sizeof(line), f) != NULL)
    {
        consumed += (long)strlen(line);
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "version ", 8) == 0)
        {
            out->version = atoi(line + 8);
        }
        else if (strncmp(line, "generated ", 10) == 0)
        {
            out->generated = (uint32_t)strtoul(line + 10, NULL, 10);
        }
        else if (strncmp(line, "fw ", 3) == 0)
        {
            // "fw <name> <size> <sha256hex>"
            char *sp1 = strchr(line + 3, ' ');
            char *sp2 = sp1 ? strchr(sp1 + 1, ' ') : NULL;
            if (sp2 == NULL || strlen(sp2 + 1) < MANIFEST_SHA256_LEN * 2 ||
                !parse_hex(sp2 + 1, out->fw_sha256, MANIFEST_SHA256_LEN))
            {
                malformed = true;
                break;
            }
            out->fw_size = (uint32_t)strtoul(sp1 + 1, NULL, 10);
            have_fw = true;
        }
        else if (strncmp(line, "f ", 2) == 0)
        {
            // Validate only - entries are re-read from disk on demand
            // (manifest_for_each_entry), never held in RAM as an array.
            manifest_entry_t e;
            if (!parse_entry_line(line, &e))
            {
                malformed = true;
                break;
            }
            out->entry_count++;
        }
    }
    fclose(f);

    if (malformed || !have_fw || out->version <= 0 || out->generated == 0 || out->entry_count == 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest verified but malformed (fw:%d ver:%d gen:%lu n:%d)",
                    have_fw, out->version, (unsigned long)out->generated, out->entry_count);
        return ESP_ERR_INVALID_RESPONSE;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Manifest OK: version %d, generated %lu, %d files",
                out->version, (unsigned long)out->generated, out->entry_count);
    return ESP_OK;
}

esp_err_t manifest_for_each_entry(const char *path, manifest_entry_cb_t cb, void *ctx)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return ESP_ERR_NOT_FOUND;

    char line[MANIFEST_MAX_PATH + MANIFEST_SHA256_LEN * 2 + 24];
    manifest_entry_t e;
    int index = 0;
    esp_err_t err = ESP_OK;
    while (fgets(line, sizeof(line), f) != NULL)
    {
        if (strncmp(line, "sig ", 4) == 0)
            break;
        if (strncmp(line, "f ", 2) != 0)
            continue;
        line[strcspn(line, "\r\n")] = '\0';
        if (!parse_entry_line(line, &e))
        {
            err = ESP_ERR_INVALID_RESPONSE; // can't happen for a file that passed load
            break;
        }
        err = cb(&e, index++, ctx);
        if (err != ESP_OK)
            break;
    }
    fclose(f);
    return err;
}

esp_err_t manifest_hash_begin(manifest_hash_t *ctx)
{
    if (psa_crypto_init() != PSA_SUCCESS) // idempotent
        return ESP_FAIL;
    *ctx = (psa_hash_operation_t)PSA_HASH_OPERATION_INIT;
    return psa_hash_setup(ctx, PSA_ALG_SHA_256) == PSA_SUCCESS ? ESP_OK : ESP_FAIL;
}

esp_err_t manifest_hash_update(manifest_hash_t *ctx, const void *data, size_t len)
{
    return psa_hash_update(ctx, data, len) == PSA_SUCCESS ? ESP_OK : ESP_FAIL;
}

esp_err_t manifest_hash_end(manifest_hash_t *ctx, uint8_t out[MANIFEST_SHA256_LEN])
{
    size_t out_len = 0;
    psa_status_t status = psa_hash_finish(ctx, out, MANIFEST_SHA256_LEN, &out_len);
    return (status == PSA_SUCCESS && out_len == MANIFEST_SHA256_LEN) ? ESP_OK : ESP_FAIL;
}

void manifest_hash_abort(manifest_hash_t *ctx)
{
    psa_hash_abort(ctx);
}

esp_err_t manifest_sha256_file(const char *path, uint8_t out[MANIFEST_SHA256_LEN])
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return ESP_ERR_NOT_FOUND;

    manifest_hash_t ctx;
    if (manifest_hash_begin(&ctx) != ESP_OK)
    {
        fclose(f);
        return ESP_FAIL;
    }

    uint8_t chunk[512];
    size_t n;
    bool hash_error = false;
    while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0)
    {
        if (manifest_hash_update(&ctx, chunk, n) != ESP_OK)
        {
            hash_error = true;
            break;
        }
    }
    bool read_error = ferror(f) != 0;
    fclose(f);

    if (hash_error || read_error)
    {
        manifest_hash_abort(&ctx);
        return ESP_FAIL;
    }
    return manifest_hash_end(&ctx, out);
}

// ---- Small persisted flags (namespace shared with the boot-fail counter) ----

static uint32_t nvs_read_u32(const char *key, uint32_t fallback)
{
    nvs_handle_t h;
    uint32_t value = fallback;
    if (nvs_open(MANIFEST_NVS_NAMESPACE, NVS_READONLY, &h) == ESP_OK)
    {
        nvs_get_u32(h, key, &value);
        nvs_close(h);
    }
    return value;
}

static void nvs_write_u32(const char *key, uint32_t value)
{
    nvs_handle_t h;
    if (nvs_open(MANIFEST_NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Cannot open NVS to write %s", key);
        return;
    }
    nvs_set_u32(h, key, value);
    nvs_commit(h);
    nvs_close(h);
}

uint32_t manifest_get_applied_generation(void)
{
    return nvs_read_u32(NVS_KEY_GENERATION, 0);
}

void manifest_set_applied_generation(uint32_t generated)
{
    nvs_write_u32(NVS_KEY_GENERATION, generated);
}

bool manifest_get_ota_in_progress(void)
{
    return nvs_read_u32(NVS_KEY_OTA_IN_PROGRESS, 0) != 0;
}

void manifest_set_ota_in_progress(bool in_progress)
{
    nvs_write_u32(NVS_KEY_OTA_IN_PROGRESS, in_progress ? 1 : 0);
}

bool manifest_get_self_heal_pending(void)
{
    return nvs_read_u32(NVS_KEY_SELF_HEAL, 0) != 0;
}

void manifest_set_self_heal_pending(bool pending)
{
    nvs_write_u32(NVS_KEY_SELF_HEAL, pending ? 1 : 0);
}
