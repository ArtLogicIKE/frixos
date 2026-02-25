#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mbedtls/md.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "cJSON.h"
#include "frixos.h"
#include "f-freestyle.h"
#include "f-integrations.h"
#include "f-membuffer.h"

static const char *TAG = "f-freestyle";
#define LIBRE_CLIENT_VERSION "4.20.0"

// Guard to prevent recursive calls to login_freestyle
static bool login_freestyle_in_progress = false;

// Default base URL for LibreLinkUp
static const char *LIBRE_BASE_URL = "https://api.libreview.io";
static const char *LIBRE_US_BASE_URL = "https://api-us.libreview.io";
static const char *LIBRE_EU_BASE_URL = "https://api-eu.libreview.io";
static const char *LIBRE_DE_BASE_URL = "https://api-de.libreview.io";
static const char *LIBRE_FR_BASE_URL = "https://api-fr.libreview.io";
static const char *LIBRE_JP_BASE_URL = "https://api-jp.libreview.io";
static const char *LIBRE_AU_BASE_URL = "https://api-au.libreview.io";

// Use unified glucose_data instead of separate freestyle_data
extern glucose_data_t glucose_data;
bool freestyle_client_initialized = false;

// Buffer for Freestyle responses
static char *freestyle_response_buffer = NULL;
static int freestyle_response_len = 0;

// Persistent HTTP client for Freestyle
static esp_http_client_handle_t freestyle_client = NULL;

// Chunk processing mode for glucose fetch
static bool is_glucose_chunk_mode = false;
static double latest_glucose_value = 0.0;
static int latest_trend_arrow = 0;
static time_t latest_timestamp = 0;
// Small sliding window buffer for handling objects that span chunks (max measurement object ~500 bytes)
#define GLUCOSE_CHUNK_BUFFER_SIZE 1024
static char glucose_chunk_buffer[GLUCOSE_CHUNK_BUFFER_SIZE];
static int glucose_chunk_buffer_len = 0;

static time_t parse_freestyle_timestamp(const char *timestamp_str);

// Function to parse UTC timestamp in the same format as Freestyle timestamp but adjust for UTC
// FactoryTimestamp uses the same "M/D/YYYY H:MM:SS AM/PM" format but is in UTC
static time_t parse_utc_timestamp(const char *timestamp_str)
{
    if (!timestamp_str || timestamp_str[0] == '\0')
        return 0;
    
    // Parse using the same format as regular timestamp
    time_t result = parse_freestyle_timestamp(timestamp_str);
    if (result == 0)
        return 0;
    
    // Get timezone offset by comparing current time in local vs UTC
    time_t now = time(NULL);
    struct tm local_tm, utc_tm;
    localtime_r(&now, &local_tm);
    gmtime_r(&now, &utc_tm);
    
    // Calculate timezone offset: difference between local and UTC interpretation
    // Create a copy of UTC tm and convert it as if it were local time
    struct tm utc_as_local = utc_tm;
    utc_as_local.tm_isdst = -1; // Let system determine DST
    time_t utc_interpreted_as_local = mktime(&utc_as_local);
    
    // The offset is the difference (positive when local time is ahead of UTC)
    // For example, if we're in UTC+2, now (local) will be 2 hours ahead of utc_interpreted_as_local
    long tz_offset = (long)(now - utc_interpreted_as_local);
    
    // Adjust the parsed UTC timestamp by adding the timezone offset
    // parse_freestyle_timestamp() uses mktime() which interprets the timestamp as local time
    // But the timestamp string is actually in UTC, so we need to convert from UTC to local
    // Example: timestamp says "10:01" (UTC), we're in UTC+2, so local time is 12:01
    // mktime() interpreted "10:01" as 10:01 local (which is 08:01 UTC)
    // We need to add 2 hours to get 12:01 local (which is 10:01 UTC)
    result += tz_offset;
    
    return result;
}

