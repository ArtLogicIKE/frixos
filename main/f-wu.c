#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON.h"
#include "f-wu.h"

#ifndef WU_HOST_TEST
#include "esp_http_client.h"
#include "esp_tls.h"
#include "f-integrations.h"
#include "f-membuffer.h"
#include "f-wifi.h"
#include "frixos.h"

static const char *TAG = "f-wu";
#endif

wu_obs_t wu_obs;

static bool json_number(const cJSON *obj, const char *key, double *out)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (!cJSON_IsNumber(item))
        return false;
    *out = item->valuedouble;
    return true;
}

bool wu_parse_current(const char *json, wu_obs_t *out)
{
    if (!json || !out)
        return false;

    cJSON *root = cJSON_Parse(json);
    if (!root)
        return false;

    cJSON *arr = cJSON_GetObjectItemCaseSensitive(root, "observations");
    if (!cJSON_IsArray(arr) || cJSON_GetArraySize(arr) < 1)
    {
        cJSON_Delete(root);
        return false;
    }

    cJSON *obs = cJSON_GetArrayItem(arr, 0);
    if (!cJSON_IsObject(obs))
    {
        cJSON_Delete(root);
        return false;
    }

    wu_obs_t parsed;
    memset(&parsed, 0, sizeof(parsed));
    parsed.valid = true;

    double epoch = 0;
    if (json_number(obs, "epoch", &epoch) && epoch > 0)
        parsed.obs_epoch = (time_t)epoch;

    parsed.has_hum = json_number(obs, "humidity", &parsed.humidity);

    double winddir = 0;
    if (json_number(obs, "winddir", &winddir))
        parsed.wind_dir_deg = (int)winddir;

    parsed.has_uv = json_number(obs, "uv", &parsed.uv);

    cJSON *metric = cJSON_GetObjectItemCaseSensitive(obs, "metric");
    if (cJSON_IsObject(metric))
    {
        parsed.has_temp = json_number(metric, "temp", &parsed.temp_c);
        parsed.has_dew = json_number(metric, "dewpt", &parsed.dew_c);
        parsed.has_pressure = json_number(metric, "pressure", &parsed.pressure_hpa);
        parsed.has_rain = json_number(metric, "precipTotal", &parsed.rain_mm);

        double kmh = 0;
        if (json_number(metric, "windSpeed", &kmh))
        {
            parsed.wind_mps = kmh / 3.6;
            parsed.has_wind = true;
        }
        if (json_number(metric, "windGust", &kmh))
        {
            parsed.gust_mps = kmh / 3.6;
            parsed.has_gust = true;
        }
    }

    cJSON_Delete(root);
    *out = parsed;
    return true;
}

static void format_temp(char *buf, size_t n, double celsius, bool fahrenheit)
{
    if (fahrenheit)
        snprintf(buf, n, "%.0f°F", (celsius * 9.0 / 5.0) + 32.0);
    else
        snprintf(buf, n, "%.0f°C", celsius);
}

