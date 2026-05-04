#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "esp_log.h"
#include "f-integrations.h"
#include "frixos.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_tls.h"     // Added for TLS types
#include "mbedtls/ssl.h" // Added for mbedTLS configuration
#include "f-wifi.h"
#include "f-membuffer.h" // Added for shared buffer management
#include "f-json.h"
#include "f-display.h"
#include "f-dexcom.h"
#include "f-stocks.h"
#include "f-freestyle.h"
#include "f-nightscout.h"

static const char *TAG = "f-integrations";

// Token management functions
bool init_integration_token(integration_token_t *token, const char *name, const char *entity, const char *path)
{
    if (token == NULL || name == NULL || entity == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "init_token: token/name/entity required");
        return false;
    }

    // Initialize to zero
    memset(token, 0, sizeof(integration_token_t));

    // Allocate and copy name
    size_t name_len = strlen(name) + 1;
    token->name = (char *)malloc(name_len);
    if (token->name == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token name alloc failed");
        return false;
    }
    strcpy(token->name, name);

    // Allocate and copy entity
    size_t entity_len = strlen(entity) + 1;
    token->entity = (char *)malloc(entity_len);
    if (token->entity == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token entity alloc failed");
        free(token->name);
        token->name = NULL;
        return false;
    }
    strcpy(token->entity, entity);

    // Allocate and copy path (if provided, otherwise use "state" as default)
    const char *path_str = (path != NULL && path[0] != '\0') ? path : "state";
    size_t path_len = strlen(path_str) + 1;
    token->path = (char *)malloc(path_len);
    if (token->path == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token path alloc failed");
        free(token->entity);
        free(token->name);
        token->entity = NULL;
        token->name = NULL;
        return false;
    }
    strcpy(token->path, path_str);

    // Initialize value to "-"
    strcpy(token->value, "-");

    return true;
}

void free_integration_token(integration_token_t *token)
{
    if (token == NULL)
    {
        return;
    }

    if (token->name != NULL)
    {
        free(token->name);
        token->name = NULL;
    }

    if (token->entity != NULL)
    {
        free(token->entity);
        token->entity = NULL;
    }

    if (token->path != NULL)
    {
        free(token->path);
        token->path = NULL;
    }

    free(token);
}

void free_integration_tokens_array(integration_token_t *tokens, int count)
{
    if (tokens == NULL)
    {
        return;
    }

    // Free dynamic fields for each token
    for (int i = 0; i < count; i++)
    {
        if (tokens[i].name != NULL)
        {
            free(tokens[i].name);
            tokens[i].name = NULL;
        }
        if (tokens[i].entity != NULL)
        {
            free(tokens[i].entity);
            tokens[i].entity = NULL;
        }
        if (tokens[i].path != NULL)
        {
            free(tokens[i].path);
            tokens[i].path = NULL;
        }
    }

    // Free the array itself
    free(tokens);
}

// Global variables for integrations
integration_token_t *integration_active_tokens[AVAILABLE_INTEGRATIONS] = {NULL}; // Dynamic array of tokens for each integration
int integration_active_tokens_count[AVAILABLE_INTEGRATIONS] = {0};               // Number of active tokens for each integration
int integration_active[AVAILABLE_INTEGRATIONS] = {0};                            // Whether each integration is active
time_t integration_last_update[AVAILABLE_INTEGRATIONS] = {0};                    // Last update time for each integration
bool integration_tokens_updated = false;

// Timer for integration updates
#define INTEGRATION_UPDATE_INTERVAL_MS (30 * 1000) // 30 seconds
esp_timer_handle_t integration_timer = NULL;

// Dynamic response buffer
static char *ha_response_buffer = NULL;
static int ha_response_len = 0;

// Task handle for integration updates
static TaskHandle_t integration_update_task_handle = NULL;
const int32_t int_update_secs = 60; // update timer should tick every 60 seconds

/** After ESP_ERR_HTTP_CONNECT / CONNECTING, skip HA until this wall time (see fetch_ha_entity). */
static time_t ha_http_connect_backoff_until = 0;
/** Set when a HA entity hit connect error; skip remaining entities this cycle. */
static bool ha_abort_remaining_tokens = false;