// Function to parse Freestyle timestamp format: "M/D/YYYY H:MM:SS AM/PM" (legacy, kept for backward compatibility)
static time_t parse_freestyle_timestamp(const char *timestamp_str)
{
    if (!timestamp_str || timestamp_str[0] == '\0')
        return 0;
    
    struct tm tm = {0};
    char ampm[3] = {0};
    int hour, minute, second, month, day, year;
    
    // Parse format: "1/11/2026 11:32:24 AM"
    // Try to parse with sscanf
    if (sscanf(timestamp_str, "%d/%d/%d %d:%d:%d %2s", 
               &month, &day, &year, &hour, &minute, &second, ampm) == 7)
    {
        // Convert 12-hour to 24-hour format
        if (ampm[0] == 'P' || ampm[0] == 'p')
        {
            if (hour != 12)
                hour += 12;
        }
        else if (ampm[0] == 'A' || ampm[0] == 'a')
        {
            if (hour == 12)
                hour = 0;
        }
        
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1; // Let system determine DST
        
        time_t result = mktime(&tm);
        if (result == (time_t)-1)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to parse timestamp: %s", timestamp_str);
            return 0;
        }
        return result;
    }
    
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timestamp format: %s", timestamp_str);
    return 0;
}

void generateSHA256(const unsigned char* text, char* hash)
{  	
	mbedtls_md_context_t ctx;
	mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    char binhash[32];

	mbedtls_md_init(&ctx);
	mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
	mbedtls_md_starts(&ctx);
	mbedtls_md_update(&ctx, (const unsigned char*)text, (size_t)strlen((const char*)text));
	mbedtls_md_finish(&ctx, (unsigned char*)binhash);
	mbedtls_md_free(&ctx);
    
    // now convert the hash to a hexadecimal string
    for (int i = 0; i < 32; i++)
    {
        sprintf(hash + i*2, "%02x", binhash[i]);
    }
    hash[64] = '\0';
   // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "SHA256 source: %.32s, hash: %.32s", text, hash);    

}

// Helper to handle redirect (placeholder if needed, but we rely on correct base URL)

// Helper function to find the matching closing brace for an opening brace
// Returns pointer to closing brace or NULL if not found
static char *find_matching_brace(char *start, char *end)
{
    if (!start || !end || start >= end || *start != '{')
        return NULL;
    
    int depth = 0;
    char *pos = start;
    while (pos < end)
    {
        if (*pos == '{')
            depth++;
        else if (*pos == '}')
        {
            depth--;
            if (depth == 0)
                return pos;
        }
        pos++;
    }
    return NULL;
}

// Helper function to extract a numeric value from a JSON field
static double extract_numeric_value(char *field_start, char *obj_end)
{
    if (!field_start || !obj_end || field_start >= obj_end)
        return 0.0;
    
    char *colon = strchr(field_start, ':');
    if (!colon || colon >= obj_end)
        return 0.0;
    
    char *value_start = colon + 1;
    // Skip whitespace
    while (value_start < obj_end && (*value_start == ' ' || *value_start == '\t'))
        value_start++;
    
    if (value_start >= obj_end)
        return 0.0;
    
    return strtod(value_start, NULL);
}

// Helper function to extract a string value from a JSON field
static bool extract_string_value(char *field_start, char *obj_end, char *buffer, int buffer_size)
{
    if (!field_start || !obj_end || field_start >= obj_end || !buffer || buffer_size <= 0)
        return false;
    
    char *colon = strchr(field_start, ':');
    if (!colon || colon >= obj_end)
        return false;
    
    char *value_start = colon + 1;
    // Skip whitespace
    while (value_start < obj_end && (*value_start == ' ' || *value_start == '\t'))
        value_start++;
    
    if (value_start >= obj_end)
        return false;
    
    // Skip opening quote if present
    if (*value_start == '"')
        value_start++;
    
    // Find end of string value (quote, comma, or brace)
    char *value_end = value_start;
    while (value_end < obj_end && *value_end != '"' && *value_end != ',' && *value_end != '}')
        value_end++;
    
    int len = value_end - value_start;
    if (len > 0 && len < buffer_size)
    {
        strncpy(buffer, value_start, len);
        buffer[len] = '\0';
        return true;
    }
    return false;
}