static const char *wind_cardinal(int dir_deg)
{
    static const char *cardinals[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int d = dir_deg % 360;
    if (d < 0)
        d += 360;
    return cardinals[((d + 22) / 45) & 7];
}

static void format_wind(char *buf, size_t n, double mps, int dir_deg, bool fahrenheit)
{
    const char *dir = wind_cardinal(dir_deg);
    if (fahrenheit)
        snprintf(buf, n, "%.1f mph %s", mps * 2.236936, dir);
    else
        snprintf(buf, n, "%.1f m/s %s", mps, dir);
}

bool wu_format_token(const char *token, char *buf, size_t n, bool fahrenheit)
{
    if (!token || !buf || n == 0 || !wu_obs.valid)
        return false;

    if (strcmp(token, "[wu:temp]") == 0 && wu_obs.has_temp)
    {
        format_temp(buf, n, wu_obs.temp_c, fahrenheit);
        return true;
    }
    if (strcmp(token, "[wu:hum]") == 0 && wu_obs.has_hum)
    {
        snprintf(buf, n, "%.0f%%", wu_obs.humidity);
        return true;
    }
    if (strcmp(token, "[wu:dew]") == 0 && wu_obs.has_dew)
    {
        format_temp(buf, n, wu_obs.dew_c, fahrenheit);
        return true;
    }
    if (strcmp(token, "[wu:wind]") == 0 && wu_obs.has_wind)
    {
        format_wind(buf, n, wu_obs.wind_mps, wu_obs.wind_dir_deg, fahrenheit);
        return true;
    }
    if (strcmp(token, "[wu:gust]") == 0 && wu_obs.has_gust)
    {
        format_wind(buf, n, wu_obs.gust_mps, wu_obs.wind_dir_deg, fahrenheit);
        return true;
    }
    if (strcmp(token, "[wu:pressure]") == 0 && wu_obs.has_pressure)
    {
        if (fahrenheit)
            snprintf(buf, n, "%.2f inHg", wu_obs.pressure_hpa * 0.02953);
        else
            snprintf(buf, n, "%.0f hPa", wu_obs.pressure_hpa);
        return true;
    }
    if (strcmp(token, "[wu:rain]") == 0 && wu_obs.has_rain)
    {
        if (fahrenheit)
            snprintf(buf, n, "%.2f in.", wu_obs.rain_mm * 0.0393701);
        else
            snprintf(buf, n, "%.1f mm", wu_obs.rain_mm);
        return true;
    }
    if (strcmp(token, "[wu:uv]") == 0 && wu_obs.has_uv)
    {
        snprintf(buf, n, "%.1f", wu_obs.uv);
        return true;
    }
    return false;
}

bool wu_obs_is_fresh(const wu_obs_t *obs, time_t now)
{
    if (!obs || !obs->valid || obs->obs_epoch <= 0)
        return false;
    if (now < (time_t)1600000000)
        return true;
    if (now < obs->obs_epoch)
        return true;
    return (now - obs->obs_epoch) < 30 * 60;
}

void wu_format_age(const wu_obs_t *obs, time_t now, char *buf, size_t n)
{
    if (!buf || n == 0)
        return;
    if (!obs || !obs->valid || obs->obs_epoch <= 0 || now < (time_t)1600000000)
    {
        snprintf(buf, n, "?");
        return;
    }
    long secs = (long)(now - obs->obs_epoch);
    if (secs < 0)
        secs = 0;
    long mins = secs / 60;
    if (mins < 60)
        snprintf(buf, n, "%ldm", mins);
    else
        snprintf(buf, n, "%ldh", mins / 60);
}

#ifndef WU_HOST_TEST

static char *wu_response_buffer = NULL;
static int wu_response_len = 0;
static esp_http_client_handle_t wu_client = NULL;
static bool wu_client_initialized = false;

static esp_err_t wu_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        wu_response_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        if (wu_response_buffer == NULL)
            return ESP_FAIL;
        if (evt->data_len <= 0 || wu_response_len < 0 ||
            (wu_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
        {
            wu_response_len = 0;
            return ESP_FAIL;
        }
        memcpy(wu_response_buffer + wu_response_len, evt->data, evt->data_len);
        wu_response_len += evt->data_len;
        wu_response_buffer[wu_response_len] = '\0';
        break;
    default:
        break;
    }
    return ESP_OK;
}

static bool init_wu_client(void)
{
    esp_http_client_config_t config = {
        .url = "https://api.weather.com/",
        .method = HTTP_METHOD_GET,
        .timeout_ms = 15000,
        .is_async = false,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .tls_version = ESP_TLS_VER_TLS_1_2,
        .user_agent = "Frixos HTTP Client",
        .event_handler = wu_http_event_handler,
    };

    wu_client = esp_http_client_init(&config);
    if (!wu_client)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "WU client init failed");
        return false;
    }
    esp_http_client_set_header(wu_client, "Accept", "application/json");
    esp_http_client_set_header(wu_client, "connection", "close");
    return true;
}