// Semaphore for SSL connections - ESP32 can only handle one SSL connection at a time
static SemaphoreHandle_t ssl_connection_semaphore = NULL;
#define SSL_SEMAPHORE_TIMEOUT_MS (30 * 1000)    // 30 seconds timeout
static const char *ssl_semaphore_holder = NULL; // Track which function currently holds the semaphore

// Helper function to acquire SSL connection semaphore
bool acquire_ssl_semaphore(const char *function_name)
{
    if (ssl_connection_semaphore == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SSL semaphore not initialized");
        return false;
    }

    TickType_t timeout_ticks = pdMS_TO_TICKS(SSL_SEMAPHORE_TIMEOUT_MS);
    if (xSemaphoreTake(ssl_connection_semaphore, timeout_ticks) == pdTRUE)
    {
        ssl_semaphore_holder = function_name;
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "SSL lock %s", function_name ? function_name : "?");
        return true;
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "SSL lock timeout %ds (held by %s)",
                    SSL_SEMAPHORE_TIMEOUT_MS / 1000, ssl_semaphore_holder ? ssl_semaphore_holder : "unknown");
        return false;
    }
}

// Helper function to release SSL connection semaphore
void release_ssl_semaphore(void)
{
    if (ssl_connection_semaphore != NULL)
    {
        const char *holder = ssl_semaphore_holder;
        ssl_semaphore_holder = NULL;
        xSemaphoreGive(ssl_connection_semaphore);
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "SSL semaphore released by %s", holder ? holder : "unknown");
    }
}

// Custom certificate validation callback to set optional validation
esp_err_t custom_crt_bundle_attach(void *conf)
{
    mbedtls_ssl_config *ssl_conf = (mbedtls_ssl_config *)conf;
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE); // No certificate verification

    // Attach certificate bundle even though we're not verifying
    // This is needed for SSL context setup - the bundle provides root CAs
    // but we set VERIFY_NONE so they won't actually be checked
    esp_err_t ret = esp_crt_bundle_attach(conf);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Cert bundle attach failed: %s", esp_err_to_name(ret));
        // Continue anyway since we're not verifying certificates
        return ESP_OK;
    }
    return ESP_OK;
}

// Function to check stack usage
static void check_stack_usage(void)
{
    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    UBaseType_t free_stack = watermark;
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    // Display the stack usage for the current task and task name
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Task %s FREE: Stack %d, heap %d",
                pcTaskGetName(NULL),
                free_stack,
                free_heap);

    if (free_stack < STACK_WARNING_THRESHOLD)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low stack: %d bytes", free_stack);
    }
}