// Function to process glucose chunks and extract latest measurement from glucoseMeasurement objects
static void process_glucose_chunk(const char *chunk, int len)
{
    if (len <= 0)
        return;
    
    // If buffer would overflow, shift left to keep only the tail (last 512 bytes)
    // This ensures we can handle objects that span chunks while not using too much memory
    int needed_space = len + 1; // +1 for null terminator
    if (glucose_chunk_buffer_len + needed_space > GLUCOSE_CHUNK_BUFFER_SIZE)
    {
        int target_size = 512;
        int shift_amount = (glucose_chunk_buffer_len + len) - target_size;
        if (shift_amount > 0 && shift_amount < glucose_chunk_buffer_len)
        {
            memmove(glucose_chunk_buffer, glucose_chunk_buffer + shift_amount, glucose_chunk_buffer_len - shift_amount);
            glucose_chunk_buffer_len -= shift_amount;
        }
        else if (shift_amount >= glucose_chunk_buffer_len)
        {
            // Not enough space even after shift, keep only last part of new chunk
            glucose_chunk_buffer_len = 0;
        }
    }
    
    // Append chunk to sliding window buffer
    int available = GLUCOSE_CHUNK_BUFFER_SIZE - glucose_chunk_buffer_len - 1;
    int copy_len = (len < available) ? len : available;
    
    if (copy_len > 0)
    {
        memcpy(glucose_chunk_buffer + glucose_chunk_buffer_len, chunk, copy_len);
        glucose_chunk_buffer_len += copy_len;
        glucose_chunk_buffer[glucose_chunk_buffer_len] = '\0';
    }
    
    // Search for "glucoseMeasurement" objects in the current buffer
    // Process all occurrences, keeping the latest one (by timestamp)
    // Handle objects that may span across chunks by using the sliding window buffer
    char *search_pos = glucose_chunk_buffer;
    char *buffer_end = glucose_chunk_buffer + glucose_chunk_buffer_len;
    
    while ((search_pos = strstr(search_pos, "\"glucoseMeasurement\"")) != NULL)
    {
        // Find the opening brace of the glucoseMeasurement object
        // In JSON: "glucoseMeasurement": { ... }
        // Look for ':' after "glucoseMeasurement", then '{' after that
        char *colon_pos = search_pos;
        while (colon_pos < buffer_end && *colon_pos != ':')
            colon_pos++;
        
        if (colon_pos >= buffer_end)
        {
            // Colon not found yet (might be in next chunk), skip for now
            search_pos++;
            continue;
        }
        
        // Look for '{' after the colon (skip whitespace)
        char *obj_start = colon_pos + 1;
        while (obj_start < buffer_end && (*obj_start == ' ' || *obj_start == '\t' || *obj_start == '\n' || *obj_start == '\r'))
            obj_start++;
        
        if (obj_start >= buffer_end || *obj_start != '{')
        {
            // Opening brace not found yet (might be in next chunk), skip for now
            search_pos++;
            continue;
        }
        
        // Find the matching closing brace
        char *obj_end = find_matching_brace(obj_start, buffer_end);
        if (!obj_end)
        {
            // Object spans beyond current buffer - it will be processed when more chunks arrive
            // Don't skip it completely, but we can't process it yet
            // The sliding window buffer will preserve it for the next chunk
            search_pos++;
            continue;
        }
        
        // We have a complete glucoseMeasurement object - extract values
        double glucose_value = 0.0;
        int trend_arrow = 0;
        time_t timestamp = 0;
        bool has_valid_value = false;
        
        // Try to extract ValueInMgPerDl
        char *value_field = strstr(obj_start, "\"ValueInMgPerDl\"");
        if (value_field && value_field < obj_end)
        {
            glucose_value = extract_numeric_value(value_field, obj_end);
            if (glucose_value > 0 && glucose_value < 1000) // Reasonable range check
            {
                has_valid_value = true;
            }
        }
        
        // If ValueInMgPerDl not found, try ValueInMmolPerl and convert to mg/dL
        if (!has_valid_value)
        {
            value_field = strstr(obj_start, "\"ValueInMmolPerl\"");
            if (value_field && value_field < obj_end)
            {
                double glucose_mmol = extract_numeric_value(value_field, obj_end);
                if (glucose_mmol > 0 && glucose_mmol < 100) // Reasonable range check for mmol/L
                {
                    // Convert mmol/L to mg/dL: mg/dL = mmol/L * 18.0182
                    glucose_value = glucose_mmol * 18.0182;
                    has_valid_value = true;
                }
            }
        }
        
        // Extract TrendArrow from the same glucoseMeasurement object
        if (has_valid_value)
        {
            char *trend_field = strstr(obj_start, "\"TrendArrow\"");
            if (trend_field && trend_field < obj_end)
            {
                double trend_val = extract_numeric_value(trend_field, obj_end);
                trend_arrow = (int)trend_val;
            }
            
            // Extract FactoryTimestamp (UTC) from the same glucoseMeasurement object
            char *timestamp_field = strstr(obj_start, "\"FactoryTimestamp\"");
            if (!timestamp_field || timestamp_field >= obj_end)
            {
                // Fallback to legacy Timestamp field
                timestamp_field = strstr(obj_start, "\"Timestamp\"");
            }
            
            if (timestamp_field && timestamp_field < obj_end)
            {
                char timestamp_str[64];
                if (extract_string_value(timestamp_field, obj_end, timestamp_str, sizeof(timestamp_str)))
                {
                    // Check if this is FactoryTimestamp (UTC) or legacy Timestamp
                    if (strstr(timestamp_field, "\"FactoryTimestamp\"") != NULL)
                    {
                        timestamp = parse_utc_timestamp(timestamp_str);
                    }
                    else
                    {
                        // Legacy Timestamp format
                        timestamp = parse_freestyle_timestamp(timestamp_str);
                    }
                }
            }
            
            // Update latest values if this measurement is newer (or if we don't have one yet)
            if (timestamp > latest_timestamp || latest_timestamp == 0)
            {
                latest_glucose_value = glucose_value;
                latest_trend_arrow = trend_arrow;
                latest_timestamp = timestamp;
            }
        }
        
        // Move search position past the end of this object to find next occurrence
        search_pos = obj_end + 1;
    }
}

