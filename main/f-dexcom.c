#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "mbedtls/ssl.h"
#include "frixos.h"
#include "f-integrations.h"
#include "f-membuffer.h"
#include "f-json.h"
#include "f-dexcom.h"
#include "frixos.h"
#include "cJSON.h"

static const char *TAG = "f-dexcom";

// Dexcom API endpoints based on region
static const char *DEXCOM_BASE_URLS[] = {
    NULL,                          // Disabled
    "https://share2.dexcom.com",   // US
    "https://share2.dexcom.jp",    // Japan
    "https://shareous1.dexcom.com" // Rest of World
};

// Global variables
// Use unified glucose_data instead of separate dexcom_data
extern glucose_data_t glucose_data;
char dexcom_account_id[64] = {0};
char dexcom_session_id[64] = {0};

// Buffer for Dexcom responses
static char *dexcom_response_buffer = NULL;
static int dexcom_response_len = 0;

// Persistent HTTP client for Dexcom
static esp_http_client_handle_t dexcom_client = NULL;
bool dexcom_client_initialized = false;

// HTTP event handler for Dexcom requests
static esp_err_t dexcom_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom HTTP Error");
        dexcom_response_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        // Ensure we have a valid buffer and the data will fit
        if (dexcom_response_buffer == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer is NULL");
            return ESP_FAIL;
        }

        // Check if we have enough space
        if (evt->data_len <= 0 || dexcom_response_len < 0 ||
            (dexcom_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer overflow prevented: len=%d, data_len=%d, max=%d",
                        dexcom_response_len, evt->data_len, HTTP_BUFFER_SIZE);
            dexcom_response_len = 0;
            return ESP_FAIL;
        }

        // Safe to copy the data
        memcpy(dexcom_response_buffer + dexcom_response_len, evt->data, evt->data_len);
        dexcom_response_len += evt->data_len;
        dexcom_response_buffer[dexcom_response_len] = '\0';
        //ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Dexcom HTTP Event: ON_DATA, data_len=%d, total_len=%d, data: %.*s", 
        //            evt->data_len, dexcom_response_len, evt->data_len, evt->data);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom HTTP Event: FINISH");
        break;
    default:        
        break;
    }
    return ESP_OK;
}

bool init_dexcom_client(void)
{
    const char *base_url = DEXCOM_BASE_URLS[eeprom_dexcom_region];
    if (!base_url)
        return false;

    glucose_data.trend_arrow = -1;

    // Initialize the client with base configuration
    esp_http_client_config_t config = {
        .url = base_url, // Just use the base URL initially
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .is_async = false,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .tls_version = ESP_TLS_VER_TLS_1_2,
        .user_agent = "Frixos HTTP Client",
        .event_handler = dexcom_http_event_handler,
    };

    dexcom_client = esp_http_client_init(&config);
    if (!dexcom_client)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize Dexcom client");
        return false;
    }

    // Set common headers
    esp_http_client_set_header(dexcom_client, "Content-Type", "application/json");
    esp_http_client_set_header(dexcom_client, "Accept", "application/json");
    esp_http_client_set_header(dexcom_client, "connection", "close");

    return true;
}

void cleanup_dexcom_client(void)
{
    if (dexcom_client)
    {
        esp_http_client_cleanup(dexcom_client);
        dexcom_client = NULL;
    }
    dexcom_client_initialized = false;
}

bool authenticate_dexcom_account(void)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Authenticating Dexcom to get account ID");
    const char *base_url = DEXCOM_BASE_URLS[eeprom_dexcom_region];
    if (!base_url)
        return false;

    // Initialize client if not already done
    if (!dexcom_client_initialized)
    {
        if (!init_dexcom_client())
        {
            return false;
        }
        dexcom_client_initialized = true;
    }

    char url[256];
    snprintf(url, sizeof(url), "%s/ShareWebServices/Services/General/AuthenticatePublisherAccount", base_url);

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("authenticate_dexcom_account"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for authenticate_dexcom_account");
        return false;
    }

    dexcom_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "DEXCOM_HTTP");
    if (!dexcom_response_buffer)
    {
        release_ssl_semaphore();
        return false;
    }
    dexcom_response_len = 0;

    // Update URL and method for this request
    esp_http_client_set_url(dexcom_client, url);
    esp_http_client_set_method(dexcom_client, HTTP_METHOD_POST);

    char auth_data[256];
    snprintf(auth_data, sizeof(auth_data),
             "{\"accountName\":\"%s\",\"password\":\"%s\",\"applicationId\":\"d89443d2-327c-4a6f-89e5-496bbb0317db\"}",
             eeprom_glucose_username, eeprom_glucose_password);

    esp_http_client_set_post_field(dexcom_client, auth_data, strlen(auth_data));

    bool success = false;
    if (esp_http_client_perform(dexcom_client) == ESP_OK)
    {
        if (esp_http_client_get_status_code(dexcom_client) == 200 && dexcom_response_len > 0)
        {
            // Response is the account ID, copy it
            int len = min(dexcom_response_len, sizeof(dexcom_account_id) - 1);
            memcpy(dexcom_account_id, dexcom_response_buffer, len);
            dexcom_account_id[len] = '\0';

            // Remove quotes
            if (dexcom_account_id[0] == '"')
            {
                memmove(dexcom_account_id, dexcom_account_id + 1, strlen(dexcom_account_id));
                dexcom_account_id[strlen(dexcom_account_id) - 1] = '\0';
            }
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom account auth successful, ID: %s", dexcom_account_id);
            success = true;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom account auth failed, status: %d", esp_http_client_get_status_code(dexcom_client));
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom account auth HTTP request failed");
    }

    release_shared_buffer(dexcom_response_buffer);
    dexcom_response_buffer = NULL;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    return success;
}

