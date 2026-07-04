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

static esp_err_t verify_signature(const uint8_t *region, size_t region_len,
                                  const uint8_t sig[MANIFEST_SIG_RAW_LEN])
{
    psa_status_t status = psa_crypto_init(); // idempotent
    if (status != PSA_SUCCESS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "psa_crypto_init failed: %d", (int)status);
        return ESP_FAIL;
    }

    uint8_t hash[MANIFEST_SHA256_LEN];
    size_t hash_len = 0;
    status = psa_hash_compute(PSA_ALG_SHA_256, region, region_len,
                              hash, sizeof(hash), &hash_len);
    if (status != PSA_SUCCESS || hash_len != sizeof(hash))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest hash failed: %d", (int)status);
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
                             hash, hash_len, sig, MANIFEST_SIG_RAW_LEN);
    psa_destroy_key(key);
    if (status != PSA_SUCCESS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest signature verification FAILED: %d", (int)status);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

esp_err_t manifest_load_and_verify(const char *path, manifest_t *out)
{
    memset(out, 0, sizeof(*out));

    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return ESP_ERR_NOT_FOUND;
    struct stat st;
    if (stat(path, &st) != 0 || st.st_size <= 0 || st.st_size > MANIFEST_MAX_SIZE)
    {
        fclose(f);
        return ESP_ERR_INVALID_SIZE;
    }

    char *buf = malloc(st.st_size + 1);
    if (buf == NULL)
    {
        fclose(f);
        return ESP_ERR_NO_MEM;
    }
    size_t got = fread(buf, 1, st.st_size, f);
    fclose(f);
    if (got != (size_t)st.st_size)
    {
        free(buf);
        return ESP_FAIL;
    }
    buf[got] = '\0';

    // Locate the trailing signature line; everything before it is signed.
    char *sig_line = strstr(buf, "\nsig ");
    if (sig_line == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest has no signature line");
        free(buf);
        return ESP_ERR_INVALID_RESPONSE;
    }
    size_t region_len = (sig_line - buf) + 1; // include the '\n'
    char *sig_hex = sig_line + 5;
    char *sig_end = strchr(sig_hex, '\n');
    if (sig_end)
        *sig_end = '\0';

    uint8_t sig_raw[MANIFEST_SIG_RAW_LEN];
    if (strlen(sig_hex) != MANIFEST_SIG_RAW_LEN * 2 ||
        !parse_hex(sig_hex, sig_raw, MANIFEST_SIG_RAW_LEN))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest signature not valid hex");
        free(buf);
        return ESP_ERR_INVALID_RESPONSE;
    }

    esp_err_t err = verify_signature((const uint8_t *)buf, region_len, sig_raw);
    if (err != ESP_OK)
    {
        free(buf);
        return err;
    }

    // Signature is good - parse the (now trusted) region line by line.
    buf[region_len - 1] = '\0'; // terminate the signed region at its last '\n'
    if (strncmp(buf, MANIFEST_MAGIC, strlen(MANIFEST_MAGIC)) != 0)
    {
        free(buf);
        return ESP_ERR_INVALID_RESPONSE;
    }

    int capacity = 0;
    bool have_fw = false, malformed = false;
    char *saveptr = NULL;
    for (char *line = strtok_r(buf, "\n", &saveptr); line != NULL;
         line = strtok_r(NULL, "\n", &saveptr))
    {
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
            // "f <sha256hex> <size> <path>"
            char *hex = line + 2;
            char *sp1 = strchr(hex, ' ');
            char *sp2 = sp1 ? strchr(sp1 + 1, ' ') : NULL;
            if (sp1 == NULL || sp2 == NULL || (sp1 - hex) != MANIFEST_SHA256_LEN * 2 ||
                strlen(sp2 + 1) == 0 || strlen(sp2 + 1) >= MANIFEST_MAX_PATH)
            {
                malformed = true;
                break;
            }
            if (out->entry_count == capacity)
            {
                capacity = capacity ? capacity * 2 : 32;
                manifest_entry_t *grown = realloc(out->entries, capacity * sizeof(*grown));
                if (grown == NULL)
                {
                    malformed = true;
                    break;
                }
                out->entries = grown;
            }
            manifest_entry_t *e = &out->entries[out->entry_count];
            if (!parse_hex(hex, e->sha256, MANIFEST_SHA256_LEN))
            {
                malformed = true;
                break;
            }
            e->size = (uint32_t)strtoul(sp1 + 1, NULL, 10);
            strlcpy(e->path, sp2 + 1, sizeof(e->path));
            out->entry_count++;
        }
    }
    free(buf);

    if (malformed || !have_fw || out->version <= 0 || out->generated == 0 || out->entry_count == 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Manifest verified but malformed (fw:%d ver:%d gen:%lu n:%d)",
                    have_fw, out->version, (unsigned long)out->generated, out->entry_count);
        manifest_free(out);
        return ESP_ERR_INVALID_RESPONSE;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Manifest OK: version %d, generated %lu, %d files",
                out->version, (unsigned long)out->generated, out->entry_count);
    return ESP_OK;
}

void manifest_free(manifest_t *m)
{
    free(m->entries);
    m->entries = NULL;
    m->entry_count = 0;
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