// Function to handle redirects or determine base URL based on region
static const char *get_freestyle_base_url()
{
    if (eeprom_libre_region_url[0] != '\0')
    {
        return eeprom_libre_region_url;
    }

    switch (eeprom_libre_region)
    {
    case 1:
        return LIBRE_US_BASE_URL;
    case 2:
        return LIBRE_EU_BASE_URL;
    case 3:
        return LIBRE_DE_BASE_URL;
    case 4:
        return LIBRE_FR_BASE_URL;
    case 5:
        return LIBRE_JP_BASE_URL;
    case 6:
        return LIBRE_AU_BASE_URL;
    case 7:
        return LIBRE_BASE_URL; // Global/RoW
    default:
        return LIBRE_BASE_URL;
    }
}

// HTTP event handler for Freestyle requests
static esp_err_t freestyle_http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Freestyle HTTP Error");
        freestyle_response_len = 0;
        if (is_glucose_chunk_mode)
        {
            glucose_chunk_buffer_len = 0;
            latest_glucose_value = 0.0;
            latest_trend_arrow = 0;
        }
        break;
    case HTTP_EVENT_ON_DATA:
        if (is_glucose_chunk_mode)
        {
            // Process chunks incrementally for glucose fetch
            if (evt->data_len > 0)
            {
                process_glucose_chunk(evt->data, evt->data_len);                
            }
        }
        else
        {
            // Original buffer-based processing for other requests
            // Ensure we have a valid buffer and the data will fit
            if (freestyle_response_buffer == NULL)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer is NULL");
                return ESP_FAIL;
            }

            // Check if we have enough space
            if (evt->data_len <= 0 || freestyle_response_len < 0 ||
                (freestyle_response_len + evt->data_len) >= HTTP_BUFFER_SIZE)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Buffer overflow prevented: len=%d, data_len=%d, max=%d",
                            freestyle_response_len, evt->data_len, HTTP_BUFFER_SIZE);
                freestyle_response_len = 0;
                return ESP_FAIL;
            }

            // Safe to copy the data
            memcpy(freestyle_response_buffer + freestyle_response_len, evt->data, evt->data_len);
            freestyle_response_len += evt->data_len;
            freestyle_response_buffer[freestyle_response_len] = '\0';
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Freestyle HTTP Event: FINISH");
        break;
    default:
        break;
    }
    return ESP_OK;
}