bool login_dexcom(void)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Logging into Dexcom to get session ID");
    const char *base_url = DEXCOM_BASE_URLS[eeprom_dexcom_region];
    if (!base_url)
        return false;

    char url[256];
    snprintf(url, sizeof(url), "%s/ShareWebServices/Services/General/LoginPublisherAccountById", base_url);

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("login_dexcom"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for login_dexcom");
        return false;
    }

    dexcom_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "DEXCOM_HTTP");
    if (!dexcom_response_buffer)
    {
        release_ssl_semaphore();
        return false;
    }
    dexcom_response_len = 0;

    // Update URL and method for this request
    esp_http_client_set_url(dexcom_client, url);
    esp_http_client_set_method(dexcom_client, HTTP_METHOD_POST);

    char auth_data[256];
    snprintf(auth_data, sizeof(auth_data),
             "{\"accountId\":\"%s\",\"password\":\"%s\",\"applicationId\":\"d89443d2-327c-4a6f-89e5-496bbb0317db\"}",
             dexcom_account_id, eeprom_glucose_password);

    esp_http_client_set_post_field(dexcom_client, auth_data, strlen(auth_data));

    bool success = false;
    if (esp_http_client_perform(dexcom_client) == ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom login response: %s", dexcom_response_buffer);
        if (esp_http_client_get_status_code(dexcom_client) == 200 && dexcom_response_len > 0)
        {
            int len = min(dexcom_response_len, sizeof(dexcom_session_id) - 1);
            memcpy(dexcom_session_id, dexcom_response_buffer, len);
            dexcom_session_id[len] = '\0';

            if (dexcom_session_id[0] == '"')
            {
                memmove(dexcom_session_id, dexcom_session_id + 1, strlen(dexcom_session_id));
                dexcom_session_id[strlen(dexcom_session_id) - 1] = '\0';
            }
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom login successful, session ID: %s", dexcom_session_id);
            success = true;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom login failed, status: %d, response: %s", esp_http_client_get_status_code(dexcom_client), dexcom_response_buffer);
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom login HTTP request failed");
    }

    release_shared_buffer(dexcom_response_buffer);
    dexcom_response_buffer = NULL;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    return success;
}