// HTTP event handler for HA requests
static esp_err_t ha_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP error");
        ha_response_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        // Ensure we have a valid buffer and the data will fit
        if (ha_response_buffer == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer null");
            return ESP_FAIL;
        }

        // Check if we have enough space
        if (evt->data_len <= 0 || ha_response_len < 0 ||
            (ha_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer overflow prevented: len=%d, data_len=%d, max=%d",
                        ha_response_len, evt->data_len, HTTP_BUFFER_SIZE);
            ha_response_len = 0;
            return ESP_FAIL;
        }

        // Safe to copy the data
        memcpy(ha_response_buffer + ha_response_len, evt->data, evt->data_len);
        ha_response_len += evt->data_len;
        ha_response_buffer[ha_response_len] = '\0';
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA HTTP done");
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Function to fetch a single HA entity value
static bool fetch_ha_entity(integration_token_t *token)
{
    if (!token || !token->entity || !token->entity[0])
    {
        return false;
    }

    // Check WiFi connection before proceeding
    if (!is_wifi_connected())
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "WiFi not connected, skipping HA entity fetch");
        return false;
    }

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("fetch_ha_entity"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SSL lock failed (HA fetch)");
        return false;
    }

    // Get shared buffer for HA response
    ha_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "HA_HTTP");
    if (ha_response_buffer == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA response buffer failed");
        release_ssl_semaphore();
        return false;
    }

    char urlstr[URL_BUFFER_SIZE];
    snprintf(urlstr, URL_BUFFER_SIZE, "%s/api/states/%s", eeprom_ha_url, token->entity);

    esp_http_client_config_t config = {
        .url = urlstr,
        .event_handler = ha_http_event_handler,
        // Keep the per-attempt budget short so a stalled HA can't tie up the radio
        // and CPU for ages. We retry on the next 60s integration cycle anyway.
        .timeout_ms = 5000,
        .buffer_size = HTTP_BUFFER_SIZE,    // Use HTTP_BUFFER_SIZE
        .buffer_size_tx = HTTP_BUFFER_SIZE, // Use HTTP_BUFFER_SIZE
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .tls_version = ESP_TLS_VER_TLS_1_2,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
        vTaskDelay(pdMS_TO_TICKS(100)); // Add a small delay before retry
        client = esp_http_client_init(&config);
        if (client == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP client init failed (after cleanup)");
            release_shared_buffer(ha_response_buffer);
            ha_response_buffer = NULL;
            release_ssl_semaphore();
            return false;
        }
    }

    // Format Authorization header with Bearer token
    char auth_header[AUTH_BUFFER_SIZE];
    snprintf(auth_header, AUTH_BUFFER_SIZE, "Bearer %.240s", eeprom_ha_token);
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "Connection", "close");

    // Single attempt per token per cycle. The integration task wakes again
    // every int_update_secs (60s); piling on multiple SSL handshakes against
    // a sick HA inside one cycle hurts the radio/CPU more than it helps.
    int max_retries = 1;
    int retry_count = 0;
    bool success = false;
    int backoff_ms = 500;

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Fetching HA entity %s for path %s", token->entity, token->path);

    while (retry_count < max_retries && !success)
    {
        ha_response_len = 0; // Reset response buffer
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA HTTP %s (%d/%d)", urlstr, retry_count + 1, max_retries);

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK)
        {
            int status_code = esp_http_client_get_status_code(client);
            if (status_code == 200)
            {
                // Parse JSON response
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA response: %s", ha_response_buffer);

                char value_buffer[64];
                char *value = get_value_from_JSON_string(ha_response_buffer, token->path, value_buffer, sizeof(value_buffer), NULL);

                if (strcmp(value, "-") != 0)
                {
                    // Remove quotes from beginning/end if present
                    char *start = value;
                    size_t len = strlen(value);
                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA value [%s] len %d", value, len);
                    if (len > 0 && (value[0] == '"' || value[0] == '\''))
                    {
                        start++;
                        len--;
                    }
                    if (len > 0 && (start[len - 1] == '"' || start[len - 1] == '\''))
                    {
                        start[len - 1] = '\0'; // len-1 is already \0, len-2 should be "
                        len--;
                    }
                    // Ensure we have a valid length after quote removal
                    if (len > 0)
                    {
                        size_t copy_len = min(len, sizeof(token->value) - 1);
                        strncpy(token->value, start, copy_len);
                        token->value[copy_len] = '\0';
                    }
                    else
                    {
                        strcpy(token->value, "-");
                    }
                    success = true;
                    ha_http_connect_backoff_until = 0;
                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA %s %s=%s",
                                token->entity, token->path, token->value);
                }
                else
                {
                    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA invalid value path %s", token->path);
                }
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA status %d (%d/%d)",
                            status_code, retry_count + 1, max_retries);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA request failed: %s (attempt %d/%d)",
                        esp_err_to_name(err), retry_count + 1, max_retries);

            // Any transport-level failure: don't keep hammering. Most likely HA is
            // down or the link is bad — and retrying repeatedly inside one cycle
            // is exactly what made the device unreachable to ping. Skip the rest
            // of this cycle and back off for at least 30s.
            time_t now = time(NULL);
            ha_http_connect_backoff_until = now + 30;
            ha_abort_remaining_tokens = true;
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "HA transport error (%s); backing off 30s",
                        esp_err_to_name(err));
            break;
        }

        if (!success && retry_count < max_retries - 1)
        {
            // Reset transport state between retries so next perform() starts fresh.
            esp_http_client_close(client);
            retry_count++;
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Retry in %d ms", backoff_ms);
            vTaskDelay(pdMS_TO_TICKS(backoff_ms));
            // backoff_ms *= 2; // Exponential backoff cancelled; don't like it
        }
        else
        {
            break;
        }
    }

    esp_http_client_cleanup(client);

    // Force garbage collection after each request
    if (heap_caps_get_free_size(MALLOC_CAP_8BIT) < 10000)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low mem before cleanup: %d", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        vTaskDelay(pdMS_TO_TICKS(100)); // Give time for cleanup
    }

    // Release the shared buffer before returning
    release_shared_buffer(ha_response_buffer);
    ha_response_buffer = NULL;

    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    return success;
}