bool init_freestyle_client(void)
{
    if (freestyle_client_initialized)
        return true;

    glucose_data.trend_arrow = -1;


    const char *base_url = get_freestyle_base_url();
    if (!base_url || base_url[0] == '\0')
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid base URL for Freestyle");
        return false;
    }

    // Initialize the client with base configuration
    esp_http_client_config_t config = {
        .url = base_url, // Just use the base URL initially
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .is_async = false,
        .buffer_size = HTTP_BUFFER_SIZE,
        .buffer_size_tx = HTTP_BUFFER_SIZE,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .tls_version = ESP_TLS_VER_TLS_1_2,        
        .user_agent = "Frixos HTTP Client",
        .event_handler = freestyle_http_event_handler,
    };

    freestyle_client = esp_http_client_init(&config);
    if (!freestyle_client)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize Freestyle client");
        return false;
    }

    // Set common headers
    esp_http_client_set_header(freestyle_client, "Content-Type", "application/json");
    esp_http_client_set_header(freestyle_client, "Accept", "application/json");
    // Don't request gzip compression - let ESP-IDF handle it automatically if needed
    esp_http_client_set_header(freestyle_client, "cache-control", "no-cache");
    // Use "close" instead of "Keep-Alive" to free resources after each request
    esp_http_client_set_header(freestyle_client, "connection", "close");
    esp_http_client_set_header(freestyle_client, "product", "llu.android");
    esp_http_client_set_header(freestyle_client, "version", LIBRE_CLIENT_VERSION);

    freestyle_client_initialized = true;
    return true;
}

void cleanup_freestyle_client(void)
{
    if (freestyle_client)
    {
        esp_http_client_cleanup(freestyle_client);
        freestyle_client = NULL;
    }
    freestyle_client_initialized = false;
}

