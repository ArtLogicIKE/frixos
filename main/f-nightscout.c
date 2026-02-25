#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "mbedtls/md.h"
#include "frixos.h"
#include "f-integrations.h"
#include "f-membuffer.h"
#include "f-nightscout.h"
#include "cJSON.h"

static const char *TAG = "f-nightscout";

extern glucose_data_t glucose_data;

static char *nightscout_response_buffer = NULL;
static int nightscout_response_len = 0;
static esp_http_client_handle_t nightscout_client = NULL;
bool nightscout_client_initialized = false;

static esp_err_t nightscout_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Nightscout HTTP Error");
        nightscout_response_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        if (nightscout_response_buffer == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer is NULL");
            return ESP_FAIL;
        }
        if (evt->data_len <= 0 || nightscout_response_len < 0 ||
            (nightscout_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer overflow prevented: len=%d, data_len=%d, max=%d",
                        nightscout_response_len, evt->data_len, HTTP_BUFFER_SIZE);
            nightscout_response_len = 0;
            return ESP_FAIL;
        }
        memcpy(nightscout_response_buffer + nightscout_response_len, evt->data, evt->data_len);
        nightscout_response_len += evt->data_len;
        nightscout_response_buffer[nightscout_response_len] = '\0';
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Nightscout HTTP Event: FINISH");
        break;
    default:
        break;
    }
    return ESP_OK;
}

/** Compute SHA1 of password and write hex string into hash_hex (at least 41 bytes). */
static void sha1_hex(const char *password, char *hash_hex)
{
    unsigned char digest[20];
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);

    if (!md_info || !password)
    {
        hash_hex[0] = '\0';
        return;
    }
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, md_info, 0);
    mbedtls_md_starts(&ctx);
    mbedtls_md_update(&ctx, (const unsigned char *)password, (size_t)strlen(password));
    mbedtls_md_finish(&ctx, digest);
    mbedtls_md_free(&ctx);

    for (int i = 0; i < 20; i++)
        sprintf(hash_hex + i * 2, "%02x", digest[i]);
    hash_hex[40] = '\0';
}

/** Map Nightscout direction string to trend_arrow: 0=down fast, 1=down, 2=stable, 3=up, 4=up fast, -1=unknown */
static int direction_to_trend(const char *direction)
{
    if (!direction)
        return -1;
    if (strcmp(direction, "DoubleDown") == 0 || strcmp(direction, "SingleDown") == 0)
        return 0;
    if (strcmp(direction, "FortyFiveDown") == 0)
        return 1;
    if (strcmp(direction, "Flat") == 0)
        return 2;
    if (strcmp(direction, "FortyFiveUp") == 0)
        return 3;
    if (strcmp(direction, "SingleUp") == 0 || strcmp(direction, "DoubleUp") == 0)
        return 4;
    return -1;
}

bool init_nightscout_client(void)
{
    if (eeprom_ns_url[0] == '\0' || eeprom_glucose_password[0] == '\0')
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Nightscout URL or API secret (password) not set");
        return false;
    }

    glucose_data.trend_arrow = -1;

    esp_http_client_config_t config = {
        .url = eeprom_ns_url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 10000,
        .is_async = false,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .tls_version = ESP_TLS_VER_TLS_1_2,
        .user_agent = "Frixos HTTP Client",
        .event_handler = nightscout_http_event_handler,
        .buffer_size = HTTP_BUFFER_SIZE,
        .buffer_size_tx = HTTP_BUFFER_SIZE,
    };

    nightscout_client = esp_http_client_init(&config);
    if (!nightscout_client)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize Nightscout client");
        return false;
    }

    esp_http_client_set_header(nightscout_client, "Accept", "application/json");
    esp_http_client_set_header(nightscout_client, "connection", "close");

    return true;
}

void cleanup_nightscout_client(void)
{
    if (nightscout_client)
    {
        esp_http_client_cleanup(nightscout_client);
        nightscout_client = NULL;
    }
    nightscout_client_initialized = false;
}