// Task function for integration updates
static void integration_update_task(void *pvParameters)
{
    bool update_ok[AVAILABLE_INTEGRATIONS] = {false, false};

    while (1)
    {
        if (!ota_update_in_progress)
        {
            check_stack_usage(); // Monitor stack usage

            // Check heap memory and log warning if low
            size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Integration cycle heap %d", free_heap);
            if (free_heap < 5000) // Less than 5KB free
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low heap: %d", free_heap);
            }

            // Monitor buffer pool usage
            size_t total_buffers, used_buffers, total_memory;
            get_buffer_pool_stats(&total_buffers, &used_buffers, &total_memory);
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Buffer pool: %d/%d buffers used (%d KB)", used_buffers, total_buffers, total_memory / 1024);
            if (used_buffers >= total_buffers)
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Buffer pool exhausted %d", total_buffers);
            }

            update_ok[INTEGRATION_HA] = false;
            update_ok[INTEGRATION_STOCK] = false;
            update_ok[INTEGRATION_DEXCOM] = false;
            update_ok[INTEGRATION_FREESTYLE] = false;
            update_ok[INTEGRATION_NIGHTSCOUT] = false;

            // Check WiFi connection before attempting updates
            if (is_wifi_connected())
            {

                // Check Home Assistant integration
                if (integration_active[INTEGRATION_HA] && integration_active_tokens_count[INTEGRATION_HA] > 0)
                {
                    // has it been long enough?
                    if (integration_last_update[INTEGRATION_HA] + (eeprom_ha_refresh_mins * 60) < time(NULL))
                    {
                        time_t now_wall = time(NULL);
                        if (now_wall < ha_http_connect_backoff_until)
                        {
                            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA in post-connect-error backoff (%lds left)",
                                        (long)(ha_http_connect_backoff_until - now_wall));
                        }
                        // Check memory before attempting SSL connection (HA needs ~15KB for SSL)
                        else
                        {
                            size_t ha_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
                            if (ha_heap < 15000)
                            {
                                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Skip HA: low mem %d (need ~15K)", ha_heap);
                            }
                            else
                            {
                                // Take http_mutex per token (not for the whole loop) so the web
                                // server can serve requests between HA fetches — otherwise a slow
                                // or timing-out HA endpoint blocks the UI for tens of seconds.
                                ha_abort_remaining_tokens = false;
                                bool ha_mutex_failed = false;
                                for (int i = 0; i < integration_active_tokens_count[INTEGRATION_HA]; i++)
                                {
                                    if (ha_abort_remaining_tokens)
                                        break;

                                    if (xSemaphoreTake(http_mutex, pdMS_TO_TICKS(5000)) != pdTRUE)
                                    {
                                        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "http_mutex timeout (HA token %d)", i);
                                        ha_mutex_failed = true;
                                        break;
                                    }

                                    bool ok = fetch_ha_entity(&integration_active_tokens[INTEGRATION_HA][i]);
                                    xSemaphoreGive(http_mutex);

                                    if (ok)
                                    {
                                        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA token %d %s=%s",
                                                    i, integration_active_tokens[INTEGRATION_HA][i].name ? integration_active_tokens[INTEGRATION_HA][i].name : "(null)",
                                                    integration_active_tokens[INTEGRATION_HA][i].value);
                                        update_ok[INTEGRATION_HA] = true;
                                    }
                                    else
                                    {
                                        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA token %d failed %s",
                                                    i, integration_active_tokens[INTEGRATION_HA][i].entity ? integration_active_tokens[INTEGRATION_HA][i].entity : "(null)");
                                    }

                                    // Yield between tokens so httpd / lwIP / WiFi tasks on PRO_CPU
                                    // get plenty of CPU and radio time before the next handshake.
                                    vTaskDelay(pdMS_TO_TICKS(250));
                                }
                                if (!ha_mutex_failed && !update_ok[INTEGRATION_HA])
                                    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "HA update failed");
                                // Small delay to allow memory cleanup before next integration
                                vTaskDelay(pdMS_TO_TICKS(100));
                            }
                        }
                    }
                }

                // Check Stock integration
                if (integration_active[INTEGRATION_STOCK] && integration_active_tokens_count[INTEGRATION_STOCK] > 0)
                {
                    // has it been long enough?
                    if (integration_last_update[INTEGRATION_STOCK] + (eeprom_stock_refresh_mins * 60) < time(NULL))
                    {
                        // Check memory before attempting SSL connection (Stocks needs ~15KB for SSL)
                        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
                        if (free_heap < 15000)
                        {
                            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Skipping Stock update - insufficient memory: %d bytes free (need ~15KB)", free_heap);
                        }
                        else
                        {
                            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock update %d",
                                        integration_active_tokens_count[INTEGRATION_STOCK]);
                            // Take http_mutex per token (not for the whole loop) so the web UI
                            // stays responsive while we poll quotes one by one.
                            bool stock_mutex_failed = false;
                            for (int i = 0; i < integration_active_tokens_count[INTEGRATION_STOCK]; i++)
                            {
                                if (xSemaphoreTake(http_mutex, pdMS_TO_TICKS(5000)) != pdTRUE)
                                {
                                    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "http_mutex timeout (Stock token %d)", i);
                                    stock_mutex_failed = true;
                                    break;
                                }

                                bool ok = fetch_stock_quote(&integration_active_tokens[INTEGRATION_STOCK][i]);
                                xSemaphoreGive(http_mutex);

                                if (ok)
                                {
                                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock token %d %s=%s",
                                                i, integration_active_tokens[INTEGRATION_STOCK][i].entity ? integration_active_tokens[INTEGRATION_STOCK][i].entity : "(null)",
                                                integration_active_tokens[INTEGRATION_STOCK][i].value);
                                    update_ok[INTEGRATION_STOCK] = true;
                                }
                                else
                                {
                                    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock token %d failed %s",
                                                i, integration_active_tokens[INTEGRATION_STOCK][i].entity);
                                }

                                // Yield between tokens so httpd can serve UI requests.
                                vTaskDelay(pdMS_TO_TICKS(250));
                            }
                            if (!stock_mutex_failed && !update_ok[INTEGRATION_STOCK])
                                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stock update failed");
                            // Small delay to allow memory cleanup before next integration
                            vTaskDelay(pdMS_TO_TICKS(100));
                        }
                    }
                }

                // Dexcom integration, always update if active
                if (integration_active[INTEGRATION_DEXCOM])
                { // If not disabled
                  // has it been long enough?
                    if (time(NULL) - integration_last_update[INTEGRATION_DEXCOM] >= (eeprom_glucose_refresh * 60))
                    {
                        if (!dexcom_client_initialized)
                        {
                            if (!init_dexcom_client())
                                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom client init failed");
                            else if (!authenticate_dexcom_account())
                                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Dexcom auth failed");
                        }

                        if (fetch_dexcom_glucose())
                            update_ok[INTEGRATION_DEXCOM] = true;
                    }
                }

                // Freestyle Libre integration, always update if active 
                if (integration_active[INTEGRATION_FREESTYLE])
                {
                    if (time(NULL) - integration_last_update[INTEGRATION_FREESTYLE] >= (eeprom_glucose_refresh * 60))
                    {
                        if (!freestyle_client_initialized)
                            init_freestyle_client();

                        if (fetch_freestyle_glucose())
                            update_ok[INTEGRATION_FREESTYLE] = true;
                    }
                }

                // Nightscout integration
                if (integration_active[INTEGRATION_NIGHTSCOUT])
                {
                    if (time(NULL) - integration_last_update[INTEGRATION_NIGHTSCOUT] >= (eeprom_glucose_refresh * 60))
                    {
                        if (!nightscout_client_initialized)
                            init_nightscout_client();

                        if (fetch_nightscout_glucose())
                            update_ok[INTEGRATION_NIGHTSCOUT] = true;
                    }
                }

                // check if any updates were successful
                bool update_ok_any = false;
                for (int i = 0; i < AVAILABLE_INTEGRATIONS; i++)
                {
                    if (update_ok[i])
                        time(&integration_last_update[i]); // store UTC time
                    update_ok_any |= update_ok[i];
                }
                // set the updated flag if any updates were successful
                integration_tokens_updated = integration_tokens_updated || update_ok_any;
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "WiFi not connected, skipping integration update");
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "OTA in progress, skip integration");
        }
        // Wait for the next timer trigger
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