bool login_freestyle(void)
{  
    
    // Guard against recursive calls
    if (login_freestyle_in_progress)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "login_freestyle already in progress, skipping duplicate call");
        return false;
    }
    login_freestyle_in_progress = true;
    
    if (eeprom_glucose_username[0] == '\0' || eeprom_glucose_password[0] == '\0')
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Libre credentials not set");
        login_freestyle_in_progress = false;
        return false;
    }

    // Initialize client if not already done
    if (!freestyle_client_initialized)
        if (!init_freestyle_client())
        {
            login_freestyle_in_progress = false;
            return false;
        }

    char login_url[128];
    snprintf(login_url, sizeof(login_url), "%s/llu/auth/login", get_freestyle_base_url());

    // Check heap memory before attempting SSL connection
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    if (free_heap < 20000) // Less than 20KB free - SSL needs significant memory
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Insufficient heap memory for SSL connection: %d bytes free (need ~20KB)", free_heap);
        login_freestyle_in_progress = false;
        return false;
    }

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("login_freestyle"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for Freestyle login");
        login_freestyle_in_progress = false;
        return false;
    }

    freestyle_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "FREESTYLE_HTTP");
    if (!freestyle_response_buffer)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to allocate response buffer (heap: %d bytes free)", heap_caps_get_free_size(MALLOC_CAP_8BIT));
        release_ssl_semaphore();
        login_freestyle_in_progress = false;
        return false;
    }
    freestyle_response_len = 0;

    // Update URL and method for this request
    esp_http_client_set_url(freestyle_client, login_url);
    esp_http_client_set_method(freestyle_client, HTTP_METHOD_POST);

    cJSON *login_json = cJSON_CreateObject();
    cJSON_AddStringToObject(login_json, "email", eeprom_glucose_username);
    cJSON_AddStringToObject(login_json, "password", eeprom_glucose_password);
    char *post_data = cJSON_PrintUnformatted(login_json);
    cJSON_Delete(login_json);

    esp_http_client_set_post_field(freestyle_client, post_data, strlen(post_data));

    bool success = false;
    esp_err_t http_err = esp_http_client_perform(freestyle_client);
    
    if (http_err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(freestyle_client);
        if (status_code == 200 && freestyle_response_len > 0)
        {

            // Extract token and account ID directly from JSON string without full parsing
            // Look for "authTicket" then find "token" within it
            char *auth_ticket = strstr(freestyle_response_buffer, "\"authTicket\"");
            if (auth_ticket)
            {
                char *token_field = strstr(auth_ticket, "\"token\"");
                if (token_field)
                {
                    char *colon = strchr(token_field, ':');
                    if (colon)
                    {
                        char *value_start = colon + 1;
                        // Skip whitespace
                        while (*value_start == ' ' || *value_start == '\t')
                            value_start++;
                        // Skip opening quote if present
                        if (*value_start == '"')
                            value_start++;
                        char *value_end = value_start;
                        // Find end of value (quote, comma, or brace)
                        while (*value_end && *value_end != '"' && *value_end != ',' && *value_end != '}')
                            value_end++;
                        int token_len = value_end - value_start;
                        if (token_len > 0 && token_len < sizeof(eeprom_libre_token))
                        {
                            strncpy(eeprom_libre_token, value_start, token_len);
                            eeprom_libre_token[token_len] = '\0';
                        }
                    }
                }
            }

            // Look for "user" then find "id" within it
            char *user_obj = strstr(freestyle_response_buffer, "\"user\"");
            if (user_obj)
            {
                char *id_field = strstr(user_obj, "\"id\"");
                if (id_field)
                {
                    char *colon = strchr(id_field, ':');
                    if (colon)
                    {
                        char *value_start = colon + 1;
                        // Skip whitespace
                        while (*value_start == ' ' || *value_start == '\t')
                            value_start++;
                        // Skip opening quote if present
                        if (*value_start == '"')
                            value_start++;
                        char *value_end = value_start;
                        // Find end of value (quote, comma, or brace)
                        while (*value_end && *value_end != '"' && *value_end != ',' && *value_end != '}')
                            value_end++;
                        int id_len = value_end - value_start;
                        if (id_len > 0 && id_len < sizeof(libre_account_id))
                        {
                            strncpy(libre_account_id, value_start, id_len);
                            libre_account_id[id_len] = '\0';
                        }
                    }
                }
            }

            if (eeprom_libre_token[0] != '\0' && libre_account_id[0] != '\0')
            {
                success = true;
           }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "login_freestyle: Failed to extract token or account ID from response");
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "login_freestyle: Login failed, status: %d", status_code);
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "login_freestyle: HTTP request failed: %s", esp_err_to_name(http_err));
        
        // If SSL handshake failed, the client might be in a bad state - recreate it
        // Check for connection failures (ESP_FAIL is returned for SSL handshake failures)
        if (http_err == ESP_FAIL)
        {
            esp_http_client_cleanup(freestyle_client);
            freestyle_client = NULL;
            freestyle_client_initialized = false;
            
            // Try to reinitialize the client for next time
            if (!init_freestyle_client())
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "login_freestyle: Failed to recreate client after SSL failure");
            }
        }
    }

    free(post_data);
    release_shared_buffer(freestyle_response_buffer);
    freestyle_response_buffer = NULL;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    
    // Clear the in-progress flag
    login_freestyle_in_progress = false;
    return success;
}

