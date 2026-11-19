#include <string.h>
#include <stdio.h>
#include <time.h>
#include "esp_mac.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"
#include "mbedtls/md.h"
#include "cJSON.h"
#include "frixos.h"
#include "f-auth.h"

static const char *TAG = "AUTH";

#define AUTH_NS "auth"         // erased by both rescue stages
#define KEY_HASH "pw_hash"     // PBKDF2 result, 32 bytes
#define KEY_SESS "sess_key"    // signs session tokens; replaced when the password changes
#define HASH_LEN 32
#define NONCE_LEN 16
#define NONCE_SLOTS 4
#define NONCE_TTL_US (60LL * 1000000)
#define PBKDF2_ITER 10000
#define SESSION_MAX_AGE_S (30u * 24 * 3600)
#define COOKIE_NAME "fx_session"
#define TOKEN_ID_LEN 8
#define TOKEN_TAG_LEN 16
#define TOKEN_LEN (TOKEN_ID_LEN + 4 + TOKEN_TAG_LEN) // id, issued (unix s), tag
#define FAILS_BEFORE_LOCK 5
#define LOCK_FIRST_S 30
#define LOCK_MAX_S 600

static uint8_t pw_hash[HASH_LEN];
static bool pw_set;
static uint8_t sess_key[HASH_LEN];
static char salt[20]; // "frixos:" + 12 hex digits

// All handlers run in the single httpd task, so this state needs no lock.
static struct
{
    uint8_t v[NONCE_LEN];
    int64_t t;
    bool live;
} nonces[NONCE_SLOTS];
static uint8_t fails;
static uint32_t lock_s;
static int64_t locked_until_us;
static char cookie_hdr[160]; // httpd keeps the pointer until the response is sent

/* ---------- crypto helpers ---------- */

