#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "mbedtls/ssl.h"
#include "f-membuffer.h"
#include "f-json.h"
#include "f-stocks.h"
#include "f-wifi.h"
#include "f-integrations.h"
#include "frixos.h"

static const char *TAG = "f-stocks";


// Dynamic response buffer
static char *stock_response_buffer = NULL;
static int stock_response_len = 0;

// Persistent HTTP client for stocks
static esp_http_client_handle_t stock_client = NULL;
bool stock_client_initialized = false;

// HTTP event handler for stock requests
static esp_err_t stock_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock HTTP error");
        stock_response_len = 0;
        break;
    case HTTP_EVENT_ON_DATA:
        // Ensure we have a valid buffer and the data will fit
        if (stock_response_buffer == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer is NULL");
            return ESP_FAIL;
        }

        // Check if we have enough space
        if (evt->data_len <= 0 || stock_response_len < 0 ||
            (stock_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer overflow len=%d data=%d max=%d",
                        stock_response_len, evt->data_len, HTTP_BUFFER_SIZE);
            stock_response_len = 0;
            return ESP_FAIL;
        }

        // Safe to copy the data
        memcpy(stock_response_buffer + stock_response_len, evt->data, evt->data_len);
        stock_response_len += evt->data_len;
        stock_response_buffer[stock_response_len] = '\0';
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock HTTP done");
        break;
    default:
        break;
    }
    return ESP_OK;
}

bool init_stock_client(void)
{
    // Initialize the client with base configuration
    esp_http_client_config_t config = {
        .url = "https://finnhub.io", // Base URL
        .method = HTTP_METHOD_GET,
        .timeout_ms = 15000,
        .is_async = false,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .tls_version = ESP_TLS_VER_TLS_1_2,
        .user_agent = "Frixos HTTP Client",
        .event_handler = stock_http_event_handler,
    };

    stock_client = esp_http_client_init(&config);
    if (!stock_client)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize stock client");
        return false;
    }

    // Set common headers
    esp_http_client_set_header(stock_client, "Accept", "application/json");
    esp_http_client_set_header(stock_client, "connection", "close");

    return true;
}

void cleanup_stock_client(void)
{
    if (stock_client)
    {
        esp_http_client_cleanup(stock_client);
        stock_client = NULL;
    }
    stock_client_initialized = false;
}

// Function to fetch a single stock quote
bool fetch_stock_quote(integration_token_t *token)
{
    if (!token || !token->entity || !token->entity[0])
    {
        return false;
    }

    // Check WiFi connection before proceeding
    if (!is_wifi_connected())
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "WiFi down, skip stock");
        return false;
    }

    // Initialize or reinitialize client if needed
    if (!stock_client_initialized || stock_client == NULL)
    {
        if (stock_client != NULL) {
            cleanup_stock_client();
        }
        if (!init_stock_client())
        {
            return false;
        }
        stock_client_initialized = true;
    }

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("fetch_stock_quote"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "SSL lock failed (Stock)");
        return false;
    }

    // Get shared buffer for stock response
    stock_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "STOCK_HTTP");
    if (stock_response_buffer == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock buffer failed");
        release_ssl_semaphore();
        return false;
    }
    stock_response_len = 0;

    char urlstr[URL_BUFFER_SIZE];
    snprintf(urlstr, URL_BUFFER_SIZE, "/api/v1/quote?symbol=%s&token=%s",
             token->entity, eeprom_stock_key);

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock HTTPS %s", urlstr);

    // Update URL for this request
    esp_http_client_set_url(stock_client, urlstr);

    bool success = false;
    esp_err_t err = esp_http_client_perform(stock_client);
    
    // Check for connection errors
    if (err == ESP_ERR_HTTP_CONNECT || err == ESP_ERR_HTTP_CONNECTING || 
        err == ESP_ERR_HTTP_EAGAIN) {
        cleanup_stock_client();
        if (!init_stock_client()) {
            release_shared_buffer(stock_response_buffer);
            stock_response_buffer = NULL;
            release_ssl_semaphore();
            return false;
        }
        // Retry the request once with new connection
        esp_http_client_set_url(stock_client, urlstr);
        err = esp_http_client_perform(stock_client);
    }

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(stock_client);
        if (status_code == 200 && stock_response_len > 0)
        {
            // Parse the JSON response
            cJSON *root = cJSON_Parse(stock_response_buffer);
            if (root)
            {
                cJSON *current_price = cJSON_GetObjectItem(root, "c");
                cJSON *change = cJSON_GetObjectItem(root, "dp");
                
                if (cJSON_IsNumber(current_price) && cJSON_IsNumber(change))
                {
                    char value_str[64];
                    snprintf(value_str, sizeof(value_str), "%.2f (%+.1f%%)", 
                            current_price->valuedouble, change->valuedouble);
                    strncpy(token->value, value_str, sizeof(token->value) - 1);
                    token->value[sizeof(token->value) - 1] = '\0';
                    success = true;
                }
                cJSON_Delete(root);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock fetch %d", status_code);
            // If we get a 401 or 403, the connection might be stale
            if (status_code == 401 || status_code == 403) {
                cleanup_stock_client();
                stock_client_initialized = false;
            }
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock HTTP %s", esp_err_to_name(err));
        // Clean up client on any other error
        cleanup_stock_client();
        stock_client_initialized = false;
    }

    release_shared_buffer(stock_response_buffer);
    stock_response_buffer = NULL;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    
    // Clean up HTTP client to free SSL context memory (~3-5KB)
    // Client will be reinitialized on next call if needed
    cleanup_stock_client();
    
    return success;
}