static bool fetch_patient_id(void)
{    
    if (eeprom_libre_token[0] == '\0')
        return false;

    if (!freestyle_client_initialized)
        if (!init_freestyle_client())
            return false;
    
    char connections_url[128];
    snprintf(connections_url, sizeof(connections_url), "%s/llu/connections", get_freestyle_base_url());

    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("fetch_patient_id"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for fetch_patient_id");
        return false;
    }

    freestyle_response_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "FREESTYLE_HTTP");
    if (!freestyle_response_buffer)
    {
        release_ssl_semaphore();
        return false;
    }
    freestyle_response_len = 0;

    // Update URL and method for this request
    esp_http_client_set_url(freestyle_client, connections_url);
    esp_http_client_set_method(freestyle_client, HTTP_METHOD_GET);

    // Use local buffer for auth header (Bearer token can be up to 512 bytes + "Bearer " prefix)
    char auth_header[512 + 7 + 1]; // "Bearer " (7) + token (512) + null terminator (1)
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", eeprom_libre_token);
    esp_http_client_set_header(freestyle_client, "authorization", auth_header);
    char id_hash[65];
    generateSHA256((const unsigned char*)libre_account_id, id_hash);    
    esp_http_client_set_header(freestyle_client, "account-id", id_hash);
    

    bool success = false;
    if (esp_http_client_perform(freestyle_client) == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(freestyle_client);
        if (status_code == 200 && freestyle_response_len > 0)
        {
            // Extract patientId from first element in data array without JSON parsing
            // Look for "patientId" field (should be in first array element)
            char *patient_id_field = strstr(freestyle_response_buffer, "\"patientId\"");
            if (patient_id_field)
            {
                char *colon = strchr(patient_id_field, ':');                
                if (colon)
                {
                    char *value_start = colon + 1;
                    // Skip whitespace
                    while (*value_start == ' ' || *value_start == '\t')
                        value_start++;
                    // Skip opening quote if present
                    if (*value_start == '"')
                        value_start++;
                    char *value_end = value_start;
                    // Find end of value (quote, comma, or brace)
                    while (*value_end && *value_end != '"' && *value_end != ',' && *value_end != '}')
                        value_end++;
                    int id_len = value_end - value_start;
                    if (id_len > 0 && id_len < sizeof(eeprom_libre_patient_id))
                    {
                        strncpy(eeprom_libre_patient_id, value_start, id_len);
                        eeprom_libre_patient_id[id_len] = '\0';
                        success = true;                        
                    }
                }
            }
        }
    }

    release_shared_buffer(freestyle_response_buffer);
    freestyle_response_buffer = NULL;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    return success;
}

bool fetch_freestyle_glucose(void)
{

    if (!freestyle_client_initialized)
    {
        if (!init_freestyle_client())
        {
            return false;
        }
    }

    if (eeprom_libre_token[0] == '\0' || libre_account_id[0] == '\0')
    {
        if (!login_freestyle())
            return false;
    }

    if (eeprom_libre_patient_id[0] == '\0')
    {
        if (!fetch_patient_id())
            return false;
    }

    char graph_url[256];
    snprintf(graph_url, sizeof(graph_url), "%s/llu/connections/%s/graph", get_freestyle_base_url(), eeprom_libre_patient_id);  

    // Enable chunk processing mode for glucose fetch
    is_glucose_chunk_mode = true;
    glucose_chunk_buffer_len = 0;
    latest_glucose_value = 0.0;
    latest_trend_arrow = 0;
    latest_timestamp = 0;

    // Update URL and method for this request
    esp_http_client_set_url(freestyle_client, graph_url);
    esp_http_client_set_method(freestyle_client, HTTP_METHOD_GET);

    // headers set from previous requests, no need to set them again
    
    // Acquire SSL connection semaphore before making SSL connection
    if (!acquire_ssl_semaphore("fetch_freestyle_glucose"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to acquire SSL semaphore for fetch_freestyle_glucose");
        is_glucose_chunk_mode = false;
        return false;
    }
    
    bool success = false;
    if (esp_http_client_perform(freestyle_client) == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(freestyle_client);
        if (status_code == 200)
        {
            success = true; // HTTP request succeeded
            // Use the latest glucose value extracted from chunks
            if (latest_timestamp > glucose_data.timestamp)
            {
                glucose_data.previous_gl_mgdl = glucose_data.current_gl_mgdl;
                glucose_data.current_gl_mgdl = latest_glucose_value;
                if (glucose_data.previous_gl_mgdl > 0)            
                    glucose_data.gl_diff = glucose_data.current_gl_mgdl - glucose_data.previous_gl_mgdl;
                else
                    glucose_data.gl_diff = 0;             
                // Map Freestyle trend_arrow (1-5) to standard values (0-4)
                // Freestyle: 1=down fast, 2=down, 3=stable, 4=up, 5=up fast
                // Standard:  0=down fast, 1=down, 2=stable, 3=up, 4=up fast
                if (latest_trend_arrow >= 1 && latest_trend_arrow <= 5)
                    glucose_data.trend_arrow = latest_trend_arrow - 1; // Map 1-5 to 0-4
                else
                    glucose_data.trend_arrow = -1; // no arrow if invalid or 0
                // Use timestamp from CGM data if available, otherwise use current time
                if (latest_timestamp > 0)
                {
                    glucose_data.timestamp = latest_timestamp;
                }
                else
                {
                    time(&glucose_data.timestamp); // Fallback to current time if parsing failed
                }
                success = true; // Data was updated
                if (eeprom_glucose_unit == 1)
                {
                    float glucose_mmol = glucose_data.current_gl_mgdl / 18.0182f;
                    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "fetch_freestyle_glucose: ok, glucose: %.1f mmol/L, trend: %i", glucose_mmol, glucose_data.trend_arrow);
                }
                else
                {
                    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "fetch_freestyle_glucose: ok, glucose: %.0f mg/dL, trend: %i", glucose_data.current_gl_mgdl, glucose_data.trend_arrow);
                }
            }
            else
            {
                // Data not newer, but HTTP request was successful
                // Return true so timestamp gets updated to respect refresh interval
                if (glucose_data.timestamp != latest_timestamp)
                {
                    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "fetch_freestyle_glucose: not newer; value: %.0f mg/dL, timestamp: %ld", glucose_data.current_gl_mgdl, (long)glucose_data.timestamp);
                    success = true; // Data was updated
                }                
            }
        }
        else if (status_code == 401)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "fetch_freestyle_glucose: unauthorized, retrying login");
            eeprom_libre_token[0] = '\0'; // Trigger re-login next time
        }
    }

    // Disable chunk processing mode
    is_glucose_chunk_mode = false;
    glucose_chunk_buffer_len = 0;
    
    // Always release SSL semaphore before returning
    release_ssl_semaphore();
    
    // Note: We do NOT clean up the client here because:
    // 1. The client is shared between login_freestyle(), fetch_patient_id(), and fetch_freestyle_glucose()
    // 2. Headers are set on the client and persist across calls
    // 3. The client needs to remain initialized for the next glucose fetch cycle
    
    return success;
}