// Timer callback function - now just notifies the task
static void integration_timer_callback(void *arg)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Integration timer");

    if (integration_update_task_handle != NULL)
    {
        xTaskNotifyGive(integration_update_task_handle);
    }
}

void parse_HA_entities(const char *input)
{
    // Free existing tokens if any
    if (integration_active_tokens[INTEGRATION_HA] != NULL)
    {
        free_integration_tokens_array(integration_active_tokens[INTEGRATION_HA], integration_active_tokens_count[INTEGRATION_HA]);
        integration_active_tokens[INTEGRATION_HA] = NULL;
        integration_active_tokens_count[INTEGRATION_HA] = 0;
    }

    // First pass: count tokens
    const char *p = input;
    while ((p = strstr(p, "[HA:")) != NULL)
    {
        integration_active_tokens_count[INTEGRATION_HA]++;
        p += 4; // Skip "[HA:"
    }

    if (integration_active_tokens_count[INTEGRATION_HA] == 0)
    {
        return; // No tokens found
    }

    // Allocate memory for token pointers
    integration_active_tokens[INTEGRATION_HA] = (integration_token_t *)calloc(integration_active_tokens_count[INTEGRATION_HA], sizeof(integration_token_t));
    if (integration_active_tokens[INTEGRATION_HA] == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HA tokens alloc failed");
        integration_active_tokens_count[INTEGRATION_HA] = 0;
        return;
    }

    // Initialize array to zero
    memset(integration_active_tokens[INTEGRATION_HA], 0, integration_active_tokens_count[INTEGRATION_HA] * sizeof(integration_token_t));

    // Second pass: parse tokens
    int token_index = 0;
    p = input;
    while ((p = strstr(p, "[HA:")) != NULL)
    {
        const char *token_start = p; // Save start of token for name
        p += 4;                      // Skip "[HA:"

        // Find the end of the token
        const char *end = strchr(p, ']');
        if (end == NULL)
        {
            break; // Malformed token
        }

        // Find the path separator, skipping the [HA: prefix
        const char *path_start = strchr(p, ':');

        // Calculate entity and path strings
        char *entity_str = NULL;
        char *path_str = NULL;
        bool path_allocated = false;

        if (path_start == NULL || path_start >= end)
        {
            // No path specified, use "state"
            size_t entity_len = end - p;
            entity_str = (char *)malloc(entity_len + 1);
            if (entity_str == NULL)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Entity alloc failed");
                break;
            }
            strncpy(entity_str, p, entity_len);
            entity_str[entity_len] = '\0';
            path_str = NULL; // Will use "state" default in init_integration_token
            path_allocated = false;
        }
        else
        {
            size_t entity_len = path_start - p;
            entity_str = (char *)malloc(entity_len + 1);
            if (entity_str == NULL)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Entity alloc failed");
                break;
            }
            strncpy(entity_str, p, entity_len);
            entity_str[entity_len] = '\0';

            size_t path_len = end - (path_start + 1);
            path_str = (char *)malloc(path_len + 1);
            if (path_str == NULL)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to allocate memory for path");
                free(entity_str);
                break;
            }
            strncpy(path_str, path_start + 1, path_len);
            path_str[path_len] = '\0';
            path_allocated = true;
        }

        // Store the complete token name
        size_t name_len = end - token_start + 1;
        char *name_str = (char *)malloc(name_len + 1);
        if (name_str == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token name alloc failed");
            free(entity_str);
            if (path_allocated)
            {
                free(path_str);
            }
            break;
        }
        strncpy(name_str, token_start, name_len);
        name_str[name_len] = '\0';

        // Initialize token in array using helper function
        if (!init_integration_token(&integration_active_tokens[INTEGRATION_HA][token_index], name_str, entity_str, path_str))
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Integration token init failed");
            free(name_str);
            free(entity_str);
            if (path_allocated)
            {
                free(path_str);
            }
            break;
        }

        // Free temporary strings (they've been copied into the token)
        free(name_str);
        free(entity_str);
        if (path_allocated)
        {
            free(path_str);
        }

        token_index++;
        p = end + 1; // Move past the token
    }

    // Update actual number of tokens parsed
    integration_active_tokens_count[INTEGRATION_HA] = token_index;

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Found %d HA tokens",
                integration_active_tokens_count[INTEGRATION_HA]);
}