static void sha256_2(const uint8_t *a, size_t al, const uint8_t *b, size_t bl, uint8_t out[32])
{
    mbedtls_md_context_t c;
    mbedtls_md_init(&c);
    mbedtls_md_setup(&c, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
    mbedtls_md_starts(&c);
    mbedtls_md_update(&c, a, al);
    if (bl)
        mbedtls_md_update(&c, b, bl);
    mbedtls_md_finish(&c, out);
    mbedtls_md_free(&c);
}

// HMAC-SHA256 for keys up to 64 bytes (ours are 32).
static void hmac_sha256(const uint8_t *key, size_t kl, const uint8_t *msg, size_t ml, uint8_t out[32])
{
    uint8_t pad[64], inner[32];
    memset(pad, 0x36, sizeof(pad));
    for (size_t i = 0; i < kl; i++)
        pad[i] ^= key[i];
    sha256_2(pad, sizeof(pad), msg, ml, inner);
    memset(pad, 0x5c, sizeof(pad));
    for (size_t i = 0; i < kl; i++)
        pad[i] ^= key[i];
    sha256_2(pad, sizeof(pad), inner, sizeof(inner), out);
}

static bool ct_equal(const uint8_t *a, const uint8_t *b, size_t n)
{
    uint8_t d = 0;
    for (size_t i = 0; i < n; i++)
        d |= a[i] ^ b[i];
    return d == 0;
}

static int nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool hex_decode(const char *s, uint8_t *out, size_t n)
{
    if (!s || strlen(s) != 2 * n)
        return false;
    for (size_t i = 0; i < n; i++)
    {
        int hi = nibble(s[2 * i]), lo = nibble(s[2 * i + 1]);
        if (hi < 0 || lo < 0)
            return false;
        out[i] = (uint8_t)(hi << 4 | lo);
    }
    return true;
}

static void hex_encode(const uint8_t *in, size_t n, char *out)
{
    for (size_t i = 0; i < n; i++)
        sprintf(out + 2 * i, "%02x", in[i]);
    out[2 * n] = '\0';
}

/* ---------- storage ---------- */

void auth_init(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(salt, sizeof(salt), "frixos:%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    pw_set = false;
    nvs_handle_t h;
    if (nvs_open(AUTH_NS, NVS_READONLY, &h) != ESP_OK)
        return; // namespace absent: no password
    size_t len = HASH_LEN;
    bool have_hash = nvs_get_blob(h, KEY_HASH, pw_hash, &len) == ESP_OK && len == HASH_LEN;
    len = HASH_LEN;
    bool have_key = nvs_get_blob(h, KEY_SESS, sess_key, &len) == ESP_OK && len == HASH_LEN;
    nvs_close(h);
    pw_set = have_hash && have_key;
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Access password %s", pw_set ? "set" : "not set");
}

bool auth_enabled(void)
{
    return pw_set;
}

void auth_erase(void)
{
    nvs_handle_t h;
    if (nvs_open(AUTH_NS, NVS_READWRITE, &h) == ESP_OK)
    {
        nvs_erase_all(h);
        nvs_commit(h);
        nvs_close(h);
    }
    pw_set = false;
}

/* ---------- nonces, lockout, sessions ---------- */

static void nonce_new(uint8_t out[NONCE_LEN])
{
    int64_t now = esp_timer_get_time();
    int slot = 0;
    for (int i = 0; i < NONCE_SLOTS; i++)
    {
        if (!nonces[i].live || now - nonces[i].t > NONCE_TTL_US)
        {
            slot = i;
            break;
        }
        if (nonces[i].t < nonces[slot].t)
            slot = i; // all in use: replace the oldest
    }
    esp_fill_random(nonces[slot].v, NONCE_LEN);
    nonces[slot].t = now;
    nonces[slot].live = true;
    memcpy(out, nonces[slot].v, NONCE_LEN);
}

// A nonce is valid once, for NONCE_TTL_US.
static bool nonce_take(const uint8_t n[NONCE_LEN])
{
    int64_t now = esp_timer_get_time();
    for (int i = 0; i < NONCE_SLOTS; i++)
    {
        if (nonces[i].live && now - nonces[i].t <= NONCE_TTL_US && ct_equal(nonces[i].v, n, NONCE_LEN))
        {
            nonces[i].live = false;
            return true;
        }
    }
    return false;
}

// The body proves knowledge of the stored hash: proof = HMAC-SHA256(hash, nonce).
static bool proof_ok(const cJSON *root)
{
    uint8_t nonce[NONCE_LEN], proof[32], want[32];
    const cJSON *n = cJSON_GetObjectItem(root, "nonce");
    const cJSON *p = cJSON_GetObjectItem(root, "proof");
    if (!cJSON_IsString(n) || !cJSON_IsString(p) || !hex_decode(n->valuestring, nonce, NONCE_LEN) ||
        !hex_decode(p->valuestring, proof, sizeof(proof)) || !nonce_take(nonce))
        return false;
    hmac_sha256(pw_hash, HASH_LEN, nonce, NONCE_LEN, want);
    return ct_equal(want, proof, sizeof(want));
}

static int lock_remaining_s(void)
{
    int64_t now = esp_timer_get_time();
    return now < locked_until_us ? (int)((locked_until_us - now) / 1000000) + 1 : 0;
}

static void note_failure(void)
{
    if (++fails < FAILS_BEFORE_LOCK)
        return;
    lock_s = lock_s ? (lock_s * 2 > LOCK_MAX_S ? LOCK_MAX_S : lock_s * 2) : LOCK_FIRST_S;
    locked_until_us = esp_timer_get_time() + (int64_t)lock_s * 1000000;
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Access password: %u failed attempts, locked for %lus", fails, (unsigned long)lock_s);
}

static void note_success(void)
{
    fails = 0;
    lock_s = 0;
}

// Tokens are signed, not stored: they survive restarts, and a new sess_key revokes them all.
static bool token_ok(const char *hex)
{
    uint8_t b[TOKEN_LEN], tag[32];
    if (!hex_decode(hex, b, TOKEN_LEN))
        return false;
    hmac_sha256(sess_key, HASH_LEN, b, TOKEN_ID_LEN + 4, tag);
    if (!ct_equal(tag, b + TOKEN_ID_LEN + 4, TOKEN_TAG_LEN))
        return false;
    uint32_t issued;
    memcpy(&issued, b + TOKEN_ID_LEN, 4);
    // Issued before the clock was set (0): valid until the password changes.
    if (issued && time_valid && (uint32_t)time(NULL) - issued > SESSION_MAX_AGE_S)
        return false;
    return true;
}

static bool session_ok(httpd_req_t *req)
{
    char v[2 * TOKEN_LEN + 8];
    size_t len = sizeof(v);
    return httpd_req_get_cookie_val(req, COOKIE_NAME, v, &len) == ESP_OK && token_ok(v);
}

static void set_session_cookie(httpd_req_t *req)
{
    uint8_t b[TOKEN_LEN], tag[32];
    char tok[2 * TOKEN_LEN + 1];
    esp_fill_random(b, TOKEN_ID_LEN);
    uint32_t issued = time_valid ? (uint32_t)time(NULL) : 0;
    memcpy(b + TOKEN_ID_LEN, &issued, 4);
    hmac_sha256(sess_key, HASH_LEN, b, TOKEN_ID_LEN + 4, tag);
    memcpy(b + TOKEN_ID_LEN + 4, tag, TOKEN_TAG_LEN);
    hex_encode(b, TOKEN_LEN, tok);
    snprintf(cookie_hdr, sizeof(cookie_hdr), COOKIE_NAME "=%s; Path=/; Max-Age=%u; HttpOnly; SameSite=Strict", tok,
             SESSION_MAX_AGE_S);
    httpd_resp_set_hdr(req, "Set-Cookie", cookie_hdr);
}

static void clear_session_cookie(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Set-Cookie", COOKIE_NAME "=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict");
}

/* ---------- HTTP ---------- */

static esp_err_t send_json(httpd_req_t *req, const char *status, const char *json)
{
    if (status)
        httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t send_locked(httpd_req_t *req, int secs)
{
    char out[96];
    snprintf(out, sizeof(out), "{\"status\":\"error\",\"message\":\"locked\",\"locked\":%d}", secs);
    return send_json(req, "429 Too Many Requests", out);
}

static cJSON *read_body(httpd_req_t *req)
{
    char buf[320];
    if (req->content_len <= 0 || req->content_len >= sizeof(buf))
        return NULL;
    int got = 0;
    while (got < (int)req->content_len)
    {
        int r = httpd_req_recv(req, buf + got, req->content_len - got);
        if (r == HTTPD_SOCK_ERR_TIMEOUT)
            continue;
        if (r <= 0)
            return NULL;
        got += r;
    }
    buf[got] = '\0';
    return cJSON_Parse(buf);
}

// Path without the query string, compared exactly.
static bool path_is(const char *uri, const char *path)
{
    size_t n = strlen(path);
    return strncmp(uri, path, n) == 0 && (uri[n] == '\0' || uri[n] == '?');
}

// Reachable without a session: the login page and what it loads, and the login API.
static bool is_public(const char *uri)
{
    static const char *const open[] = {"/login.html", "/js/auth.js", "/logo.jpg", "/favicon.ico",
                                       "/api/auth/challenge", "/api/auth/login", "/api/auth/logout"};
    for (size_t i = 0; i < sizeof(open) / sizeof(open[0]); i++)
        if (path_is(uri, open[i]))
            return true;
    return false;
}

// A browser navigation (a page, or a captive-portal probe), as opposed to a file or API call.
static bool wants_page(const char *uri)
{
    if (strncmp(uri, "/api/", 5) == 0)
        return false;
    const char *end = strchr(uri, '?');
    size_t len = end ? (size_t)(end - uri) : strlen(uri);
    const char *slash = uri;
    for (const char *p = uri; p < uri + len; p++)
        if (*p == '/')
            slash = p;
    const char *dot = memchr(slash, '.', uri + len - slash);
    return !dot || (uri + len - dot == 5 && strncmp(dot, ".html", 5) == 0);
}

esp_err_t auth_gate(httpd_req_t *req)
{
    esp_err_t (*handler)(httpd_req_t *) = (esp_err_t(*)(httpd_req_t *))req->user_ctx;
    if (!pw_set || is_public(req->uri) || session_ok(req))
        return handler(req);

    if (req->method == HTTP_GET && wants_page(req->uri))
    {
        httpd_resp_set_status(req, "302 Found");
        httpd_resp_set_hdr(req, "Location", "/login.html");
        httpd_resp_set_hdr(req, "Cache-Control", "no-store");
        return httpd_resp_send(req, NULL, 0);
    }
    return send_json(req, "401 Unauthorized", "{\"status\":\"error\",\"message\":\"auth_required\"}");
}

esp_err_t auth_challenge_handler(httpd_req_t *req)
{
    uint8_t n[NONCE_LEN];
    char nh[2 * NONCE_LEN + 1], out[192];
    nonce_new(n);
    hex_encode(n, NONCE_LEN, nh);
    snprintf(out, sizeof(out), "{\"auth\":%s,\"salt\":\"%s\",\"iter\":%d,\"nonce\":\"%s\",\"locked\":%d}",
             pw_set ? "true" : "false", salt, PBKDF2_ITER, nh, lock_remaining_s());
    return send_json(req, NULL, out);
}

esp_err_t auth_login_handler(httpd_req_t *req)
{
    if (!pw_set)
        return send_json(req, NULL, "{\"status\":\"ok\"}");
    int lock = lock_remaining_s();
    if (lock)
        return send_locked(req, lock);
    cJSON *root = read_body(req);
    bool ok = root && proof_ok(root);
    cJSON_Delete(root);
    if (!ok)
    {
        note_failure();
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Access password: failed login");
        char out[96];
        snprintf(out, sizeof(out), "{\"status\":\"error\",\"message\":\"wrong_password\",\"locked\":%d}",
                 lock_remaining_s());
        return send_json(req, "401 Unauthorized", out);
    }
    note_success();
    set_session_cookie(req);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Access password: login");
    return send_json(req, NULL, "{\"status\":\"ok\"}");
}

esp_err_t auth_logout_handler(httpd_req_t *req)
{
    clear_session_cookie(req);
    return send_json(req, NULL, "{\"status\":\"ok\"}");
}

// Body: {"hash": "<64 hex>" or "" to remove, "nonce", "proof"}; the proof (of the
// current password) is required when one is set.
esp_err_t auth_password_handler(httpd_req_t *req)
{
    int lock = pw_set ? lock_remaining_s() : 0;
    if (lock)
        return send_locked(req, lock);
    cJSON *root = read_body(req);
    const cJSON *hash = root ? cJSON_GetObjectItem(root, "hash") : NULL;
    uint8_t new_hash[HASH_LEN];
    bool remove = cJSON_IsString(hash) && hash->valuestring[0] == '\0';
    if (!cJSON_IsString(hash) || (!remove && !hex_decode(hash->valuestring, new_hash, HASH_LEN)))
    {
        cJSON_Delete(root);
        return send_json(req, "400 Bad Request", "{\"status\":\"error\",\"message\":\"invalid hash\"}");
    }
    if (pw_set)
    {
        bool ok = proof_ok(root);
        if (!ok)
        {
            cJSON_Delete(root);
            note_failure();
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Access password: change refused, wrong current password");
            return send_json(req, "403 Forbidden", "{\"status\":\"error\",\"message\":\"wrong_password\"}");
        }
        note_success();
    }
    cJSON_Delete(root);

    if (remove)
    {
        auth_erase();
        clear_session_cookie(req);
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Access password removed");
        return send_json(req, NULL, "{\"status\":\"ok\",\"auth\":false}");
    }

    uint8_t key[HASH_LEN];
    esp_fill_random(key, sizeof(key));
    nvs_handle_t h;
    esp_err_t err = nvs_open(AUTH_NS, NVS_READWRITE, &h);
    if (err == ESP_OK)
    {
        err = nvs_set_blob(h, KEY_HASH, new_hash, HASH_LEN);
        if (err == ESP_OK)
            err = nvs_set_blob(h, KEY_SESS, key, sizeof(key));
        if (err == ESP_OK)
            err = nvs_commit(h);
        nvs_close(h);
    }
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Access password: save failed: %s", esp_err_to_name(err));
        return send_json(req, "500 Internal Server Error", "{\"status\":\"error\",\"message\":\"save failed\"}");
    }
    memcpy(pw_hash, new_hash, HASH_LEN);
    memcpy(sess_key, key, sizeof(key)); // logs out every other session
    pw_set = true;
    set_session_cookie(req);            // the browser that set it stays logged in
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Access password set");
    return send_json(req, NULL, "{\"status\":\"ok\",\"auth\":true}");
}