static void cleanup_wu_client(void)
{
    if (wu_client)
    {
        esp_http_client_cleanup(wu_client);
        wu_client = NULL;
    }
    wu_client_initialized = false;
}

static bool append_query_value(char *dst, size_t dst_size, const char *src)
{
    size_t used = strlen(dst);
    for (; *src; src++)
    {
        unsigned char c = (unsigned char)*src;
        int wrote;
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.')
            wrote = snprintf(dst + used, dst_size - used, "%c", c);
        else
            wrote = snprintf(dst + used, dst_size - used, "%%%02X", c);
        if (wrote < 0 || (size_t)wrote >= dst_size - used)
            return false;
        used += (size_t)wrote;
    }
    return true;
}

bool fetch_wu_observation(void)
{
    if (eeprom_wu_station[0] == '\0' || eeprom_wu_key[0] == '\0')
        return false;
    if (!is_wifi_connected())
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "WiFi down, skip WU");
        return false;
    }

    if (!wu_client_initialized || wu_client == NULL)
    {
        if (wu_client != NULL)
            cleanup_wu_client();
        if (!init_wu_client())
            return false;
        wu_client_initialized = true;
    }

    if (!acquire_ssl_semaphore("fetch_wu_observation"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SSL lock failed (WU)");
        return false;
    }

    wu_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "WU_HTTP");
    if (wu_response_buffer == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "WU buffer failed");
        release_ssl_semaphore();
        return false;
    }
    wu_response_len = 0;

    char urlstr[URL_BUFFER_SIZE];
    int prefix = snprintf(urlstr, sizeof(urlstr),
                          "https://api.weather.com/v2/pws/observations/current?stationId=%s"
                          "&format=json&units=m&numericPrecision=decimal&apiKey=",
                          eeprom_wu_station);
    bool url_ok = prefix > 0 && (size_t)prefix < sizeof(urlstr) &&
                  append_query_value(urlstr, sizeof(urlstr), eeprom_wu_key);
    if (!url_ok)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "WU URL failed");
        release_shared_buffer(wu_response_buffer);
        wu_response_buffer = NULL;
        release_ssl_semaphore();
        cleanup_wu_client();
        return false;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WU fetch %s", eeprom_wu_station);
    esp_http_client_set_url(wu_client, urlstr);

    bool success = false;
    esp_err_t err = esp_http_client_perform(wu_client);
    if (err == ESP_ERR_HTTP_CONNECT || err == ESP_ERR_HTTP_CONNECTING || err == ESP_ERR_HTTP_EAGAIN)
    {
        cleanup_wu_client();
        if (init_wu_client())
        {
            wu_client_initialized = true;
            wu_response_len = 0;
            esp_http_client_set_url(wu_client, urlstr);
            err = esp_http_client_perform(wu_client);
        }
    }

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(wu_client);
        if (status_code == 200 && wu_response_len > 0)
        {
            wu_obs_t parsed;
            if (wu_parse_current(wu_response_buffer, &parsed))
            {
                wu_obs = parsed;
                success = true;
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WU %s temp %.1fC", eeprom_wu_station, wu_obs.temp_c);
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WU %s no observation", eeprom_wu_station);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "WU fetch %d", status_code);
            if (status_code == 401 || status_code == 403)
            {
                cleanup_wu_client();
                wu_client_initialized = false;
            }
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "WU HTTP %s", esp_err_to_name(err));
        cleanup_wu_client();
    }

    release_shared_buffer(wu_response_buffer);
    wu_response_buffer = NULL;
    release_ssl_semaphore();
    cleanup_wu_client();
    return success;
}

#endif /* WU_HOST_TEST */