void parse_integrations(void)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Parse integrations");

    // determine active integrations
    integration_active[INTEGRATION_HA] = (eeprom_ha_url[0] != '\0' && eeprom_ha_token[0] != '\0');
    integration_active[INTEGRATION_STOCK] = (eeprom_stock_key[0] != '\0');

    // Enforce mutual exclusivity: the last integration takes precedence if configured
    integration_active[INTEGRATION_DEXCOM] = 0;
    integration_active[INTEGRATION_FREESTYLE] = 0;
    integration_active[INTEGRATION_NIGHTSCOUT] = 0;

    if (eeprom_ns_url[0] != '\0' && eeprom_glucose_password[0] != '\0')
    {
        integration_active[INTEGRATION_NIGHTSCOUT] = 1;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Nightscout active");
    }
    if (eeprom_dexcom_region > 0 && eeprom_glucose_username[0] != '\0' && eeprom_glucose_password[0] != '\0')
    {
        integration_active[INTEGRATION_NIGHTSCOUT] = 0;
        integration_active[INTEGRATION_DEXCOM] = 1;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Dexcom on");
    }
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Libre region %d user %s", eeprom_libre_region, eeprom_glucose_username);
    if (eeprom_libre_region > 0 && eeprom_glucose_username[0] != '\0' && eeprom_glucose_password[0] != '\0')
    {
        integration_active[INTEGRATION_NIGHTSCOUT] = 0;
        integration_active[INTEGRATION_DEXCOM] = 0;
        integration_active[INTEGRATION_FREESTYLE] = 1;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Libre on, Dexcom/NS disabled");
    }

    // Parse HA entities if HA integration is active
    if (integration_active[INTEGRATION_HA])
    {
        parse_HA_entities(eeprom_message);
    }

    if (integration_active[INTEGRATION_STOCK])
    {
        parse_stock_entities(eeprom_message);
    }

    if (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT])
    {
        // fake two tokens for each CGM integration, [CGM:glucose] and [CGM:reading]
        integration_active_tokens_count[INTEGRATION_DEXCOM] = 2;
        integration_active_tokens_count[INTEGRATION_FREESTYLE] = 2;
        integration_active_tokens_count[INTEGRATION_NIGHTSCOUT] = 2;
    }

    // force updating of the integrations
    integration_last_update[INTEGRATION_HA] = 0;
    integration_last_update[INTEGRATION_STOCK] = 0;
    integration_last_update[INTEGRATION_DEXCOM] = 0;
    integration_last_update[INTEGRATION_FREESTYLE] = 0;
    integration_last_update[INTEGRATION_NIGHTSCOUT] = 0;

    // prepare the tokens for display
    prepare_tokens();

    // Mark that tokens have been updated
    integration_tokens_updated = true;
}

