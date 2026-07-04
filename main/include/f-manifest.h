/* Signed OTA manifest (frixos-manifest-v1) parsing and verification.
 *
 * A manifest is a small text file published per release by push.py:
 *
 *   frixos-manifest-v1
 *   version <N>
 *   generated <unix-ts>            monotonic per publish; devices reject older
 *   fw revE<N>.bin <size> <sha256hex>
 *   f <sha256hex> <size> <relpath>
 *   ...
 *   sig <hex raw 64-byte ECDSA-P256 r||s signature>
 *
 * The signature covers every byte before the "sig" line and is verified
 * against the compiled-in public key (f-ota-pubkey.h). Only a manifest that
 * verifies is ever acted upon; the update process installs nothing unsigned.
 * (Local uploads through /api/ota are a separate, deliberately open path.)
 */
#ifndef F_MANIFEST_H
#define F_MANIFEST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "psa/crypto.h"

#define MANIFEST_PATH "/spiffs/manifest.txt"
#define MANIFEST_SHA256_LEN 32
#define MANIFEST_MAX_PATH 96

typedef struct
{
    uint8_t sha256[MANIFEST_SHA256_LEN];
    uint32_t size;
    char path[MANIFEST_MAX_PATH]; // relative, forward slashes (e.g. "js/core.js")
} manifest_entry_t;

typedef struct
{
    int version;
    uint32_t generated; // unix timestamp stamped by push.py
    uint8_t fw_sha256[MANIFEST_SHA256_LEN];
    uint32_t fw_size;
    int entry_count; // number of "f" lines (entries stay on disk; see below)
} manifest_t;

/* Load a manifest file, verify its signature against the embedded public
 * key, and parse/validate it. Returns ESP_OK only for an authentic,
 * well-formed manifest; ESP_ERR_INVALID_RESPONSE for a bad signature or
 * format. File entries are validated but NOT kept in RAM (a ~100-entry
 * array cost ~14 KB and failed to allocate in the low-heap boot window);
 * iterate them from disk with manifest_for_each_entry. */
esp_err_t manifest_load_and_verify(const char *path, manifest_t *out);

/* Stream the "f" entries of an ALREADY-VERIFIED manifest file one at a
 * time (tiny stack footprint). The callback returns ESP_OK to continue;
 * any other value stops the walk and is returned. `index` counts entries
 * from 0. */
typedef esp_err_t (*manifest_entry_cb_t)(const manifest_entry_t *e, int index, void *ctx);
esp_err_t manifest_for_each_entry(const char *path, manifest_entry_cb_t cb, void *ctx);

/* SHA-256 of a file on SPIFFS (streamed). ESP_ERR_NOT_FOUND if absent. */
esp_err_t manifest_sha256_file(const char *path, uint8_t out[MANIFEST_SHA256_LEN]);

/* Streaming SHA-256 wrappers over PSA Crypto (mbedTLS 4 removed the
 * mbedtls/sha256.h low-level API). Used by f-ota.c to hash downloads
 * as they stream in. */
typedef psa_hash_operation_t manifest_hash_t;
esp_err_t manifest_hash_begin(manifest_hash_t *ctx);
esp_err_t manifest_hash_update(manifest_hash_t *ctx, const void *data, size_t len);
esp_err_t manifest_hash_end(manifest_hash_t *ctx, uint8_t out[MANIFEST_SHA256_LEN]);
void manifest_hash_abort(manifest_hash_t *ctx);

/* Downgrade protection: last applied "generated" stamp, persisted in NVS.
 * Returns 0 when none has been recorded yet. */
uint32_t manifest_get_applied_generation(void);
void manifest_set_applied_generation(uint32_t generated);

/* "OTA writes in flight" flag, persisted in NVS. While set, the boot-loop
 * auto-rescue threshold is raised (see check_boot_fail_count in main.c):
 * an interrupted update is safely resumed by self-heal, so crash-reboots
 * during one must not wipe the user's settings. */
bool manifest_get_ota_in_progress(void);
void manifest_set_ota_in_progress(bool in_progress);

/* Set before rebooting into a freshly installed firmware; tells the first
 * boot to verify all SPIFFS files against the stored manifest. */
bool manifest_get_self_heal_pending(void);
void manifest_set_self_heal_pending(bool pending);

#endif // F_MANIFEST_H