void parse_stock_entities(const char *input)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Parse stocks");

    // Free existing tokens if any
    if (integration_active_tokens[INTEGRATION_STOCK] != NULL)
    {
        free_integration_tokens_array(integration_active_tokens[INTEGRATION_STOCK], integration_active_tokens_count[INTEGRATION_STOCK]);
        integration_active_tokens[INTEGRATION_STOCK] = NULL;
        integration_active_tokens_count[INTEGRATION_STOCK] = 0;
    }

    // First pass: count tokens
    const char *p = input;
    while ((p = strstr(p, "[$:")) != NULL)
    {
        integration_active_tokens_count[INTEGRATION_STOCK]++;
        p += 3; // Skip "[$:"
    }

    if (integration_active_tokens_count[INTEGRATION_STOCK] == 0)
    {
        return; // No tokens found
    }

    // Log memory status before allocation
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t required_memory = integration_active_tokens_count[INTEGRATION_STOCK] * sizeof(integration_token_t);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stock tokens %d size %d mem %d heap %d",
                integration_active_tokens_count[INTEGRATION_STOCK],
                sizeof(integration_token_t),
                required_memory,
                free_heap);

    // Allocate memory for tokens with error checking
    integration_active_tokens[INTEGRATION_STOCK] = (integration_token_t *)malloc(required_memory);
    if (integration_active_tokens[INTEGRATION_STOCK] == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock tokens alloc failed");
        integration_active_tokens_count[INTEGRATION_STOCK] = 0;
        return;
    }
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stock mem ok");
    // Initialize the allocated memory to zero
    memset(integration_active_tokens[INTEGRATION_STOCK], 0, required_memory);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stock mem cleared");

    // Second pass: parse tokens
    int token_index = 0;
    p = input;
    while ((p = strstr(p, "[$:")) != NULL)
    {
        const char *token_start = p; // Save start of token

        // Find the end of the token
        const char *end = strchr(p, ']');
        if (end == NULL)
        {
            break; // Malformed token
        }

        // Calculate symbol length
        size_t symbol_len = end - p + 1;
        size_t name_len = symbol_len;
        size_t entity_len = symbol_len - 4; // Skip "[$:" prefix and ']'

        // Allocate strings
        char *name_str = (char *)malloc(name_len + 1);
        char *entity_str = (char *)malloc(entity_len + 1);
        
        if (name_str == NULL || entity_str == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock token strings alloc failed");
            if (name_str != NULL) free(name_str);
            if (entity_str != NULL) free(entity_str);
            break;
        }

        // Copy token name (full token including brackets)
        strncpy(name_str, p, name_len);
        name_str[name_len] = '\0';

        // Copy entity (just the symbol, skipping the [$: prefix)
        strncpy(entity_str, p + 3, entity_len);
        entity_str[entity_len] = '\0';

        // Initialize token in array using helper function (path is NULL for stocks)
        if (!init_integration_token(&integration_active_tokens[INTEGRATION_STOCK][token_index], name_str, entity_str, NULL))
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Stock token init failed");
            free(name_str);
            free(entity_str);
            break;
        }

        // Free temporary strings (they've been copied into the token)
        free(name_str);
        free(entity_str);

        token_index++;
        p = end + 1; // Move past the token
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Tokens ok");
    // Update actual number of tokens parsed
    integration_active_tokens_count[INTEGRATION_STOCK] = token_index;

    // Log the parsed tokens
    for (int i = 0; i < integration_active_tokens_count[INTEGRATION_STOCK]; i++)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock token %d %s %s",
                    i, integration_active_tokens[INTEGRATION_STOCK][i].name ? integration_active_tokens[INTEGRATION_STOCK][i].name : "(null)",
                    integration_active_tokens[INTEGRATION_STOCK][i].entity ? integration_active_tokens[INTEGRATION_STOCK][i].entity : "(null)");
    }
} 