bool fetch_dexcom_glucose()
{
    // Initialize client if not already done
    if (!dexcom_client_initialized)
    {
        if (!init_dexcom_client())
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize Dexcom client");
            return false;
        }
        dexcom_client_initialized = true;
    }



    // Step 1: Get Account ID if we don't have it
    // this shouldn't be needed, but just in case
    if (dexcom_account_id[0] == '\0') {
        if (!authenticate_dexcom_account()) {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Aborting Dexcom fetch: Could not get Account ID.");
            return false;
        }
    }


    for (int i = 0; i < 2; i++)
    { // Allow one retry on session failure
      // Step 2: Get Session ID if we don't have it
        if (dexcom_session_id[0] == '\0')
        {
            if (!login_dexcom())
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Aborting Dexcom fetch: Could not log in.");
                return false;
            }
        }

        // Step 3: Fetch glucose data
        const char *base_url = DEXCOM_BASE_URLS[eeprom_dexcom_region];
        char url[384];
        snprintf(url, sizeof(url), "%s/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues?sessionId=%s&minutes=1440&maxCount=2", base_url, dexcom_session_id);
        //snprintf(url, sizeof(url), "https://share2.dexcom.com/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues?sessionId=e629db54-0854-48d6-8e96-2252fe66750a&minutes=1440&maxCount=4");

        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom glucose fetch URL: %s", url);
        
        // Acquire SSL connection semaphore before making SSL connection
        if (!acquire_ssl_semaphore("fetch_dexcom_glucose"))
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for fetch_dexcom_glucose");
            return false;
        }
        
        dexcom_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "DEXCOM_HTTP");
        if (!dexcom_response_buffer)
        {
            release_ssl_semaphore();
            return false;
        }
        dexcom_response_len = 0;
        strcpy(dexcom_response_buffer, "EMPTY BUFFER");

        // Update URL and method for this request
        esp_http_client_set_url(dexcom_client, url);
        esp_http_client_set_method(dexcom_client, HTTP_METHOD_GET);
        // esp_http_client_set_post_field(dexcom_client, "", 0);

        bool success = false;
        if (esp_http_client_perform(dexcom_client) == ESP_OK)
        {
            int status_code = esp_http_client_get_status_code(dexcom_client);
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Dexcom glucose fetch code %d response: %s", status_code, dexcom_response_buffer);
            if (status_code == 200)
            {
                cJSON *root = cJSON_Parse(dexcom_response_buffer);
                if (root && cJSON_IsArray(root) && cJSON_GetArraySize(root) >= 1)
                {
                    cJSON *latest = cJSON_GetArrayItem(root, 0);
                    cJSON *previous = cJSON_GetArraySize(root) > 1 ? cJSON_GetArrayItem(root, 1) : NULL;

                    // Get latest reading
                    cJSON *value = cJSON_GetObjectItem(latest, "Value");
                    cJSON *timestamp = cJSON_GetObjectItem(latest, "WT");
                    cJSON *trend = cJSON_GetObjectItem(latest, "Trend");

                    if (cJSON_IsNumber(value) && cJSON_IsString(timestamp))
                    {
                        glucose_data.previous_gl_mgdl = glucose_data.current_gl_mgdl;
                        glucose_data.current_gl_mgdl = value->valuedouble;
                        glucose_data.timestamp = parse_dexcom_timestamp(timestamp->valuestring);

                        // Get previous reading if available
                        if (previous)
                        {
                            value = cJSON_GetObjectItem(previous, "Value");
                            if (cJSON_IsNumber(value))
                            {
                                glucose_data.previous_gl_mgdl = value->valuedouble;
                            }
                        }
                        else
                        {
                            glucose_data.previous_gl_mgdl = glucose_data.current_gl_mgdl;
                        }

                        // Calculate difference
                        glucose_data.gl_diff = glucose_data.current_gl_mgdl - glucose_data.previous_gl_mgdl;
                        
                        // Map Dexcom Trend string to our 5-value system
                        // 0=down fast, 1=45 down, 2=stable, 3=45 up, 4=up fast, -1=no arrow
                        if (trend && cJSON_IsString(trend))
                        {
                            const char *trend_str = trend->valuestring;
                            if (strcmp(trend_str, "DoubleDown") == 0 || strcmp(trend_str, "SingleDown") == 0)
                                glucose_data.trend_arrow = 0; // down fast
                            else if (strcmp(trend_str, "FortyFiveDown") == 0)
                                glucose_data.trend_arrow = 1; // 45 down
                            else if (strcmp(trend_str, "Flat") == 0)
                                glucose_data.trend_arrow = 2; // stable
                            else if (strcmp(trend_str, "FortyFiveUp") == 0)
                                glucose_data.trend_arrow = 3; // 45 up
                            else if (strcmp(trend_str, "SingleUp") == 0 || strcmp(trend_str, "DoubleUp") == 0)
                                glucose_data.trend_arrow = 4; // up fast (rising)
                            else
                                glucose_data.trend_arrow = -1; // None, NotComputable, RateOutOfRange
                        }
                        else
                        {
                            glucose_data.trend_arrow = -1; // Invalid or missing trend data
                        }
                        
                        success = true;
                    }
                }
                if (root)
                    cJSON_Delete(root);

                if (success)
                {
                    release_shared_buffer(dexcom_response_buffer);
                    dexcom_response_buffer = NULL;
                    release_ssl_semaphore();
                    return true; // Success, exit loop
                }
            }
            else if (status_code == 500)
            { // could look for specific errors, like SessionIdNotFound, but what the heck, login again && strstr(dexcom_response_buffer, "SessionIdNotFound") != NULL) {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Dexcom session expired. Re-logging in...");
                dexcom_session_id[0] = '\0'; // Invalidate session, loop will retry
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom glucose fetch returned http error: %d", status_code);
                // Don't retry for other errors
                release_shared_buffer(dexcom_response_buffer);
                dexcom_response_buffer = NULL;
                release_ssl_semaphore();
                return false;
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom glucose fetch failed");
        }

        release_shared_buffer(dexcom_response_buffer);
        dexcom_response_buffer = NULL;
        release_ssl_semaphore();
    }

    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom glucose fetch failed after retry.");
    release_ssl_semaphore();
    return false;
}

time_t parse_dexcom_timestamp(const char *timestamp)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Parsing Dexcom timestamp: %s", timestamp);
    // Dexcom timestamp format: "/Date(1234567890123-0700)/" 
    char *start = strstr(timestamp, "(");
    if (!start)
        return 0;

    char *end = strstr(start, "-");
    if (!end)
        end = strstr(start, ")");
    if (!end)
        return 0;

    char time_str[32];
    size_t len = end - start - 1;
    if (len >= sizeof(time_str))
        return 0;

    strncpy(time_str, start + 1, len);
    time_str[len] = '\0';

    return (time_t)(atoll(time_str) / 1000); // Convert milliseconds to seconds
}

// format_dexcom_token removed - now using unified format_glucose_token from f-freestyle.c