void startup_integrations(void)
{
    // Create SSL connection semaphore (binary semaphore - only one SSL connection at a time)
    ssl_connection_semaphore = xSemaphoreCreateBinary();
    if (ssl_connection_semaphore == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SSL semaphore create failed");
        return;
    }
    // Give the semaphore initially so first connection can acquire it
    xSemaphoreGive(ssl_connection_semaphore);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "SSL semaphore ready");

    // parse for any active integrations
    parse_integrations();

    // Create the integration update task with increased stack size
    // Increased to 10KB to handle SSL connections and HTTP buffers when multiple integrations run
    // Pin to APP_CPU (core 1). WiFi/lwIP/httpd run on PRO_CPU (core 0) and
    // heavy mbedtls handshakes here would otherwise starve them — observable
    // as multi-second ping latency or "destination host unreachable" while a
    // failing HA endpoint is being retried.
    BaseType_t task_created = xTaskCreatePinnedToCore(
        integration_update_task,         // Task function
        "integration_update_task",       // Task name
        8192 + 2048,                     // 10KB stack for multiple SSL connections
        NULL,                            // Task parameters
        3,                               // Task priority (reduced from 5 to 3)
        &integration_update_task_handle, // Task handle
        1                                // APP_CPU (NOT 0 / PRO_CPU)
    );

    if (task_created != pdPASS)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Integration task create failed");
        return;
    }

    // Create and start the integration update timer
    esp_timer_create_args_t timer_args = {
        .callback = &integration_timer_callback,
        .name = "integration_timer"};

    esp_err_t err = esp_timer_create(&timer_args, &integration_timer);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Integration timer create: %s", esp_err_to_name(err));
        vTaskDelete(integration_update_task_handle);
        integration_update_task_handle = NULL;
        return;
    }

    err = esp_timer_start_periodic(integration_timer, int_update_secs * 1000 * 1000); // Convert to microseconds
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Integration timer start: %s", esp_err_to_name(err));
        esp_timer_delete(integration_timer);
        integration_timer = NULL;
        vTaskDelete(integration_update_task_handle);
        integration_update_task_handle = NULL;
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Integration update timer started with %d second interval", eeprom_ha_refresh_mins);
}