void format_glucose_token(char *buffer, size_t buffer_size)
{
    if (glucose_data.current_gl_mgdl <= 0)
    {
        snprintf(buffer, buffer_size, "Glucose: N/A");
        return;
    }

    struct tm timeinfo;
    localtime_r(&glucose_data.timestamp, &timeinfo);
    char time_str[32];
    if (eeprom_12hour)
    {
        strftime(time_str, sizeof(time_str), "%I:%M%p", &timeinfo);
    }
    else
    {
        strftime(time_str, sizeof(time_str), "%H:%M", &timeinfo);
    }

    if (eeprom_glucose_unit == 1)
    {
        // Display in mmol/L with 1 decimal place
        float glucose_mmol = glucose_data.current_gl_mgdl / 18.0182f;
        float diff_mmol = glucose_data.gl_diff / 18.0182f;
        // Format with arrow (Freestyle style) in mmol/L
        snprintf(buffer, buffer_size, "%.1f mmol/l (%+.2f @ %s)",
                 glucose_mmol,
                 diff_mmol,
                 time_str);
    }
    else
    {
        // Format with arrow (Freestyle style) in mg/dL
        snprintf(buffer, buffer_size, "%.0f mg/dl (%+.0f @ %s)",
                 glucose_data.current_gl_mgdl,
                 glucose_data.gl_diff,
                 time_str);
    }
}

// Get plain glucose reading (just the number, no formatting)
void get_glucose_reading_plain(char *buffer, size_t buffer_size)
{
    if (glucose_data.current_gl_mgdl <= 0)
    {
        snprintf(buffer, buffer_size, "-");
        return;
    }

    if (eeprom_glucose_unit == 1)
    {
        // Display in mmol/L with 1 decimal place
        float glucose_mmol = glucose_data.current_gl_mgdl / 18.0182f;
        snprintf(buffer, buffer_size, "%.1f", glucose_mmol);
    }
    else
    {
        // Display in mg/dL as integer
        snprintf(buffer, buffer_size, "%.0f", glucose_data.current_gl_mgdl);
    }
}