bool fetch_nightscout_glucose(void)
{
    if (!nightscout_client_initialized)
    {
        if (!init_nightscout_client())
            return false;
        nightscout_client_initialized = true;
    }

    char url_buf[256];
    size_t base_len = strlen(eeprom_ns_url);
    if (base_len >= sizeof(url_buf) - 32)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Nightscout URL too long");
        return false;
    }
    strncpy(url_buf, eeprom_ns_url, sizeof(url_buf) - 1);
    url_buf[sizeof(url_buf) - 1] = '\0';
    if (base_len > 0 && url_buf[base_len - 1] == '/')
        url_buf[base_len - 1] = '\0';
    strncat(url_buf, "/api/v1/entries.json?count=2", sizeof(url_buf) - strlen(url_buf) - 1);

    char api_secret_hex[41];
    // only set api-secret if password is set
    if (strlen(eeprom_glucose_password) > 0)
    {
        sha1_hex(eeprom_glucose_password, api_secret_hex);
        esp_http_client_set_header(nightscout_client, "api-secret", api_secret_hex);
    }

    esp_http_client_set_url(nightscout_client, url_buf);
    esp_http_client_set_method(nightscout_client, HTTP_METHOD_GET);

    if (!acquire_ssl_semaphore("fetch_nightscout_glucose"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for fetch_nightscout_glucose");
        return false;
    }

    nightscout_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "NIGHTSCOUT_HTTP");
    if (!nightscout_response_buffer)
    {
        release_ssl_semaphore();
        return false;
    }
    nightscout_response_len = 0;

    bool success = false;
    esp_err_t err = esp_http_client_perform(nightscout_client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(nightscout_client);
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Nightscout glucose fetch code %d", status_code);

        if (status_code == 200 && nightscout_response_len > 0)
        {
            cJSON *root = cJSON_Parse(nightscout_response_buffer);
            if (root && cJSON_IsArray(root) && cJSON_GetArraySize(root) >= 1)
            {
                cJSON *latest = cJSON_GetArrayItem(root, 0);
                cJSON *previous = cJSON_GetArraySize(root) > 1 ? cJSON_GetArrayItem(root, 1) : NULL;

                cJSON *sgv = cJSON_GetObjectItem(latest, "sgv");
                cJSON *date_item = cJSON_GetObjectItem(latest, "date");
                cJSON *direction = cJSON_GetObjectItem(latest, "direction");

                if (cJSON_IsNumber(sgv))
                {
                    float value = (float)sgv->valuedouble;
                    time_t ts = 0;
                    if (cJSON_IsNumber(date_item))
                        ts = (time_t)((int64_t)(date_item->valuedouble) / 1000);
                    else if (cJSON_IsString(date_item))
                        ts = (time_t)(atoll(date_item->valuestring) / 1000);

                    glucose_data.previous_gl_mgdl = glucose_data.current_gl_mgdl;
                    glucose_data.current_gl_mgdl = value;
                    glucose_data.timestamp = ts;

                    if (previous)
                    {
                        cJSON *prev_sgv = cJSON_GetObjectItem(previous, "sgv");
                        if (cJSON_IsNumber(prev_sgv))
                            glucose_data.previous_gl_mgdl = (float)prev_sgv->valuedouble;
                    }
                    else
                    {
                        glucose_data.previous_gl_mgdl = glucose_data.current_gl_mgdl;
                    }

                    glucose_data.gl_diff = glucose_data.current_gl_mgdl - glucose_data.previous_gl_mgdl;
                    glucose_data.trend_arrow = direction_to_trend(cJSON_IsString(direction) ? direction->valuestring : NULL);
                    success = true;

                    if (eeprom_glucose_unit == 1)
                        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "fetch_nightscout_glucose: ok, glucose: %.1f mmol/L, trend: %i",
                                    glucose_data.current_gl_mgdl / 18.0182f, glucose_data.trend_arrow);
                    else
                        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "fetch_nightscout_glucose: ok, glucose: %.0f mg/dL, trend: %i",
                                    glucose_data.current_gl_mgdl, glucose_data.trend_arrow);
                }
            }
            if (root)
                cJSON_Delete(root);
        }
        else if (status_code == 401)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Nightscout unauthorized (check API secret)");
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Nightscout glucose fetch returned http %d", status_code);
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Nightscout glucose fetch failed: %s", esp_err_to_name(err));
    }

    release_shared_buffer(nightscout_response_buffer);
    nightscout_response_buffer = NULL;
    release_ssl_semaphore();
    return success;
}