// Cleanup function to free resources
void force_integration_update(void)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Force integration update");

    ha_http_connect_backoff_until = 0;
    ha_abort_remaining_tokens = false;

    // Reset last update times to force immediate update
    integration_last_update[INTEGRATION_HA] = 0;
    integration_last_update[INTEGRATION_STOCK] = 0;
    integration_last_update[INTEGRATION_DEXCOM] = 0;
    integration_last_update[INTEGRATION_FREESTYLE] = 0;
    integration_last_update[INTEGRATION_NIGHTSCOUT] = 0;

    // Signal the integration task to run immediately
    if (integration_update_task_handle != NULL)
    {
        xTaskNotifyGive(integration_update_task_handle);
    }

    // Mark tokens as updated to trigger display refresh
    integration_tokens_updated = true;

    // Also trigger the timer to ensure the task runs
    if (integration_timer != NULL)
    {
        esp_timer_stop(integration_timer);
        esp_timer_start_periodic(integration_timer, int_update_secs * 1000 * 1000);
    }
}

void cleanup_integrations(void)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Integration cleanup");
    cleanup_dexcom_client();
    cleanup_freestyle_client();
    cleanup_nightscout_client();
    if (integration_timer)
    {
        esp_timer_stop(integration_timer);
        esp_timer_delete(integration_timer);
        integration_timer = NULL;
    }
    if (integration_update_task_handle)
    {
        vTaskDelete(integration_update_task_handle);
        integration_update_task_handle = NULL;
    }
    if (ssl_connection_semaphore)
    {
        vSemaphoreDelete(ssl_connection_semaphore);
        ssl_connection_semaphore = NULL;
    }
}
