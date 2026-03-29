/**
 * ESP-IDF Web Configuration Interface
 * Implementation file for settings management
 */

#include "f-settings.h"
#include "f-integrations.h"
#include "f-provisioning.h"
#include "f-membuffer.h"
#include "frixos.h"
#include "f-pwm.h"
#include "f-wifi.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_clk_tree.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "unistd.h"

/*
 * Field Name Mappings (pXX format to reduce HTTP header size):
 *
 * Basic Settings:
 * - p00 = hostname (Device hostname)
 * - p34 = wifi_ssid (WiFi SSID)
 * - p35 = wifi_pass (WiFi password)
 * - p36 = fahrenheit (Temperature unit)
 * - p37 = hour12 (12-hour format)
 * - p39 = update_firmware (Auto firmware update)
 * - p38 = scroll_speed (Scroll speed)
 * - p40 = dark_theme (Dark theme)
 * - p41 = language (Language index: 0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da)
 *
 * Advanced Settings:
 * - p01 = ofs_x (X offset)
 * - p02 = ofs_y (Y offset)
 * - p03 = rotation (Display rotation)
 * - p04 = dayfont (Day font)
 * - p05 = nightfont (Night font)
 * - p06 = quiet_scroll (Show scrolling message)
 * - p07 = quiet_weather (Show weather forecast)
 * - p08 = show_grid (Show grid)
 * - p09 = mirroring (Mirror display)
 * - p10 = color_filter (Day color filter)
 * - p11 = night_color_filter (Night color filter)
 * - p12 = msg_color (Message color)
 * - p13 = msg_font (Message font)
 * - p14 = scroll_delay (Scroll delay)
 * - p15 = night_msg_color (Night message color)
 * - p16 = message (Scrolling message)
 * - p17 = lat (Latitude)
 * - p18 = lon (Longitude)
 * - p19 = timezone (Timezone)
 * - p20 = lux_sensitivity (Light sensitivity)
 * - p21 = lux_threshold (Light threshold)
 * - p22 = dim_disable (Maintain full brightness)
 * - p23 = brightness_LED (LED brightness array)
 * - p24 = show_leading_zero (Show leading zero)
 * - p50 = dots_breathe (Disable breathing time dots)
 * - p42 = pwm_frequency (PWM frequency in Hz, range 10-1000000)
 * - p43 = max_power (Max power, range 1-1023)
 *
 * Integration Settings (shortened):
 * - p25 = eeprom_ha_url (Home Assistant URL)
 * - p26 = eeprom_ha_token (Home Assistant token)
 * - p27 = eeprom_ha_refresh_mins (HA refresh interval)
 * - p28 = eeprom_stock_key (Stock API key)
 * - p29 = eeprom_stock_refresh_mins (Stock refresh interval)
 * - p30 = eeprom_dexcom_region (Dexcom region)
 * - p31 = eeprom_glucose_username (Shared glucose username)
 * - p32 = eeprom_glucose_password (Shared glucose password)
 * - p33 = eeprom_glucose_refresh (Shared glucose refresh interval)
 * - p45 = glucose_validity_duration (Glucose data validity duration in minutes)
 * - p48 = eeprom_sec_time (Alternate time display duration in seconds)
 * - p49 = eeprom_sec_cgm (Alternate CGM display duration in seconds)
 * - p44 = eeprom_libre_region (Libre region)
 * - p54 = eeprom_ns_url (Nightscout URL, max 100 chars)
 */

// Tag for logging
static const char *TAG = "f-settings";

// HTTP server handle
static httpd_handle_t server = NULL;

// Server state tracking
static struct
{
    int active_connections;
    SemaphoreHandle_t mutex;
    bool is_restarting;
} server_state = {0, NULL, false};

// Helper function declarations
static bool check_memory_available(void);

// Add memory tracking structure
static struct
{
    size_t peak_usage;
    size_t current_usage;
    SemaphoreHandle_t mutex;
} memory_stats = {0, 0, NULL};

// Add memory tracking function
static void track_memory_allocation(size_t size)
{
    if (memory_stats.mutex && xSemaphoreTake(memory_stats.mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        memory_stats.current_usage += size;
        if (memory_stats.current_usage > memory_stats.peak_usage)
        {
            memory_stats.peak_usage = memory_stats.current_usage;
        }
        xSemaphoreGive(memory_stats.mutex);
    }
}

static void track_memory_free(size_t size)
{
    if (memory_stats.mutex && xSemaphoreTake(memory_stats.mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (memory_stats.current_usage >= size)
        {
            memory_stats.current_usage -= size;
        }
        xSemaphoreGive(memory_stats.mutex);
    }
}

// Helper function to check memory availability
static bool check_memory_available(void)
{
    size_t free_heap = esp_get_free_heap_size();
    size_t min_free_heap = MIN_FREE_HEAP;

    // Add extra margin for HTTP operations
    if (memory_stats.current_usage > 0)
    {
        min_free_heap += memory_stats.current_usage;
    }

    if (free_heap < min_free_heap)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Low memory: %d bytes free, %d bytes in use",
                    free_heap, memory_stats.current_usage);
        return false;
    }
    return true;
}

// WiFi scanning variables
static wifi_ap_record_t ap_records[MAX_AP_SCAN];
static uint16_t ap_count = 0;
static bool wifi_scan_running = false;
static bool wifi_scan_done = false;
static SemaphoreHandle_t scan_semaphore = NULL;

// Event handler for WiFi events
static void wifi_scan_event_handler(void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi scan completed event received");

        // Get scan results
        uint16_t scan_count = MAX_AP_SCAN;
        memset(ap_records, 0, sizeof(ap_records));
        esp_err_t err = esp_wifi_scan_get_ap_records(&scan_count, ap_records);

        if (err == ESP_OK)
        {
            ap_count = scan_count;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Found %d networks", ap_count);

            // Sort networks by RSSI (signal strength)
            for (int i = 0; i < ap_count - 1; i++)
            {
                for (int j = i + 1; j < ap_count; j++)
                {
                    if (ap_records[j].rssi > ap_records[i].rssi)
                    {
                        wifi_ap_record_t temp = ap_records[i];
                        ap_records[i] = ap_records[j];
                        ap_records[j] = temp;
                    }
                }
            }

            // Print found networks for debugging
            for (int i = 0; i < ap_count && i < 5; i++)
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Network %d: SSID: %s, RSSI: %d, Auth: %d",
                            i, ap_records[i].ssid, ap_records[i].rssi, ap_records[i].authmode);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to get scan results: %s", esp_err_to_name(err));
            ap_count = 0;
        }

        wifi_scan_running = false;
        wifi_scan_done = true;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Scan complete, status updated: running=%d, done=%d",
                    wifi_scan_running, wifi_scan_done);

        // Signal that scan is complete
        if (scan_semaphore != NULL)
        {
            xSemaphoreGive(scan_semaphore);
        }
    }
}

// Function to start WiFi network scan
static esp_err_t start_wifi_scan(void)
{
    // Check if a scan is already running
    if (wifi_scan_running)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Scan already in progress");
        return ESP_ERR_INVALID_STATE;
    }

    wifi_scan_running = true;
    wifi_scan_done = false;

    // Ensure we're in the right mode for scanning
    wifi_mode_t current_mode;
    esp_err_t mode_ret = esp_wifi_get_mode(&current_mode);
    if (mode_ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to get WiFi mode: %s", esp_err_to_name(mode_ret));
    }
    else if (current_mode != WIFI_MODE_APSTA && current_mode != WIFI_MODE_STA)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Setting WiFi mode to APSTA for scanning");
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    // Set scan configuration
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 100,
        .scan_time.active.max = 300};

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Starting WiFi scan...");

    // Start scan in non-blocking mode
    esp_err_t ret = esp_wifi_scan_start(&scan_config, false);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start scan: %s", esp_err_to_name(ret));
        wifi_scan_running = false;

        // Even if scan fails, set the done flag to true so we don't get stuck
        wifi_scan_done = true;
        return ret;
    }

    return ESP_OK;
}

// Generic file handler for static files (HTML, CSS, JS)
esp_err_t generic_file_handler(httpd_req_t *req)
{
    // Check for captive portal detection endpoints
    const char *uri = req->uri;
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Serving file for URI: %s", uri);

    // Get the filename from the URI by stripping the leading slash
    const char *filename = uri;
    if (filename[0] == '/')
    {
        filename++;
    }

    // Use default file if the URI is the root or empty
    if (strlen(filename) == 0)
    {
        filename = "index.html";
    }

    // Log the requested file
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Serving file: %s", filename);

    // Try multiple possible paths for the file
    FILE *file = NULL;
    esp_err_t ret = ESP_FAIL;
    // Try to open file from SPIFFS
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "/spiffs/%.110s", filename);

    file = fopen(filepath, "r");
    if (file != NULL)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "File found at: %s", filepath);
        ret = ESP_OK;
    }

    if (ret != ESP_OK)
    {
        // If file not found, redirect to captive portal
        httpd_resp_set_status(req, "302 Found");
        char location[256];
        snprintf(location, sizeof(location), "http://%s/index.html", portal_address_str);
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "File %s not found, redirecting to: %s", filepath, location);
        httpd_resp_set_hdr(req, "Location", location);
        httpd_resp_set_hdr(req, "X-Apple-Captive", "1");
        httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
        httpd_resp_set_hdr(req, "Connection", "close");

        char response[256];
        snprintf(response, sizeof(response),
                 "<html><body>Redirecting... <a href='http://%s/index.html'>Click here</a> if not redirected.</body></html>",
                 portal_address_dns);
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    // Set content type based on file extension
    if (strstr(filename, ".html"))
    {
        httpd_resp_set_type(req, "text/html");
    }
    else if (strstr(filename, ".css"))
    {
        httpd_resp_set_type(req, "text/css");
    }
    else if (strstr(filename, ".js"))
    {
        httpd_resp_set_type(req, "application/javascript");
    }
    else if (strstr(filename, ".json"))
    {
        httpd_resp_set_type(req, "application/json");
    }
    else if (strstr(filename, ".jpg") || strstr(filename, ".jpeg"))
    {
        httpd_resp_set_type(req, "image/jpeg");
    }
    else if (strstr(filename, ".png"))
    {
        httpd_resp_set_type(req, "image/png");
    }

    // Read and send file in chunks
    char chunk[1024];
    size_t bytes_read;
    while ((bytes_read = fread(chunk, 1, sizeof(chunk), file)) > 0)
    {
        if (httpd_resp_send_chunk(req, chunk, bytes_read) != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to send chunk");
            fclose(file);
            return ESP_FAIL;
        }
    }
    httpd_resp_send_chunk(req, NULL, 0); // Send empty chunk to end response
    fclose(file);
    return ESP_OK;
}

#define MAX_LOG_LINE_LENGTH 256 // Maximum characters per log line chunk

// Helper function to get query parameter value from URI
// Returns static buffer with parameter value (caller should copy if needed)
static char query_param_buffer[256];
static const char *get_query_param(const char *uri, const char *param_name)
{
    const char *query_start = strchr(uri, '?');
    if (!query_start)
    {
        return NULL;
    }

    query_start++; // Skip '?'
    size_t param_name_len = strlen(param_name);
    const char *param_pos = query_start;

    while (*param_pos)
    {
        // Check if current position matches parameter name
        if (strncmp(param_pos, param_name, param_name_len) == 0 && param_pos[param_name_len] == '=')
        {
            const char *value_start = param_pos + param_name_len + 1; // Value after '='
            const char *value_end = strchr(value_start, '&');
            if (!value_end)
            {
                value_end = value_start + strlen(value_start); // End of string
            }

            size_t value_len = value_end - value_start;
            if (value_len >= sizeof(query_param_buffer))
            {
                value_len = sizeof(query_param_buffer) - 1;
            }

            strncpy(query_param_buffer, value_start, value_len);
            query_param_buffer[value_len] = '\0';
            return query_param_buffer;
        }

        // Move to next parameter (after '&' or end of string)
        param_pos = strchr(param_pos, '&');
        if (!param_pos)
            break;
        param_pos++; // Skip '&'
    }

    return NULL;
}

// Helper function to check if a parameter should be included based on filter
static bool should_include_param(const char *param_key, const char *group_filter, const char *params_filter)
{
    // If no filters specified, include all
    if (!group_filter && !params_filter)
    {
        return true;
    }

    // Check params filter (comma-separated list like "p40,p41")
    if (params_filter)
    {
        const char *search_start = params_filter;
        size_t param_len = strlen(param_key);

        while (*search_start)
        {
            // Skip whitespace
            while (*search_start == ' ' || *search_start == ',')
            {
                search_start++;
            }
            if (!*search_start)
                break;

            // Find next comma or end
            const char *param_end = strchr(search_start, ',');
            size_t check_len = param_end ? (size_t)(param_end - search_start) : strlen(search_start);

            // Trim trailing whitespace
            while (check_len > 0 && (search_start[check_len - 1] == ' '))
            {
                check_len--;
            }

            // Check if this matches
            if (check_len == param_len && strncmp(search_start, param_key, param_len) == 0)
            {
                return true;
            }

            if (!param_end)
                break;
            search_start = param_end + 1;
        }
    }

    // Check group filter
    if (group_filter)
    {
        if (strcmp(group_filter, "theme") == 0)
        {
            // Theme group: p40, p41
            return (strcmp(param_key, "p40") == 0 || strcmp(param_key, "p41") == 0);
        }
        else if (strcmp(group_filter, "settings") == 0)
        {
            // Settings group: p00, p34-p39
            return (strcmp(param_key, "p00") == 0 ||
                    strcmp(param_key, "p34") == 0 || strcmp(param_key, "p35") == 0 ||
                    strcmp(param_key, "p36") == 0 || strcmp(param_key, "p37") == 0 ||
                    strcmp(param_key, "p39") == 0);
        }
        else if (strcmp(group_filter, "advanced") == 0)
        {
            // Advanced group: p01-p24, p42-p43
            return (strcmp(param_key, "p01") == 0 || strcmp(param_key, "p02") == 0 ||
                    strcmp(param_key, "p03") == 0 || strcmp(param_key, "p04") == 0 ||
                    strcmp(param_key, "p05") == 0 || strcmp(param_key, "p06") == 0 ||
                    strcmp(param_key, "p07") == 0 || strcmp(param_key, "p08") == 0 ||
                    strcmp(param_key, "p09") == 0 || strcmp(param_key, "p10") == 0 ||
                    strcmp(param_key, "p11") == 0 || strcmp(param_key, "p12") == 0 ||
                    strcmp(param_key, "p13") == 0 || strcmp(param_key, "p14") == 0 ||
                    strcmp(param_key, "p15") == 0 || strcmp(param_key, "p16") == 0 ||
                    strcmp(param_key, "p17") == 0 || strcmp(param_key, "p18") == 0 ||
                    strcmp(param_key, "p19") == 0 || strcmp(param_key, "p20") == 0 ||
                    strcmp(param_key, "p21") == 0 || strcmp(param_key, "p22") == 0 ||
                    strcmp(param_key, "p23") == 0 || strcmp(param_key, "p24") == 0 ||
                    strcmp(param_key, "p42") == 0 || strcmp(param_key, "p43") == 0 ||
                    strcmp(param_key, "p46") == 0 || strcmp(param_key, "p47") == 0 ||
                    strcmp(param_key, "p50") == 0);
        }
        else if (strcmp(group_filter, "integrations") == 0)
        {
            // Integrations group: p25-p33, p44, p45, p48, p49, p51-p54
            return (strcmp(param_key, "p25") == 0 || strcmp(param_key, "p26") == 0 ||
                    strcmp(param_key, "p27") == 0 || strcmp(param_key, "p28") == 0 ||
                    strcmp(param_key, "p29") == 0 || strcmp(param_key, "p30") == 0 ||
                    strcmp(param_key, "p31") == 0 || strcmp(param_key, "p32") == 0 ||
                    strcmp(param_key, "p33") == 0 || strcmp(param_key, "p45") == 0 ||
                    strcmp(param_key, "p44") == 0 || strcmp(param_key, "p48") == 0 ||
                    strcmp(param_key, "p49") == 0 || strcmp(param_key, "p51") == 0 ||
                    strcmp(param_key, "p52") == 0 || strcmp(param_key, "p53") == 0 ||
                    strcmp(param_key, "p54") == 0);
        }
    }

    return false;
}

esp_err_t send_json_settings(httpd_req_t *req)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Settings GET request received");
    esp_err_t ret = ESP_OK;
    cJSON *root = cJSON_CreateObject();

    // Parse query parameters
    const char *group_param = get_query_param(req->uri, "group");
    const char *params_param = get_query_param(req->uri, "params");

    if (group_param || params_param)
    {
        ESP_LOG_WEB(ESP_LOG_DEBUG, TAG, "Query params: group=%s, params=%s",
                    group_param ? group_param : "none", params_param ? params_param : "none");
    }

    // Add parameters based on filter
    if (should_include_param("p00", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p00", eeprom_hostname);
    }
    if (should_include_param("p34", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p34", eeprom_wifi_ssid);
    }
    if (should_include_param("p35", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p35", eeprom_wifi_pass);
    }
    if (should_include_param("p17", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p17", eeprom_lat);
    }
    if (should_include_param("p18", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p18", eeprom_lon);
    }
    if (should_include_param("p19", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p19", eeprom_timezone);
    }
    if (should_include_param("p46", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p46", eeprom_wifi_start);
    }
    if (should_include_param("p47", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p47", eeprom_wifi_end);
    }
    if (should_include_param("p04", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p04", eeprom_font[0]);
    }
    if (should_include_param("p05", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p05", eeprom_font[1]);
    }
    if (should_include_param("p20", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p20", eeprom_lux_sensitivity);
    }
    if (should_include_param("p21", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p21", eeprom_lux_threshold);
    }
    if (should_include_param("p22", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p22", eeprom_dim_disable);
    }
    if (should_include_param("p16", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p16", eeprom_message);
    }

    if (should_include_param("p23", group_param, params_param))
    {
        cJSON *brightness_array = cJSON_CreateArray();
        for (int i = 0; i < 2; i++)
        {
            cJSON_AddItemToArray(brightness_array, cJSON_CreateNumber(eeprom_brightness_LED[i]));
        }
        cJSON_AddItemToObject(root, "p23", brightness_array);
    }

    if (should_include_param("p36", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p36", eeprom_fahrenheit);
    }
    if (should_include_param("p37", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p37", eeprom_12hour);
    }
    if (should_include_param("p24", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p24", eeprom_show_leading_zero);
    }
    if (should_include_param("p50", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p50", eeprom_dots_breathe);
    }
    if (should_include_param("p06", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p06", eeprom_quiet_scroll);
    }
    if (should_include_param("p07", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p07", eeprom_quiet_weather);
    }
    if (should_include_param("p10", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p10", eeprom_color_filter[0]);
    }
    if (should_include_param("p11", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p11", eeprom_color_filter[1]);
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Sending night_color_filter in JSON response: %u", eeprom_color_filter[1]);
    }

    if (should_include_param("p12", group_param, params_param))
    {
        char day_color_hex[8];
        snprintf(day_color_hex, sizeof(day_color_hex), "#%02x%02x%02x",
                 eeprom_msg_red[0], eeprom_msg_green[0], eeprom_msg_blue[0]);
        cJSON_AddStringToObject(root, "p12", day_color_hex);
    }

    if (should_include_param("p15", group_param, params_param))
    {
        char night_color_hex[8];
        snprintf(night_color_hex, sizeof(night_color_hex), "#%02x%02x%02x",
                 eeprom_msg_red[1], eeprom_msg_green[1], eeprom_msg_blue[1]);
        cJSON_AddStringToObject(root, "p15", night_color_hex);
    }

    if (should_include_param("p13", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p13", eeprom_msg_font);
    }

    if (should_include_param("p01", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p01", eeprom_ofs_x);
    }
    if (should_include_param("p02", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p02", eeprom_ofs_y);
    }
    if (should_include_param("p03", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p03", eeprom_rotation);
    }
    if (should_include_param("p09", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p09", eeprom_mirroring);
    }
    if (should_include_param("p08", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p08", eeprom_show_grid);
    }
    if (should_include_param("p38", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p38", eeprom_scroll_speed);
    }
    if (should_include_param("p14", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p14", eeprom_scroll_delay);
    }
    if (should_include_param("p39", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p39", eeprom_update_firmware);
    }
    if (should_include_param("p40", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p40", eeprom_dark_theme);
    }
    if (should_include_param("p41", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p41", eeprom_language);
    }
    if (should_include_param("p42", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p42", eeprom_pwm_frequency);
    }
    if (should_include_param("p43", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p43", eeprom_max_power);
    }

    // Add Home Assistant integration settings
    if (should_include_param("p25", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p25", eeprom_ha_url);
    }
    if (should_include_param("p26", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p26", eeprom_ha_token);
    }
    if (should_include_param("p27", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p27", eeprom_ha_refresh_mins);
    }

    // Add Stock Quote Service settings
    if (should_include_param("p28", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p28", eeprom_stock_key);
    }
    if (should_include_param("p29", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p29", eeprom_stock_refresh_mins);
    }

    // Add Dexcom settings
    if (should_include_param("p30", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p30", eeprom_dexcom_region);
    }
    if (should_include_param("p31", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p31", eeprom_glucose_username);
    }
    if (should_include_param("p32", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p32", eeprom_glucose_password);
    }
    if (should_include_param("p33", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p33", eeprom_glucose_refresh);
    }
    if (should_include_param("p45", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p45", glucose_validity_duration);
    }
    if (should_include_param("p48", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p48", eeprom_sec_time);
    }
    if (should_include_param("p49", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p49", eeprom_sec_cgm);
    }

    // Add Libre settings (region only - other fields are internal and set by API responses)
    if (should_include_param("p44", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p44", eeprom_libre_region);
    }

    if (should_include_param("p54", group_param, params_param))
    {
        cJSON_AddStringToObject(root, "p54", eeprom_ns_url);
    }

    // Add Glucose Thresholds and Unit
    if (should_include_param("p51", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p51", eeprom_glucose_high);
    }
    if (should_include_param("p52", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p52", eeprom_glucose_low);
    }
    if (should_include_param("p53", group_param, params_param))
    {
        cJSON_AddNumberToObject(root, "p53", eeprom_glucose_unit);
    }

    // Convert to string and send response
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str)
    {
        ESP_LOGE(TAG, "Failed to create JSON string");
        ret = ESP_FAIL;
        goto cleanup;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

cleanup:
    if (json_str)
        cJSON_free(json_str);
    if (root)
        cJSON_Delete(root);
    return ret;
}

// Helper function for hostname validation
static bool is_valid_hostname_char(char c)
{
    return isalnum((unsigned char)c) || c == '-';
}

// Valid font names (from UI select options)
static const char *const VALID_FONTS[] = {
    "bold", "light", "lcd", "nixie", "robrito", "ficasso", "lichten",
    "kablame", "kablamo", "kaboom", "kabboom", "user1", "user2"
};
static const int NUM_VALID_FONTS = sizeof(VALID_FONTS) / sizeof(VALID_FONTS[0]);

static bool is_valid_font_name(const char *name)
{
    if (!name || strlen(name) >= 12)
        return false;
    for (int i = 0; i < NUM_VALID_FONTS; i++)
    {
        if (strcmp(name, VALID_FONTS[i]) == 0)
            return true;
    }
    return false;
}

// Validate all JSON parameters against UI limits. Returns true if all valid.
// On failure, fills err_buf with error message (max err_size chars) and returns false.
static bool validate_json_params(cJSON *root, char *err_buf, size_t err_size)
{
    cJSON *item;
    const char *sval;
    size_t len;

#define CHECK_RANGE(param, val, min_val, max_val) do { \
    if ((val) < (min_val) || (val) > (max_val)) { \
        snprintf(err_buf, err_size, "Invalid %s: value %d out of range (%d-%d)", \
                 (param), (int)(val), (min_val), (max_val)); \
        return false; \
    } \
} while (0)

#define CHECK_DOUBLE_RANGE(param, val, min_val, max_val) do { \
    if ((val) < (min_val) || (val) > (max_val)) { \
        snprintf(err_buf, err_size, "Invalid %s: value %.1f out of range (%.1f-%.1f)", \
                 (param), (double)(val), (double)(min_val), (double)(max_val)); \
        return false; \
    } \
} while (0)

#define CHECK_STR_LEN(param, str, max_len) do { \
    if ((str) && (len = strlen(str)) > (max_len)) { \
        snprintf(err_buf, err_size, "Invalid %s: length %d exceeds max %d", \
                 (param), (int)len, (int)(max_len)); \
        return false; \
    } \
} while (0)

    /* p00 hostname */
    if ((item = cJSON_GetObjectItem(root, "p00")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        len = strlen(sval);
        if (len == 0 || len >= sizeof(eeprom_hostname))
        {
            snprintf(err_buf, err_size, "Invalid hostname: length must be 1-%d", (int)sizeof(eeprom_hostname) - 1);
            return false;
        }
        if (sval[0] == '-' || sval[len - 1] == '-')
        {
            snprintf(err_buf, err_size, "Invalid hostname: cannot start or end with hyphen");
            return false;
        }
        for (size_t i = 0; i < len; i++)
        {
            if (!is_valid_hostname_char(sval[i]))
            {
                snprintf(err_buf, err_size, "Invalid hostname: invalid character");
                return false;
            }
        }
    }

    /* p34 wifi_ssid */
    if ((item = cJSON_GetObjectItem(root, "p34")) && cJSON_IsString(item))
        CHECK_STR_LEN("wifi_ssid", item->valuestring, sizeof(eeprom_wifi_ssid) - 1);

    /* p35 wifi_pass */
    if ((item = cJSON_GetObjectItem(root, "p35")) && cJSON_IsString(item))
        CHECK_STR_LEN("wifi_pass", item->valuestring, sizeof(eeprom_wifi_pass) - 1);

    /* p36 fahrenheit */
    if ((item = cJSON_GetObjectItem(root, "p36")) && cJSON_IsNumber(item))
        CHECK_RANGE("fahrenheit", item->valueint, 0, 1);

    /* p37 hour12 */
    if ((item = cJSON_GetObjectItem(root, "p37")) && cJSON_IsNumber(item))
        CHECK_RANGE("hour12", item->valueint, 0, 1);

    /* p24 show_leading_zero */
    if ((item = cJSON_GetObjectItem(root, "p24")) && cJSON_IsNumber(item))
        CHECK_RANGE("show_leading_zero", item->valueint, 0, 1);

    /* p50 dots_breathe */
    if ((item = cJSON_GetObjectItem(root, "p50")) && cJSON_IsNumber(item))
        CHECK_RANGE("dots_breathe", item->valueint, 0, 1);

    /* p39 update_firmware */
    if ((item = cJSON_GetObjectItem(root, "p39")) && cJSON_IsNumber(item))
        CHECK_RANGE("update_firmware", item->valueint, 0, 1);

    /* p17 lat */
    if ((item = cJSON_GetObjectItem(root, "p17")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        len = strlen(sval);
        if (len == 0)
            ; /* empty is valid */
        else if (len >= sizeof(eeprom_lat))
        {
            snprintf(err_buf, err_size, "Invalid lat: length exceeds max %d", (int)sizeof(eeprom_lat) - 1);
            return false;
        }
        else
        {
            char tmp[12];
            strncpy(tmp, sval, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            if (!validate_coordinate(tmp, true))
            {
                snprintf(err_buf, err_size, "Invalid lat: invalid coordinate format");
                return false;
            }
        }
    }

    /* p18 lon */
    if ((item = cJSON_GetObjectItem(root, "p18")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        len = strlen(sval);
        if (len == 0)
            ;
        else if (len >= sizeof(eeprom_lon))
        {
            snprintf(err_buf, err_size, "Invalid lon: length exceeds max %d", (int)sizeof(eeprom_lon) - 1);
            return false;
        }
        else
        {
            char tmp[12];
            strncpy(tmp, sval, sizeof(tmp) - 1);
            tmp[sizeof(tmp) - 1] = '\0';
            if (!validate_coordinate(tmp, false))
            {
                snprintf(err_buf, err_size, "Invalid lon: invalid coordinate format");
                return false;
            }
        }
    }

    /* p19 timezone */
    if ((item = cJSON_GetObjectItem(root, "p19")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        len = strlen(sval);
        if (len >= TZ_LENGTH)
        {
            snprintf(err_buf, err_size, "Invalid timezone: length exceeds max %d", (int)TZ_LENGTH - 1);
            return false;
        }
        if (len > 0 && !validate_timezone(sval))
        {
            snprintf(err_buf, err_size, "Invalid timezone: use POSIX format (e.g. GMT-2 not GMT +2)");
            return false;
        }
    }

    /* p46 wifi_start, p47 wifi_end */
    if ((item = cJSON_GetObjectItem(root, "p46")) && cJSON_IsNumber(item))
        CHECK_RANGE("wifi_start", item->valueint, 0, 23);
    if ((item = cJSON_GetObjectItem(root, "p47")) && cJSON_IsNumber(item))
        CHECK_RANGE("wifi_end", item->valueint, 0, 23);

    /* p23 brightness_LED array - exactly 2 elements [day, night], each 1-100 */
    if ((item = cJSON_GetObjectItem(root, "p23")) && cJSON_IsArray(item))
    {
        int arr_size = cJSON_GetArraySize(item);
        if (arr_size > 2)
        {
            snprintf(err_buf, err_size, "Invalid brightness_LED: array size %d exceeds max 2", arr_size);
            return false;
        }
        for (int i = 0; i < arr_size; i++)
        {
            cJSON *elem = cJSON_GetArrayItem(item, i);
            if (cJSON_IsNumber(elem))
            {
                int val = (int)(elem->valuedouble + 0.5); /* works for both int and float JSON numbers */
                CHECK_RANGE("brightness_LED", val, 1, 100);
            }
        }
    }

    /* p20 lux_sensitivity */
    if ((item = cJSON_GetObjectItem(root, "p20")) && cJSON_IsNumber(item))
        CHECK_DOUBLE_RANGE("lux_sensitivity", item->valuedouble, 0, 50);

    /* p21 lux_threshold */
    if ((item = cJSON_GetObjectItem(root, "p21")) && cJSON_IsNumber(item))
        CHECK_DOUBLE_RANGE("lux_threshold", item->valuedouble, 0, 500);

    /* p04 dayfont, p05 nightfont */
    if ((item = cJSON_GetObjectItem(root, "p04")) && cJSON_IsString(item))
    {
        if (!is_valid_font_name(item->valuestring))
        {
            snprintf(err_buf, err_size, "Invalid dayfont: unknown font name");
            return false;
        }
    }
    if ((item = cJSON_GetObjectItem(root, "p05")) && cJSON_IsString(item))
    {
        if (!is_valid_font_name(item->valuestring))
        {
            snprintf(err_buf, err_size, "Invalid nightfont: unknown font name");
            return false;
        }
    }

    /* p22 dim_disable */
    if ((item = cJSON_GetObjectItem(root, "p22")) && cJSON_IsNumber(item))
        CHECK_RANGE("dim_disable", item->valueint, 0, 1);

    /* p16 message */
    if ((item = cJSON_GetObjectItem(root, "p16")) && cJSON_IsString(item))
        CHECK_STR_LEN("message", item->valuestring, SCROLL_MSG_LENGTH - 1);

    /* p06 quiet_scroll, p07 quiet_weather */
    if ((item = cJSON_GetObjectItem(root, "p06")) && cJSON_IsNumber(item))
        CHECK_RANGE("quiet_scroll", item->valueint, 0, 1);
    if ((item = cJSON_GetObjectItem(root, "p07")) && cJSON_IsNumber(item))
        CHECK_RANGE("quiet_weather", item->valueint, 0, 1);

    /* p10 color_filter, p11 night_color_filter */
    if ((item = cJSON_GetObjectItem(root, "p10")) && cJSON_IsNumber(item))
        CHECK_RANGE("color_filter", item->valueint, 0, 4);
    if ((item = cJSON_GetObjectItem(root, "p11")) && cJSON_IsNumber(item))
        CHECK_RANGE("night_color_filter", item->valueint, 0, 4);

    /* p12 msg_color, p15 night_msg_color - hex #RRGGBB */
    if ((item = cJSON_GetObjectItem(root, "p12")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        if (strlen(sval) != 0 && (strlen(sval) != 7 || sval[0] != '#' ||
            !isxdigit((unsigned char)sval[1]) || !isxdigit((unsigned char)sval[2]) ||
            !isxdigit((unsigned char)sval[3]) || !isxdigit((unsigned char)sval[4]) ||
            !isxdigit((unsigned char)sval[5]) || !isxdigit((unsigned char)sval[6])))
        {
            snprintf(err_buf, err_size, "Invalid msg_color: must be #RRGGBB hex format");
            return false;
        }
    }
    if ((item = cJSON_GetObjectItem(root, "p15")) && cJSON_IsString(item))
    {
        sval = item->valuestring;
        if (strlen(sval) != 0 && (strlen(sval) != 7 || sval[0] != '#' ||
            !isxdigit((unsigned char)sval[1]) || !isxdigit((unsigned char)sval[2]) ||
            !isxdigit((unsigned char)sval[3]) || !isxdigit((unsigned char)sval[4]) ||
            !isxdigit((unsigned char)sval[5]) || !isxdigit((unsigned char)sval[6])))
        {
            snprintf(err_buf, err_size, "Invalid night_msg_color: must be #RRGGBB hex format");
            return false;
        }
    }

    /* p01 ofs_x, p02 ofs_y */
    if ((item = cJSON_GetObjectItem(root, "p01")) && cJSON_IsNumber(item))
        CHECK_RANGE("ofs_x", item->valueint, 0, 160);
    if ((item = cJSON_GetObjectItem(root, "p02")) && cJSON_IsNumber(item))
        CHECK_RANGE("ofs_y", item->valueint, 0, 160);

    /* p03 rotation */
    if ((item = cJSON_GetObjectItem(root, "p03")) && cJSON_IsNumber(item))
        CHECK_RANGE("rotation", item->valueint, 0, 3);

    /* p09 mirroring, p08 show_grid */
    if ((item = cJSON_GetObjectItem(root, "p09")) && cJSON_IsNumber(item))
        CHECK_RANGE("mirroring", item->valueint, 0, 1);
    if ((item = cJSON_GetObjectItem(root, "p08")) && cJSON_IsNumber(item))
        CHECK_RANGE("show_grid", item->valueint, 0, 1);

    /* p38 scroll_speed */
    if ((item = cJSON_GetObjectItem(root, "p38")) && cJSON_IsNumber(item))
        CHECK_RANGE("scroll_speed", item->valueint, 1, 255);

    /* p14 scroll_delay (stored as uint8_t, max 255) */
    if ((item = cJSON_GetObjectItem(root, "p14")) && cJSON_IsNumber(item))
        CHECK_RANGE("scroll_delay", item->valueint, 30, 255);

    /* p40 dark_theme, p41 language */
    if ((item = cJSON_GetObjectItem(root, "p40")) && cJSON_IsNumber(item))
        CHECK_RANGE("dark_theme", item->valueint, 0, 1);
    if ((item = cJSON_GetObjectItem(root, "p41")) && cJSON_IsNumber(item))
        CHECK_RANGE("language", item->valueint, 0, 8);

    /* p42 pwm_frequency */
    if ((item = cJSON_GetObjectItem(root, "p42")) && cJSON_IsNumber(item))
        CHECK_RANGE("pwm_frequency", item->valueint, 10, 1000000);

    /* p43 max_power */
    if ((item = cJSON_GetObjectItem(root, "p43")) && cJSON_IsNumber(item))
        CHECK_RANGE("max_power", item->valueint, 1, 1023);

    /* p13 msg_font */
    if ((item = cJSON_GetObjectItem(root, "p13")) && cJSON_IsNumber(item))
        CHECK_RANGE("msg_font", item->valueint, 0, 2);

    /* p25 ha_url, p26 ha_token */
    if ((item = cJSON_GetObjectItem(root, "p25")) && cJSON_IsString(item))
        CHECK_STR_LEN("ha_url", item->valuestring, sizeof(eeprom_ha_url) - 1);
    if ((item = cJSON_GetObjectItem(root, "p26")) && cJSON_IsString(item))
        CHECK_STR_LEN("ha_token", item->valuestring, sizeof(eeprom_ha_token) - 1);

    /* p27 ha_refresh_mins */
    if ((item = cJSON_GetObjectItem(root, "p27")) && cJSON_IsNumber(item))
        CHECK_RANGE("ha_refresh_mins", item->valueint, 1, 7200);

    /* p28 stock_key */
    if ((item = cJSON_GetObjectItem(root, "p28")) && cJSON_IsString(item))
        CHECK_STR_LEN("stock_key", item->valuestring, sizeof(eeprom_stock_key) - 1);

    /* p29 stock_refresh_mins */
    if ((item = cJSON_GetObjectItem(root, "p29")) && cJSON_IsNumber(item))
        CHECK_RANGE("stock_refresh_mins", item->valueint, 1, 1440);

    /* p30 dexcom_region */
    if ((item = cJSON_GetObjectItem(root, "p30")) && cJSON_IsNumber(item))
        CHECK_RANGE("dexcom_region", item->valueint, 0, 3);

    /* p31 glucose_username, p32 glucose_password */
    if ((item = cJSON_GetObjectItem(root, "p31")) && cJSON_IsString(item))
        CHECK_STR_LEN("glucose_username", item->valuestring, sizeof(eeprom_glucose_username) - 1);
    if ((item = cJSON_GetObjectItem(root, "p32")) && cJSON_IsString(item))
        CHECK_STR_LEN("glucose_password", item->valuestring, sizeof(eeprom_glucose_password) - 1);

    /* p33 glucose_refresh */
    if ((item = cJSON_GetObjectItem(root, "p33")) && cJSON_IsNumber(item))
        CHECK_RANGE("glucose_refresh", item->valueint, 1, 60);

    /* p45 glucose_validity_duration */
    if ((item = cJSON_GetObjectItem(root, "p45")) && cJSON_IsNumber(item))
        CHECK_RANGE("glucose_validity_duration", item->valueint, 10, 360);

    /* p48 sec_time, p49 sec_cgm */
    if ((item = cJSON_GetObjectItem(root, "p48")) && cJSON_IsNumber(item))
        CHECK_RANGE("sec_time", item->valueint, 0, 120);
    if ((item = cJSON_GetObjectItem(root, "p49")) && cJSON_IsNumber(item))
        CHECK_RANGE("sec_cgm", item->valueint, 0, 120);

    /* p44 libre_region */
    if ((item = cJSON_GetObjectItem(root, "p44")) && cJSON_IsNumber(item))
        CHECK_RANGE("libre_region", item->valueint, 0, 7);

    /* p54 ns_url */
    if ((item = cJSON_GetObjectItem(root, "p54")) && cJSON_IsString(item))
        CHECK_STR_LEN("ns_url", item->valuestring, 100);

    /* p51 glucose_high, p52 glucose_low */
    if ((item = cJSON_GetObjectItem(root, "p51")) && cJSON_IsNumber(item))
        CHECK_RANGE("glucose_high", item->valueint, 1, 400);
    if ((item = cJSON_GetObjectItem(root, "p52")) && cJSON_IsNumber(item))
        CHECK_RANGE("glucose_low", item->valueint, 1, 400);

    /* p53 glucose_unit */
    if ((item = cJSON_GetObjectItem(root, "p53")) && cJSON_IsNumber(item))
        CHECK_RANGE("glucose_unit", item->valueint, 0, 1);

    return true;
#undef CHECK_RANGE
#undef CHECK_DOUBLE_RANGE
#undef CHECK_STR_LEN
}

// Handler for settings form submission

esp_err_t settings_post_handler(httpd_req_t *req)
{
    // Track connection
    if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if (server_state.is_restarting)
        {
            xSemaphoreGive(server_state.mutex);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server is restarting");
            return ESP_FAIL;
        }
        server_state.active_connections++;
        xSemaphoreGive(server_state.mutex);
    }

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Settings POST request received, content_len=%d", req->content_len);
    int ret, remaining = req->content_len;

    // Get a shared buffer for HTTP data
    char *http_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "settings_post");
    if (!http_buffer)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to get shared buffer");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server memory error");
        goto cleanup;
    }

    if (remaining > HTTP_BUFFER_SIZE - 1)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "POST data too large : %d", remaining);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too large");
        goto cleanup;
    }

    // Read POST data
    int received = 0;
    while (remaining > 0)
    {
        ret = httpd_req_recv(req, http_buffer + received, MIN(remaining, HTTP_BUFFER_SIZE - received - 1));
        if (ret <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                continue;
            }
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to receive data: %d", ret);
            goto cleanup;
        }
        received += ret;
        remaining -= ret;
    }
    http_buffer[received] = '\0';

    // Parse JSON data
    cJSON *root = cJSON_Parse(http_buffer);
    if (!root)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to parse JSON");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Failed to parse JSON");
        goto cleanup;
    }

    // Validate all parameters against UI limits before applying any changes
    {
        char validation_err[128];
        if (!validate_json_params(root, validation_err, sizeof(validation_err)))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "JSON validation failed: %s", validation_err);
            httpd_resp_set_status(req, "400 Bad Request");
            httpd_resp_set_type(req, "application/json");
            char err_json[192];
            snprintf(err_json, sizeof(err_json), "{\"status\":\"error\",\"message\":\"%s\"}", validation_err);
            httpd_resp_send(req, err_json, HTTPD_RESP_USE_STRLEN);
            cJSON_Delete(root);
            goto cleanup;
        }
    }

    // Store original values of critical settings to check if they changed
    char orig_hostname[33];
    char orig_wifi_ssid[33];
    char orig_wifi_pass[64];
    char orig_lat[12];
    char orig_lon[12];
    char orig_timezone[TZ_LENGTH];
    char orig_message[SCROLL_MSG_LENGTH]; // Add this line

    // Copy original values
    strncpy(orig_hostname, eeprom_hostname, sizeof(orig_hostname));
    strncpy(orig_wifi_ssid, eeprom_wifi_ssid, sizeof(orig_wifi_ssid));
    strncpy(orig_wifi_pass, eeprom_wifi_pass, sizeof(orig_wifi_pass));
    strncpy(orig_lat, eeprom_lat, sizeof(orig_lat));
    strncpy(orig_lon, eeprom_lon, sizeof(orig_lon));
    strncpy(orig_timezone, eeprom_timezone, sizeof(orig_timezone));
    strncpy(orig_message, eeprom_message, sizeof(orig_message)); // Add this line

    // Process network settings
    cJSON *hostname_json = cJSON_GetObjectItem(root, "p00"); // hostname
    if (cJSON_IsString(hostname_json))
    {
        const char *new_hostname = hostname_json->valuestring;
        size_t len = strlen(new_hostname);
        bool valid = true;

        if (len == 0 || len >= sizeof(eeprom_hostname))
        { // Check length (incl. null term)
            valid = false;
            ESP_LOGW(TAG, "Invalid hostname length: %d", len);
        }
        else if (new_hostname[0] == '-' || new_hostname[len - 1] == '-')
        {
            valid = false;
            ESP_LOGW(TAG, "Hostname cannot start or end with a hyphen: %s", new_hostname);
        }
        else
        {
            for (size_t i = 0; i < len; i++)
            {
                if (!is_valid_hostname_char(new_hostname[i]))
                {
                    valid = false;
                    ESP_LOGW(TAG, "Invalid character '%c' in hostname: %s", new_hostname[i], new_hostname);
                    break;
                }
            }
        }

        if (valid)
        {
            strncpy(eeprom_hostname, new_hostname, sizeof(eeprom_hostname) - 1);
            eeprom_hostname[sizeof(eeprom_hostname) - 1] = '\0'; // Ensure null termination
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Accepted new hostname: %s", eeprom_hostname);
        }
        else
        {
            ESP_LOGW(TAG, "Received invalid hostname '%s'. Keeping current hostname '%s'.", new_hostname, eeprom_hostname);
            // Optional: Send an error back? For now, just log and ignore.
        }
    }

    cJSON *wifi_ssid = cJSON_GetObjectItem(root, "p34");
    if (cJSON_IsString(wifi_ssid))
    {
        strncpy(eeprom_wifi_ssid, wifi_ssid->valuestring, sizeof(eeprom_wifi_ssid) - 1);
    }

    cJSON *wifi_pass = cJSON_GetObjectItem(root, "p35");
    if (cJSON_IsString(wifi_pass))
    {
        strncpy(eeprom_wifi_pass, wifi_pass->valuestring, sizeof(eeprom_wifi_pass) - 1);
    }

    // Process display format settings
    cJSON *fahrenheit = cJSON_GetObjectItem(root, "p36");
    if (cJSON_IsNumber(fahrenheit))
    {
        eeprom_fahrenheit = (uint8_t)fahrenheit->valueint;
    }

    cJSON *hour12 = cJSON_GetObjectItem(root, "p37");
    if (cJSON_IsNumber(hour12))
    {
        eeprom_12hour = (uint8_t)hour12->valueint;
    }

    cJSON *show_leading_zero = cJSON_GetObjectItem(root, "p24");
    if (cJSON_IsNumber(show_leading_zero))
    {
        eeprom_show_leading_zero = (uint8_t)show_leading_zero->valueint;
    }

    cJSON *dots_breathe = cJSON_GetObjectItem(root, "p50");
    if (cJSON_IsNumber(dots_breathe))
    {
        int breathe_value = dots_breathe->valueint;
        if (breathe_value >= 0 && breathe_value <= 1)
        {
            eeprom_dots_breathe = (uint8_t)breathe_value;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid dots_breathe value: %d, must be 0 or 1", breathe_value);
        }
    }

    // Process auto firmware update setting
    cJSON *update_firmware = cJSON_GetObjectItem(root, "p39");
    if (cJSON_IsNumber(update_firmware))
    {
        eeprom_update_firmware = (uint8_t)update_firmware->valueint;
    }

    // Process location settings
    cJSON *lat = cJSON_GetObjectItem(root, "p17");
    if (cJSON_IsString(lat))
    {
        strncpy(eeprom_lat, lat->valuestring, sizeof(eeprom_lat) - 1);
        eeprom_lat[sizeof(eeprom_lat) - 1] = '\0'; // Ensure null termination
        // Validate latitude before accepting
        if (!validate_coordinate(eeprom_lat, true))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid latitude received: '%s', rejecting", eeprom_lat);
            strcpy(eeprom_lat, ""); // Clear invalid value
        }
    }

    cJSON *lon = cJSON_GetObjectItem(root, "p18");
    if (cJSON_IsString(lon))
    {
        strncpy(eeprom_lon, lon->valuestring, sizeof(eeprom_lon) - 1);
        eeprom_lon[sizeof(eeprom_lon) - 1] = '\0'; // Ensure null termination
        // Validate longitude before accepting
        if (!validate_coordinate(eeprom_lon, false))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid longitude received: '%s', rejecting", eeprom_lon);
            strcpy(eeprom_lon, ""); // Clear invalid value
        }
    }

    cJSON *timezone = cJSON_GetObjectItem(root, "p19");
    if (cJSON_IsString(timezone))
    {
        strncpy(eeprom_timezone, timezone->valuestring, sizeof(eeprom_timezone) - 1);
        eeprom_timezone[sizeof(eeprom_timezone) - 1] = '\0';
        if (!validate_timezone(eeprom_timezone))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone received: '%s', rejecting (use POSIX format e.g. GMT-2 not GMT +2)", eeprom_timezone);
            strcpy(eeprom_timezone, "");
        }
    }

    // Process WiFi Active Hours settings
    cJSON *wifi_start = cJSON_GetObjectItem(root, "p46");
    if (cJSON_IsNumber(wifi_start))
    {
        int start_value = wifi_start->valueint;
        if (start_value >= 0 && start_value <= 23)
        {
            eeprom_wifi_start = (uint8_t)start_value;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid wifi_start value: %d, must be 0-23", start_value);
        }
    }

    cJSON *wifi_end = cJSON_GetObjectItem(root, "p47");
    if (cJSON_IsNumber(wifi_end))
    {
        int end_value = wifi_end->valueint;
        if (end_value >= 0 && end_value <= 23)
        {
            eeprom_wifi_end = (uint8_t)end_value;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid wifi_end value: %d, must be 0-23", end_value);
        }
    }

    // Process LED brightness settings (eeprom_brightness_LED has exactly 2 elements)
    cJSON *brightness_array = cJSON_GetObjectItem(root, "p23");
    if (cJSON_IsArray(brightness_array))
    {
        for (int i = 0; i < cJSON_GetArraySize(brightness_array) && i < 2; i++)
        {
            cJSON *brightness_item = cJSON_GetArrayItem(brightness_array, i);
            if (cJSON_IsNumber(brightness_item))
            {
                int val = (int)(brightness_item->valuedouble + 0.5);
                eeprom_brightness_LED[i] = (uint8_t)val;
            }
        }
    }

    // Process light sensor settings
    cJSON *lux_sensitivity = cJSON_GetObjectItem(root, "p20");
    if (cJSON_IsNumber(lux_sensitivity))
    {
        eeprom_lux_sensitivity = (float)lux_sensitivity->valuedouble;
    }

    cJSON *lux_threshold = cJSON_GetObjectItem(root, "p21");
    if (cJSON_IsNumber(lux_threshold))
    {
        eeprom_lux_threshold = (float)lux_threshold->valuedouble;
    }

    // Process font settings
    cJSON *dayfont = cJSON_GetObjectItem(root, "p04");
    if (cJSON_IsString(dayfont))
    {
        strncpy(eeprom_font[0], dayfont->valuestring, sizeof(eeprom_font[0]) - 1);
    }

    cJSON *nightfont = cJSON_GetObjectItem(root, "p05");
    if (cJSON_IsString(nightfont))
    {
        strncpy(eeprom_font[1], nightfont->valuestring, sizeof(eeprom_font[1]) - 1);
    }

    // Process display settings
    cJSON *dim_disable = cJSON_GetObjectItem(root, "p22");
    if (cJSON_IsNumber(dim_disable))
    {
        eeprom_dim_disable = (uint8_t)dim_disable->valueint;
    }

    cJSON *message = cJSON_GetObjectItem(root, "p16");
    if (cJSON_IsString(message))
    {
        strncpy(eeprom_message, message->valuestring, sizeof(eeprom_message) - 1);
        eeprom_message[sizeof(eeprom_message) - 1] = '\0'; // Ensure null termination
    }

    cJSON *quiet_scroll = cJSON_GetObjectItem(root, "p06");
    if (cJSON_IsNumber(quiet_scroll))
    {
        eeprom_quiet_scroll = (uint8_t)quiet_scroll->valueint;
    }

    cJSON *quiet_weather = cJSON_GetObjectItem(root, "p07");
    if (cJSON_IsNumber(quiet_weather))
    {
        eeprom_quiet_weather = (uint8_t)quiet_weather->valueint;
    }

    // Process color filter setting
    cJSON *color_filter = cJSON_GetObjectItem(root, "p10");
    if (cJSON_IsNumber(color_filter))
    {
        eeprom_color_filter[0] = (uint8_t)color_filter->valueint;
    }

    // Process message color setting
    cJSON *msg_color = cJSON_GetObjectItem(root, "p12");
    if (cJSON_IsString(msg_color))
    {
        const char *color_str = msg_color->valuestring;
        if (strlen(color_str) == 7 && color_str[0] == '#')
        {
            // Convert hex color to RGB
            char hex[3] = {0};
            hex[0] = color_str[1];
            hex[1] = color_str[2];
            eeprom_msg_red[0] = (uint8_t)strtol(hex, NULL, 16);
            hex[0] = color_str[3];
            hex[1] = color_str[4];
            eeprom_msg_green[0] = (uint8_t)strtol(hex, NULL, 16);
            hex[0] = color_str[5];
            hex[1] = color_str[6];
            eeprom_msg_blue[0] = (uint8_t)strtol(hex, NULL, 16);
        }
    }

    // Process display position settings
    cJSON *ofs_x = cJSON_GetObjectItem(root, "p01");
    if (cJSON_IsNumber(ofs_x))
    {
        eeprom_ofs_x = (uint8_t)ofs_x->valueint;
    }

    cJSON *ofs_y = cJSON_GetObjectItem(root, "p02");
    if (cJSON_IsNumber(ofs_y))
    {
        eeprom_ofs_y = (uint8_t)ofs_y->valueint;
    }

    cJSON *rotation = cJSON_GetObjectItem(root, "p03");
    if (cJSON_IsNumber(rotation))
    {
        eeprom_rotation = (uint8_t)rotation->valueint;
    }

    cJSON *mirroring = cJSON_GetObjectItem(root, "p09");
    if (cJSON_IsNumber(mirroring))
    {
        eeprom_mirroring = (uint8_t)mirroring->valueint;
    }

    cJSON *show_grid = cJSON_GetObjectItem(root, "p08");
    if (cJSON_IsNumber(show_grid))
    {
        eeprom_show_grid = (uint8_t)show_grid->valueint;
    }

    cJSON *scroll_speed = cJSON_GetObjectItem(root, "p38");
    if (cJSON_IsNumber(scroll_speed))
    {
        eeprom_scroll_speed = (uint8_t)scroll_speed->valueint;
    }

    cJSON *scroll_delay = cJSON_GetObjectItem(root, "p14");
    if (cJSON_IsNumber(scroll_delay))
    {
        int delay_value = (int)scroll_delay->valueint;
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Received scroll_delay: %d", delay_value);
        // Apply bounds checking: minimum 30, maximum 255 (uint8_t storage)
        if (delay_value < 30)
            delay_value = 30;
        if (delay_value > 255)
            delay_value = 255;
        eeprom_scroll_delay = (uint8_t)delay_value;
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Set eeprom_scroll_delay to: %u", eeprom_scroll_delay);
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "scroll_delay not found in POST data or not a number");
    }

    cJSON *dark_theme = cJSON_GetObjectItem(root, "p40");
    if (cJSON_IsNumber(dark_theme))
    {
        eeprom_dark_theme = (uint8_t)dark_theme->valueint;
    }

    cJSON *language = cJSON_GetObjectItem(root, "p41");
    if (cJSON_IsNumber(language))
    {
        eeprom_language = (uint8_t)language->valueint;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Language set to index: %d", eeprom_language);
    }

    // Process PWM frequency
    bool pwm_frequency_changed = false;
    cJSON *pwm_frequency = cJSON_GetObjectItem(root, "p42");
    if (cJSON_IsNumber(pwm_frequency))
    {
        int freq_value = (int)pwm_frequency->valueint;
        // Validate range 10-1000000
        if (freq_value >= 10 && freq_value <= 1000000)
        {
            eeprom_pwm_frequency = (uint32_t)freq_value;
            pwm_frequency_changed = true;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "PWM frequency set to: %lu Hz", (unsigned long)eeprom_pwm_frequency);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "PWM frequency out of range (10-1000000): %d, keeping current value", freq_value);
        }
    }

    // Process max power
    cJSON *max_power = cJSON_GetObjectItem(root, "p43");
    if (cJSON_IsNumber(max_power))
    {
        int power_value = (int)max_power->valueint;
        // Validate range 1-1023
        if (power_value >= 1 && power_value <= 1023)
        {
            eeprom_max_power = (uint16_t)power_value;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Max power set to: %u", eeprom_max_power);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Max power out of range (1-1023): %d, keeping current value", power_value);
        }
    }

    // Process night color filter
    cJSON *night_color_filter = cJSON_GetObjectItem(root, "p11");
    if (cJSON_IsNumber(night_color_filter))
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Received night_color_filter: %d, setting eeprom_color_filter[1] to %u",
                    night_color_filter->valueint, (uint8_t)night_color_filter->valueint);
        eeprom_color_filter[1] = (uint8_t)night_color_filter->valueint;
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "night_color_filter not found in POST data or not a number");
    }

    // Process night message color
    cJSON *night_msg_color = cJSON_GetObjectItem(root, "p15");
    if (cJSON_IsString(night_msg_color))
    {
        const char *color_str = night_msg_color->valuestring;
        if (strlen(color_str) == 7 && color_str[0] == '#')
        {
            char hex[3] = {0};
            hex[0] = color_str[1];
            hex[1] = color_str[2];
            eeprom_msg_red[1] = (uint8_t)strtol(hex, NULL, 16);
            hex[0] = color_str[3];
            hex[1] = color_str[4];
            eeprom_msg_green[1] = (uint8_t)strtol(hex, NULL, 16);
            hex[0] = color_str[5];
            hex[1] = color_str[6];
            eeprom_msg_blue[1] = (uint8_t)strtol(hex, NULL, 16);
        }
    }

    // Process message font size
    cJSON *msg_font = cJSON_GetObjectItem(root, "p13");
    if (cJSON_IsNumber(msg_font))
    {
        int font_value = (int)msg_font->valueint;
        if (font_value >= 0 && font_value <= 4)
        {
            eeprom_msg_font = (uint8_t)font_value;
        }
    }

    // Track if any integration-related settings changed (needed to re-parse integrations)
    // These parameters are only sent when changed, so their presence indicates a change
    bool integration_settings_changed = false;

    // Process Home Assistant integration settings
    cJSON *ha_url = cJSON_GetObjectItem(root, "p25");
    if (cJSON_IsString(ha_url))
    {
        integration_settings_changed = true;
        strncpy(eeprom_ha_url, ha_url->valuestring, sizeof(eeprom_ha_url) - 1);
        eeprom_ha_url[sizeof(eeprom_ha_url) - 1] = '\0'; // Ensure null termination

        // Remove trailing slashes
        size_t len = strlen(eeprom_ha_url);
        while (len > 0 && eeprom_ha_url[len - 1] == '/')
        {
            eeprom_ha_url[len - 1] = '\0';
            len--;
        }
    }

    cJSON *ha_token = cJSON_GetObjectItem(root, "p26");
    if (cJSON_IsString(ha_token))
    {
        integration_settings_changed = true;
        strncpy(eeprom_ha_token, ha_token->valuestring, sizeof(eeprom_ha_token) - 1);
        eeprom_ha_token[sizeof(eeprom_ha_token) - 1] = '\0'; // Ensure null termination
    }

    cJSON *ha_refresh_mins = cJSON_GetObjectItem(root, "p27");
    if (cJSON_IsNumber(ha_refresh_mins))
    {
        eeprom_ha_refresh_mins = (uint16_t)ha_refresh_mins->valueint;
    }

    // Add Stock Quote Service settings
    cJSON *stock_key = cJSON_GetObjectItem(root, "p28");
    if (cJSON_IsString(stock_key))
    {
        integration_settings_changed = true;
        strncpy(eeprom_stock_key, stock_key->valuestring, sizeof(eeprom_stock_key) - 1);
        eeprom_stock_key[sizeof(eeprom_stock_key) - 1] = '\0'; // Ensure null termination
    }

    cJSON *stock_refresh_mins = cJSON_GetObjectItem(root, "p29");
    if (cJSON_IsNumber(stock_refresh_mins))
    {
        eeprom_stock_refresh_mins = (uint16_t)stock_refresh_mins->valueint;
    }

    // Dexcom settings
    if (cJSON_HasObjectItem(root, "p30"))
    {
        integration_settings_changed = true;
        int dexcom_region = cJSON_GetObjectItem(root, "p30")->valueint;
        if (dexcom_region >= 0 && dexcom_region <= 3)
        {
            eeprom_dexcom_region = (uint8_t)dexcom_region;
        }
    }
    // Shared glucose monitoring settings (p31, p32, p33)
    if (cJSON_HasObjectItem(root, "p31"))
    {
        integration_settings_changed = true;
        const char *glucose_username = cJSON_GetObjectItem(root, "p31")->valuestring;
        if (glucose_username && strlen(glucose_username) <= 64)
        {
            strncpy(eeprom_glucose_username, glucose_username, sizeof(eeprom_glucose_username) - 1);
            eeprom_glucose_username[sizeof(eeprom_glucose_username) - 1] = '\0';
        }
    }
    if (cJSON_HasObjectItem(root, "p32"))
    {
        integration_settings_changed = true;
        const char *glucose_password = cJSON_GetObjectItem(root, "p32")->valuestring;
        if (glucose_password && strlen(glucose_password) <= 64)
        {
            strncpy(eeprom_glucose_password, glucose_password, sizeof(eeprom_glucose_password) - 1);
            eeprom_glucose_password[sizeof(eeprom_glucose_password) - 1] = '\0';
        }
    }
    if (cJSON_HasObjectItem(root, "p33"))
    {
        int glucose_refresh = cJSON_GetObjectItem(root, "p33")->valueint;
        if (glucose_refresh >= 1 && glucose_refresh <= 60)
        {
            eeprom_glucose_refresh = (uint8_t)glucose_refresh;
        }
    }
    if (cJSON_HasObjectItem(root, "p45"))
    {
        int validity_duration = cJSON_GetObjectItem(root, "p45")->valueint;
        if (validity_duration >= 10 && validity_duration <= 360)
        {
            glucose_validity_duration = (uint16_t)validity_duration;
        }
    }
    if (cJSON_HasObjectItem(root, "p48"))
    {
        int sec_time = cJSON_GetObjectItem(root, "p48")->valueint;
        if (sec_time >= 0 && sec_time <= 120)
        {
            eeprom_sec_time = (uint8_t)sec_time;
        }
    }
    if (cJSON_HasObjectItem(root, "p49"))
    {
        int sec_cgm = cJSON_GetObjectItem(root, "p49")->valueint;
        if (sec_cgm >= 0 && sec_cgm <= 120)
        {
            eeprom_sec_cgm = (uint8_t)sec_cgm;
        }
    }

    // Libre settings (region only - credentials use shared glucose variables via p31, p32, p33)
    // Other Libre fields (patient_id, token, account_id, region_url) are internal and set by API responses, not via web interface
    if (cJSON_HasObjectItem(root, "p44"))
    {
        integration_settings_changed = true;
        int libre_region = cJSON_GetObjectItem(root, "p44")->valueint;
        if (libre_region >= 0 && libre_region <= 7)
        {
            eeprom_libre_region = (uint8_t)libre_region;
        }
    }

    if (cJSON_HasObjectItem(root, "p54"))
    {
        integration_settings_changed = true;
        cJSON *p54 = cJSON_GetObjectItem(root, "p54");
        const char *ns_url = cJSON_IsString(p54) ? p54->valuestring : NULL;
        if (ns_url && strlen(ns_url) <= 100)
        {
            strncpy(eeprom_ns_url, ns_url, sizeof(eeprom_ns_url) - 1);
            eeprom_ns_url[sizeof(eeprom_ns_url) - 1] = '\0';
        }
        else
        {
            eeprom_ns_url[0] = '\0';
        }
    }

    // Process glucose threshold settings
    if (cJSON_HasObjectItem(root, "p51"))
    {
        int glucose_high = cJSON_GetObjectItem(root, "p51")->valueint;
        if (glucose_high > 0 && glucose_high <= 400)
        {
            eeprom_glucose_high = (uint16_t)glucose_high;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid glucose_high value: %d, must be 1-400", glucose_high);
        }
    }

    if (cJSON_HasObjectItem(root, "p52"))
    {
        int glucose_low = cJSON_GetObjectItem(root, "p52")->valueint;
        if (glucose_low > 0 && glucose_low <= 400)
        {
            eeprom_glucose_low = (uint16_t)glucose_low;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid glucose_low value: %d, must be 1-400", glucose_low);
        }
    }

    // Process glucose unit setting
    if (cJSON_HasObjectItem(root, "p53"))
    {
        int unit_value = cJSON_GetObjectItem(root, "p53")->valueint;
        if (unit_value >= 0 && unit_value <= 1)
        {
            eeprom_glucose_unit = (uint8_t)unit_value;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid glucose_unit value: %d, must be 0 or 1", unit_value);
        }
    }

    cJSON_Delete(root);

    // Save settings to NVS
    esp_err_t err = write_nvs_parameters();
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Error saving settings to NVS");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save settings");
        goto cleanup;
    }

    // Reconfigure PWM frequency if it was changed
    if (pwm_frequency_changed)
    {
        reconfigure_led_pwm_frequency();
    }
    set_led_pwm_brightness(eeprom_brightness_LED[font_index]); // set the brightness again, in case something changed

    // Check if critical settings have changed
    bool critical_settings_changed = false;

    // Check if network settings changed
    if (strcmp(orig_wifi_ssid, eeprom_wifi_ssid) != 0 ||
        strcmp(orig_wifi_pass, eeprom_wifi_pass) != 0 ||
        strcmp(orig_hostname, eeprom_hostname) != 0)
    {
        critical_settings_changed = true;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Network settings changed, device will restart");
    }

    // Check if location settings changed
    if (strcmp(orig_lat, eeprom_lat) != 0 ||
        strcmp(orig_lon, eeprom_lon) != 0 ||
        strcmp(orig_timezone, eeprom_timezone) != 0)
    {
        critical_settings_changed = true;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Location or timezone settings changed, device will restart");
    }

    // Check if message has changed and call parse_integrations if it has
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Orig message: %s, new message: %s", orig_message, eeprom_message);
    if ((strcmp(orig_message, eeprom_message) != 0) || integration_settings_changed)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Message or integration settings changed, parsing integrations");
        parse_integrations();
    }

    // Only restart if critical settings changed, otherwise just set the flag
    if (critical_settings_changed)
    {
        // Send success response with restart message
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"status\":\"ok\",\"message\":\"Settings saved, rebooting...\"}", HTTPD_RESP_USE_STRLEN);
        taskYIELD();

        // Create a task to reset after a brief delay to allow response to be sent
        xTaskCreate(&restart_device, "restart_task", 2048, NULL, 5, NULL);
    }
    else
    {
        // Set the flag for non-critical settings update
        settings_updated = true;
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Non-critical settings updated, no restart needed");

        // Send success response without restart message
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"status\":\"ok\",\"message\":\"Settings saved\"}", HTTPD_RESP_USE_STRLEN);
    }

    // Clean up and return
cleanup:
    // Release the shared buffer if we got one
    if (http_buffer)
    {
        release_shared_buffer(http_buffer);
    }

    // Decrement active connections
    if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if (server_state.active_connections > 0)
        {
            server_state.active_connections--;
        }
        xSemaphoreGive(server_state.mutex);
    }
    return ESP_OK;
}

// Add these constants at the top with other defines
#define HTTP_CLIENT_TIMEOUT_MS 10000    // 10 second timeout
#define HTTP_CLIENT_RETRY_COUNT 3       // Number of retries for failed requests
#define HTTP_CLIENT_RETRY_DELAY_MS 1000 // Delay between retries

// Add this helper function for HTTP client configuration
static esp_err_t configure_http_client(httpd_req_t *req)
{
    // Set socket options for better connection handling
    int sockfd = httpd_req_to_sockfd(req);
    if (sockfd < 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to get socket fd");
        return ESP_FAIL;
    }

    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = HTTP_CLIENT_TIMEOUT_MS / 1000;
    tv.tv_usec = (HTTP_CLIENT_TIMEOUT_MS % 1000) * 1000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set socket receive timeout");
        return ESP_FAIL;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set socket send timeout");
        return ESP_FAIL;
    }

    // Enable TCP keepalive
    int keepAlive = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive)) < 0)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to set socket keepalive");
    }

    return ESP_OK;
}

// Update the status_api_handler to include HA memory stats
esp_err_t status_api_handler(httpd_req_t *req)
{
    // Configure HTTP client first
    esp_err_t ret = configure_http_client(req);
    if (ret != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to configure connection");
        return ESP_FAIL;
    }

    // Check memory availability first
    if (!check_memory_available())
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server memory low");
        return ESP_FAIL;
    }

    // Track connection with retry logic
    int retry_count = 0;
    while (retry_count < HTTP_CLIENT_RETRY_COUNT)
    {
        if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            if (server_state.is_restarting)
            {
                xSemaphoreGive(server_state.mutex);
                if (retry_count < HTTP_CLIENT_RETRY_COUNT - 1)
                {
                    retry_count++;
                    vTaskDelay(pdMS_TO_TICKS(HTTP_CLIENT_RETRY_DELAY_MS));
                    continue;
                }
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Server is restarting");
                return ESP_FAIL;
            }
            server_state.active_connections++;
            xSemaphoreGive(server_state.mutex);
            break;
        }
        retry_count++;
        if (retry_count >= HTTP_CLIENT_RETRY_COUNT)
        {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to acquire server mutex");
            return ESP_FAIL;
        }
        vTaskDelay(pdMS_TO_TICKS(HTTP_CLIENT_RETRY_DELAY_MS));
    }

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Status API request received");

    // Check if logs should be included (default to no logs to save bandwidth/CPU)
    const char *logs_param = get_query_param(req->uri, "logs");
    bool include_logs = (logs_param != NULL && strcmp(logs_param, "1") == 0);

    // Set response headers first to ensure proper HTTP response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");

    // Create JSON with minimal initial allocation
    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create JSON root object");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create response");
        goto cleanup;
    }
    track_memory_allocation(sizeof(cJSON));

    // Add only essential status information
    wifi_ap_record_t ap_info;
    bool wifi_connected = (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
    cJSON_AddBoolToObject(root, "wifi_connected", wifi_connected);
    if (wifi_connected)
    {
        cJSON_AddStringToObject(root, "connected_ssid", (char *)ap_info.ssid);
    }

    // Add system information (minimal)
    cJSON_AddStringToObject(root, "app", app);
    cJSON_AddStringToObject(root, "version", version);
    cJSON_AddNumberToObject(root, "fwversion", fwversion);

    // Add additional system information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);

    cJSON_AddNumberToObject(root, "poh", current_poh);
    cJSON_AddNumberToObject(root, "chip_revision", chip_info.revision);
    cJSON_AddNumberToObject(root, "flash_size", flash_size);
    cJSON_AddNumberToObject(root, "cpu_freq", esp_rom_get_cpu_ticks_per_us() * 1000000);
    cJSON_AddStringToObject(root, "compile_time", __DATE__ " " __TIME__);

    // Get MAC address
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(root, "mac_address", mac_str);

    // Get IP address
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
    {
        char ip_str[16];
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
        cJSON_AddStringToObject(root, "ip_address", ip_str);
    }

    // Add memory status
    cJSON_AddNumberToObject(root, "free_heap", esp_get_free_heap_size());
    cJSON_AddNumberToObject(root, "min_free_heap", esp_get_minimum_free_heap_size());
    cJSON_AddNumberToObject(root, "current_memory_usage", memory_stats.current_usage);
    cJSON_AddNumberToObject(root, "peak_memory_usage", memory_stats.peak_usage);

    // Add time status
    cJSON_AddNumberToObject(root, "time_valid", time_valid);
    cJSON_AddNumberToObject(root, "time_just_validated", time_just_validated);

    // Add weather and moon info
    cJSON_AddNumberToObject(root, "weather_icon_index", weather_icon_index);
    cJSON_AddNumberToObject(root, "moon_icon_index", moon_icon_index);

    // Add weather and time status with timestamps
    time_t current_time = time(NULL);
    bool weather_status = (current_time - last_weather_update) < (6 * 3600); // 6 hours
    bool time_status = (current_time - last_time_update) < (2 * 3600);       // 2 hours

    cJSON_AddBoolToObject(root, "weather_status", weather_status);
    cJSON_AddBoolToObject(root, "time_status", time_status);
    cJSON_AddNumberToObject(root, "last_weather_update", (double)last_weather_update);
    cJSON_AddNumberToObject(root, "last_time_update", (double)last_time_update);
    cJSON_AddNumberToObject(root, "uptime", (double)(esp_timer_get_time() / 1000000ULL));

    // Add sensor and location data
    // Add sensor data
    double lux = ltr303_get_frixos_lux();
    lux = round(lux * 100.0) / 100.0; // Round to 2 decimal places
    cJSON_AddNumberToObject(root, "lux", lux);
    cJSON_AddStringToObject(root, "latitude", my_lat);
    cJSON_AddStringToObject(root, "longitude", my_lon);
    cJSON_AddStringToObject(root, "timezone", my_timezone);

    // Add system logs and HA/integration tokens only if requested
    if (include_logs)
    {
        // Add system logs
        cJSON *logs_array = cJSON_CreateArray();
        if (logs_array)
        {
            if (weblog.capacity > 0 && weblog.buffer != NULL)
            {
                int start_index = weblog.full ? weblog.head : 0;
                int current_buf_idx;
                int line_idx = 0;
                char line_buffer[MAX_LOG_LINE_LENGTH];

                for (int i = 0; i < weblog.size; i++)
                {
                    current_buf_idx = (start_index + i) % weblog.capacity;
                    char c = weblog.buffer[current_buf_idx];

                    bool end_of_line = (c == '\n');
                    bool buffer_full = (line_idx >= MAX_LOG_LINE_LENGTH - 1);

                    if (end_of_line || buffer_full)
                    {
                        if (line_idx > 0)
                        {
                            line_buffer[line_idx] = '\0';
                            cJSON_AddItemToArray(logs_array, cJSON_CreateString(line_buffer));
                        }
                        line_idx = 0;
                        if (buffer_full && !end_of_line && c != '\r')
                        {
                            if (line_idx < MAX_LOG_LINE_LENGTH - 1)
                            {
                                line_buffer[line_idx++] = c;
                            }
                        }
                    }
                    else if (c != '\r')
                    {
                        if (line_idx < MAX_LOG_LINE_LENGTH - 1)
                        {
                            line_buffer[line_idx++] = c;
                        }
                    }
                }

                // Add any remaining characters
                if (line_idx > 0)
                {
                    line_buffer[line_idx] = '\0';
                    cJSON_AddItemToArray(logs_array, cJSON_CreateString(line_buffer));
                }
            }
            cJSON_AddItemToObject(root, "system_logs", logs_array);
        }

        // Create JSON array for HA tokens
        cJSON *integration_log = cJSON_CreateArray();
        // Add HA tokens if available

        if (integration_log != NULL)
        {
            // Add combined info for all tokens
            char info[512];
            snprintf(info, sizeof(info), "Home Assistant tokens: %d, Last update: %s",
                     integration_active_tokens_count[INTEGRATION_HA],
                     ctime(&integration_last_update[INTEGRATION_HA]));
            cJSON_AddStringToObject(integration_log, "info", info);

            if (integration_active_tokens_count[INTEGRATION_HA] > 0)
            {
                // Add each HA token
                for (int i = 0; i < integration_active_tokens_count[INTEGRATION_HA]; i++)
                {
                    snprintf(info, sizeof(info), "%d: %s entity %s path %s value %s", i,
                             integration_active_tokens[INTEGRATION_HA][i].name ? integration_active_tokens[INTEGRATION_HA][i].name : "(null)",
                             integration_active_tokens[INTEGRATION_HA][i].entity ? integration_active_tokens[INTEGRATION_HA][i].entity : "(null)",
                             integration_active_tokens[INTEGRATION_HA][i].path ? integration_active_tokens[INTEGRATION_HA][i].path : "(null)",
                             integration_active_tokens[INTEGRATION_HA][i].value);
                    cJSON_AddStringToObject(integration_log, "info", info);
                }
            }

            snprintf(info, sizeof(info), "\nStock (finnhub.io) tokens: %d, Last update: %s",
                     integration_active_tokens_count[INTEGRATION_STOCK],
                     ctime(&integration_last_update[INTEGRATION_STOCK]));
            cJSON_AddStringToObject(integration_log, "info", info);

            // Add stock tokens if available
            if (integration_active_tokens[INTEGRATION_STOCK] != NULL && integration_active_tokens_count[INTEGRATION_STOCK] > 0)
            {
                // Add each stock token
                for (int i = 0; i < integration_active_tokens_count[INTEGRATION_STOCK]; i++)
                {
                    snprintf(info, sizeof(info), "%d: %s symbol %s value %s", i,
                             integration_active_tokens[INTEGRATION_STOCK][i].name ? integration_active_tokens[INTEGRATION_STOCK][i].name : "(null)",
                             integration_active_tokens[INTEGRATION_STOCK][i].entity ? integration_active_tokens[INTEGRATION_STOCK][i].entity : "(null)",
                             integration_active_tokens[INTEGRATION_STOCK][i].value);
                    cJSON_AddStringToObject(integration_log, "info", info);
                }
            }

            // Add dexcom tokens
            if (integration_active[INTEGRATION_DEXCOM])
            {
                snprintf(info, sizeof(info), "\nDexcom active: %d tokens, last update %s",
                         integration_active_tokens_count[INTEGRATION_DEXCOM],
                         ctime(&integration_last_update[INTEGRATION_DEXCOM]));
                cJSON_AddStringToObject(integration_log, "info", info);

                // Format glucose value directly from glucose_data
                char glucose_value[64];
                if (glucose_data.current_gl_mgdl > 0)
                {
                    format_glucose_token(glucose_value, sizeof(glucose_value));
                    snprintf(info, sizeof(info), "[CGM:glucose] value %s", glucose_value);
                }
                else
                {
                    snprintf(info, sizeof(info), "[CGM:glucose] value N/A");
                }
                cJSON_AddStringToObject(integration_log, "info", info);
            }
            else
            {
                snprintf(info, sizeof(info), "\nDexcom not active");
                cJSON_AddStringToObject(integration_log, "info", info);
            }

            // Add freestyle libre tokens
            if (integration_active[INTEGRATION_FREESTYLE])
            {
                snprintf(info, sizeof(info), "\nFreeStyle Libre active: %d tokens, last update %s",
                         integration_active_tokens_count[INTEGRATION_FREESTYLE],
                         ctime(&integration_last_update[INTEGRATION_FREESTYLE]));
                cJSON_AddStringToObject(integration_log, "info", info);

                // Format glucose value directly from glucose_data
                char glucose_value[64];
                if (glucose_data.current_gl_mgdl > 0)
                {
                    format_glucose_token(glucose_value, sizeof(glucose_value));
                    snprintf(info, sizeof(info), "[CGM:glucose] value %s", glucose_value);
                }
                else
                {
                    snprintf(info, sizeof(info), "[CGM:glucose] value N/A");
                }
                cJSON_AddStringToObject(integration_log, "info", info);
            }
            else
            {
                snprintf(info, sizeof(info), "\nFreeStyle Libre not active");
                cJSON_AddStringToObject(integration_log, "info", info);
            }

            // Add Nightscout tokens
            if (integration_active[INTEGRATION_NIGHTSCOUT])
            {
                snprintf(info, sizeof(info), "\nNightscout active: %d tokens, last update %s",
                         integration_active_tokens_count[INTEGRATION_NIGHTSCOUT],
                         ctime(&integration_last_update[INTEGRATION_NIGHTSCOUT]));
                cJSON_AddStringToObject(integration_log, "info", info);

                char glucose_value[64];
                if (glucose_data.current_gl_mgdl > 0)
                {
                    format_glucose_token(glucose_value, sizeof(glucose_value));
                    snprintf(info, sizeof(info), "[CGM:glucose] value %s", glucose_value);
                }
                else
                {
                    snprintf(info, sizeof(info), "[CGM:glucose] value N/A");
                }
                cJSON_AddStringToObject(integration_log, "info", info);
            }
            else
            {
                snprintf(info, sizeof(info), "\nNightscout not active");
                cJSON_AddStringToObject(integration_log, "info", info);
            }

            cJSON_AddItemToObject(root, "ha_tokens", integration_log);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create HA tokens array");
        }
    }

    // Send the response
    char *response = cJSON_Print(root);
    if (response)
    {
        httpd_resp_sendstr(req, response);
        free(response);
    }
    else
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create response");
    }

cleanup:
    // Clean up JSON objects
    if (root)
    {
        track_memory_free(sizeof(cJSON));
        cJSON_Delete(root);
    }

    // Decrement active connections
    if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if (server_state.active_connections > 0)
        {
            server_state.active_connections--;
        }
        xSemaphoreGive(server_state.mutex);
    }

    return ESP_OK;
}

#define FILE_PATH "/spiffs/uploaded.bin"
#define MAX_FILE_SIZE (2500 * 1024) // 2500 KB max file size

esp_err_t ota_post_handler(httpd_req_t *req)
{
    char buf[1024];
    char filename[128] = "firmware.bin"; // Default filename
    int recv_len = 0, total_received = 0, remaining = req->content_len;
    bool is_multipart = false, filename_found = false;
    esp_err_t err = ESP_OK;

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Upload request received with content length: %d", req->content_len);

    if (req->content_len <= 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid content length: %d", req->content_len);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid content length");
        return ESP_FAIL;
    }

    // Check content type
    char content_type[128] = {0};
    size_t content_type_len = httpd_req_get_hdr_value_len(req, "Content-Type");

    if (content_type_len > 0 && content_type_len < sizeof(content_type) - 1)
    {
        httpd_req_get_hdr_value_str(req, "Content-Type", content_type, content_type_len + 1);
        is_multipart = (strstr(content_type, "multipart/form-data") != NULL);
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Content-Type: %s, multipart: %d", content_type, is_multipart);
    }

    // Check if filename is provided in headers
    size_t filename_len = httpd_req_get_hdr_value_len(req, "X-Filename");
    if (filename_len > 0 && filename_len < sizeof(filename) - 1)
    {
        httpd_req_get_hdr_value_str(req, "X-Filename", filename, filename_len + 1);
        filename_found = true;
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Filename from X-Filename header: %s", filename);
    }

    // If no filename in header, check Content-Disposition
    if (!filename_found)
    {
        size_t disp_len = httpd_req_get_hdr_value_len(req, "Content-Disposition");
        if (disp_len > 0 && disp_len < sizeof(buf) - 1)
        {
            char disp_buf[256] = {0};
            httpd_req_get_hdr_value_str(req, "Content-Disposition", disp_buf, disp_len + 1);

            char *fname = strstr(disp_buf, "filename=");
            if (fname)
            {
                fname += 9; // Move past "filename="
                if (*fname == '"')
                    fname++; // Skip starting quote if present

                char *end = strchr(fname, '"');
                if (!end)
                    end = strchr(fname, ';');
                if (!end)
                    end = strchr(fname, '\r');
                if (!end)
                    end = strchr(fname, '\n');
                if (!end)
                    end = fname + strlen(fname);

                int name_len = end - fname;
                if (name_len > 0 && name_len < sizeof(filename) - 1)
                {
                    memset(filename, 0, sizeof(filename));
                    strncpy(filename, fname, name_len);
                    filename_found = true;
                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Filename from Content-Disposition: %s", filename);
                }
            }
        }
    }

    // Read first chunk to extract filename if multipart and not found yet
    if (is_multipart && !filename_found)
    {
        recv_len = httpd_req_recv(req, buf, sizeof(buf) - 1);
        if (recv_len > 0)
        {
            buf[recv_len] = '\0'; // Null terminate for string operations

            // Look for Content-Disposition in the form data
            char *disp = strstr(buf, "Content-Disposition:");
            if (disp)
            {
                char *fname = strstr(disp, "filename=");
                if (fname)
                {
                    fname += 9; // Move past "filename="
                    if (*fname == '"')
                        fname++; // Skip starting quote

                    char *end = strchr(fname, '"');
                    if (!end)
                        end = strchr(fname, '\r');
                    if (!end)
                        end = strchr(fname, '\n');
                    if (end)
                    {
                        int name_len = end - fname;
                        if (name_len > 0 && name_len < sizeof(filename) - 1)
                        {
                            memset(filename, 0, sizeof(filename));
                            strncpy(filename, fname, name_len);
                            filename_found = true;
                            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Filename from multipart form: %s", filename);
                        }
                    }
                }
            }

            // Update remaining data to read
            remaining -= recv_len;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to read first chunk: %d", recv_len);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read uploaded data");
            return ESP_FAIL;
        }
    }

    // Determine if this is a firmware update or a regular file
    bool is_firmware = (strstr(filename, ".bin") != NULL);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Processing %s file: %s, size: %d bytes",
                is_firmware ? "firmware" : "regular", filename, req->content_len);

    // Prepare file or OTA handle
    FILE *file = NULL;
    esp_ota_handle_t ota_handle;
    const esp_partition_t *update_partition = NULL;
    char filepath[256] = {0};

    if (is_firmware)
    {
        // Initialize OTA update
        update_partition = esp_ota_get_next_update_partition(NULL);
        if (!update_partition)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to find OTA partition");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition found");
            return ESP_FAIL;
        }

        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA begin failed: %s", esp_err_to_name(err));
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to start OTA process");
            return ESP_FAIL;
        }

        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "OTA update started for partition: %s", update_partition->label);
    }
    else
    {
        // Open file for writing
        snprintf(filepath, sizeof(filepath), "/spiffs/%s", filename);
        file = fopen(filepath, "wb");
        if (!file)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open file for writing: %s", filepath);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
            return ESP_FAIL;
        }
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "File opened for writing: %s", filepath);
    }

    // Process the first chunk if we already read it (multipart form data)
    if (is_multipart && recv_len > 0)
    {
        char *body_start = strstr(buf, "\r\n\r\n");
        if (body_start)
        {
            body_start += 4; // Move past CRLF separator
            int body_len = recv_len - (body_start - buf);

            if (body_len > 0)
            {
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Processing first chunk body: %d bytes", body_len);

                if (is_firmware)
                {
                    err = esp_ota_write(ota_handle, body_start, body_len);
                    if (err != ESP_OK)
                    {
                        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA write error on first chunk: %s", esp_err_to_name(err));
                        esp_ota_abort(ota_handle);
                        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA write error");
                        return ESP_FAIL;
                    }
                }
                else
                {
                    size_t written = fwrite(body_start, 1, body_len, file);
                    if (written != body_len)
                    {
                        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "File write error on first chunk: %d of %d bytes written",
                                    written, body_len);
                        fclose(file);
                        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "File write error");
                        return ESP_FAIL;
                    }
                }

                total_received += body_len;
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "First chunk processed: %d bytes, total: %d", body_len, total_received);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "No body separator found in first chunk, skipping");
        }
    }

    // Continue receiving data
    int retry_count = 0;
    const int max_retries = 5;

    while (remaining > 0)
    {
        recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

        if (recv_len < 0)
        {
            if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
            {
                // Timeout - retry a few times
                retry_count++;
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Socket timeout (%d/%d), retrying...", retry_count, max_retries);

                if (retry_count >= max_retries)
                {
                    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Too many timeouts, aborting upload");

                    if (is_firmware)
                    {
                        esp_ota_abort(ota_handle);
                    }
                    else
                    {
                        fclose(file);
                    }

                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Upload timeout");
                    return ESP_FAIL;
                }

                // Continue to try again
                vTaskDelay(pdMS_TO_TICKS(100)); // Small delay before retry
                continue;
            }
            else
            {
                // Other errors
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Socket error: %d", recv_len);

                if (is_firmware)
                {
                    esp_ota_abort(ota_handle);
                }
                else
                {
                    fclose(file);
                }

                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Socket error");
                return ESP_FAIL;
            }
        }
        else if (recv_len == 0)
        {
            // Connection closed prematurely
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Connection closed with %d bytes remaining", remaining);
            break;
        }

        // For multipart data, check for boundaries
        if (is_multipart)
        {
            // Simple check for ending boundary (--boundary--)
            if (recv_len >= 6 && memcmp(buf, "--", 2) == 0 &&
                (strstr(buf, "--\r\n") != NULL || strstr(buf, "--\r\n\r\n") != NULL))
            {
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Final boundary detected, ending upload");
                break;
            }
        }

        // Process received data
        if (is_firmware)
        {
            err = esp_ota_write(ota_handle, buf, recv_len);
            if (err != ESP_OK)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA write error: %s", esp_err_to_name(err));
                esp_ota_abort(ota_handle);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA write error");
                return ESP_FAIL;
            }
        }
        else
        {
            size_t written = fwrite(buf, 1, recv_len, file);
            if (written != recv_len)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "File write error: %d of %d bytes written", written, recv_len);
                fclose(file);
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "File write error");
                return ESP_FAIL;
            }
        }

        // Update progress counters
        total_received += recv_len;
        remaining -= recv_len;
        retry_count = 0; // Reset retry counter after successful read

        // Log progress periodically
        if (total_received % 10240 == 0 || total_received == req->content_len)
        {
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Upload progress: %d/%d bytes (%.1f%%)",
                        total_received, req->content_len,
                        (float)total_received * 100 / req->content_len);
        }
    }

    // Finalize the upload
    if (is_firmware)
    {
        err = esp_ota_end(ota_handle);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "OTA end failed: %s", esp_err_to_name(err));
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to finalize firmware update");
            return ESP_FAIL;
        }

        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to set boot partition");
            return ESP_FAIL;
        }

        // Send success response
        httpd_resp_set_type(req, "application/json");
        char response[256];
        snprintf(response, sizeof(response),
                 "{\"status\":\"ok\",\"message\":\"Firmware update successful, rebooting...\",\"size\":%d}",
                 total_received);
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

        // Schedule reboot task
        xTaskCreate(&restart_device, "restart_task", 2048, NULL, 5, NULL);
    }
    else
    {
        // Close file handle
        fclose(file);

        // Verify the file size on disk
        struct stat file_stat;
        if (stat(filepath, &file_stat) == 0)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "File size verification: uploaded %d bytes, file size is %ld bytes",
                        total_received, file_stat.st_size);

            if (file_stat.st_size != total_received)
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Size mismatch detected!");
            }
        }

        // Send success response
        httpd_resp_set_type(req, "application/json");
        char response[256];
        snprintf(response, sizeof(response),
                 "{\"status\":\"ok\",\"message\":\"File %s uploaded successfully\",\"size\":%d}",
                 filename, total_received);
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

        // Notify that display should be updated
        display_changed();
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "File upload complete: %s, %d bytes received", filename, total_received);
    return ESP_OK;
}

// Handler for device reset
esp_err_t reset_post_handler(httpd_req_t *req)
{
    // Respond with success and indicate restart
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\",\"message\":\"Device is restarting...\"}", HTTPD_RESP_USE_STRLEN);

    // Create a task to reset after a brief delay to allow response to be sent
    xTaskCreate(&restart_device, "restart_task", 2048, NULL, 5, NULL);

    return ESP_OK;
}

// Task to restart the device
void restart_device(void *pvParameter)
{
    vTaskDelay(pdMS_TO_TICKS(5000)); // wait 5 seconds to allow response to be sent
    esp_restart();
}

// Add these constants at the top with other defines
#define HTTP_SERVER_TIMEOUT_MS 5000
#define HTTP_SERVER_MAX_REQ_SIZE 4096
#define HTTP_SERVER_MAX_HEADERS 8
#define HTTP_SERVER_MAX_URI_HANDLERS 20
#define HTTP_SERVER_STACK_SIZE 8192 // Increased from 4096 to 8192 bytes

// Add HTTP server error tracking
static struct
{
    uint32_t parse_errors;
    uint32_t socket_errors;
    uint32_t timeout_errors;
    uint32_t last_error_time;
    SemaphoreHandle_t mutex;
} http_error_stats = {0, 0, 0, 0, NULL};

// Custom error handler for HTTP 400 errors
static esp_err_t http_400_error_handler(httpd_req_t *req, httpd_err_code_t error)
{
    uint32_t current_time = esp_timer_get_time() / 1000000; // Convert to seconds

    // Only log if it's a real request (not just network noise) and not too frequent
    if (strlen(req->uri) > 0 &&
        (current_time - http_error_stats.last_error_time) > 5)
    { // Log max once every 5 seconds
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "HTTP 400 error: URI='%s', method=%d, content_len=%d",
                    req->uri, req->method, req->content_len);
        http_error_stats.last_error_time = current_time;
    }
    return ESP_OK;
}

// Add HTTP request validation helper
static bool validate_http_request(httpd_req_t *req)
{
    if (!req)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid request pointer");
        return false;
    }

    // Log ALL requests for debugging (including malformed ones)
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HTTP request received: method=%d, content_len=%d, uri='%s'",
                req->method, req->content_len, req->uri);

    // Check request method
    if (req->method != HTTP_GET && req->method != HTTP_POST)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid request method: %d", req->method);
        return false;
    }

    // Check content length for POST requests
    if (req->method == HTTP_POST && req->content_len > HTTP_SERVER_MAX_REQ_SIZE)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Request too large: %d bytes (max: %d)", req->content_len, HTTP_SERVER_MAX_REQ_SIZE);
        return false;
    }

    return true;
}

// Wrapper for handlers that need HTTP mutex protection during OTA
static esp_err_t http_mutex_wrapper(httpd_req_t *req, esp_err_t (*handler)(httpd_req_t *))
{
    // Log the request before processing
    ESP_LOG_WEB(ESP_LOG_DEBUG, TAG, "Processing request: %s", req->uri);

    // Try to take HTTP mutex with timeout to prevent blocking during OTA
    if (xSemaphoreTake(http_mutex, pdMS_TO_TICKS(5000)) != pdTRUE)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "HTTP mutex unavailable, request blocked during OTA update");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Service temporarily unavailable during update");
        return ESP_FAIL;
    }

    esp_err_t result = handler(req);
    xSemaphoreGive(http_mutex);

    // Log the result
    ESP_LOG_WEB(ESP_LOG_DEBUG, TAG, "Request processed with result: %s", esp_err_to_name(result));
    return result;
}

// Status handler wrapper with validation
static esp_err_t status_handler_wrapper(httpd_req_t *req)
{
    if (!validate_http_request(req))
    {
        if (http_error_stats.mutex && xSemaphoreTake(http_error_stats.mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            http_error_stats.parse_errors++;
            xSemaphoreGive(http_error_stats.mutex);
        }
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }

    // Status requests should always work - don't use HTTP mutex for them
    // This ensures the web UI can always get status data
    return status_api_handler(req);
}

// Wrapper for settings GET handler
static esp_err_t settings_get_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, send_json_settings);
}

// Wrapper for settings POST handler
static esp_err_t settings_post_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, settings_post_handler);
}

// Wrapper for OTA POST handler
static esp_err_t ota_post_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, ota_post_handler);
}

// Wrapper for reset POST handler
static esp_err_t reset_post_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, reset_post_handler);
}

// Wrapper for WiFi scan start handler
static esp_err_t wifi_scan_start_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, wifi_scan_start_handler);
}

// Wrapper for WiFi scan status handler
static esp_err_t wifi_scan_status_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, wifi_scan_status_handler);
}

// Wrapper for generic file handler
static esp_err_t generic_file_wrapper(httpd_req_t *req)
{
    return http_mutex_wrapper(req, generic_file_handler);
}

// Catch-all handler for any unhandled POST requests
static esp_err_t catch_all_handler(httpd_req_t *req)
{
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Unhandled POST request: URI='%s', method=%d, content_len=%d",
                req->uri, req->method, req->content_len);

    // Send a proper 404 response instead of letting it cause parsing errors
    httpd_resp_set_status(req, "404 Not Found");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"error\":\"Not Found\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Update start_webserver with improved configuration
esp_err_t start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Basic configuration
    config.stack_size = HTTP_SERVER_STACK_SIZE;
    config.max_uri_handlers = HTTP_SERVER_MAX_URI_HANDLERS;
    config.max_resp_headers = HTTP_SERVER_MAX_HEADERS;
    config.uri_match_fn = httpd_uri_match_wildcard; // vital, otherwise our wildcard handler doesn't work
    config.backlog_conn = 3;                        // Reduced to prevent connection buildup
    config.lru_purge_enable = true;
    config.max_open_sockets = 7;      // Limit concurrent connections
    config.keep_alive_enable = false; // Disable keep-alive to prevent parsing issues

    // Timeout settings
    config.send_wait_timeout = HTTP_SERVER_TIMEOUT_MS / 1000;
    config.recv_wait_timeout = HTTP_SERVER_TIMEOUT_MS / 1000;

    // Task settings
    config.task_priority = tskIDLE_PRIORITY + 5;
    config.core_id = 0;
    config.server_port = 80;

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Starting HTTP server with config: stack=%d, max_handlers=%d, max_headers=%d",
                config.stack_size, config.max_uri_handlers, config.max_resp_headers);

    // Initialize HTTP error tracking
    if (!http_error_stats.mutex)
    {
        http_error_stats.mutex = xSemaphoreCreateMutex();
        if (!http_error_stats.mutex)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create HTTP error tracking mutex");
            return ESP_FAIL;
        }
    }

    // Check memory before starting server
    if (!check_memory_available())
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Not enough memory to start server");
        return ESP_ERR_NO_MEM;
    }

    // Stop existing server if running
    if (server != NULL)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Stopping existing server before restart");
        stop_webserver();
        vTaskDelay(pdMS_TO_TICKS(100)); // Give time for cleanup
    }

    // Start server
    esp_err_t ret = httpd_start(&server, &config);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register custom error handler to catch parsing errors
    httpd_register_err_handler(server, HTTPD_400_BAD_REQUEST, http_400_error_handler);

    // Register URI handlers with improved error handling
    httpd_uri_t uri_handler;

    // Status handler with validation
    uri_handler.uri = "/api/status";
    uri_handler.method = HTTP_GET;
    uri_handler.handler = status_handler_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register status handler");
        return ret;
    }

    // Settings GET handler
    uri_handler.uri = "/api/settings";
    uri_handler.method = HTTP_GET;
    uri_handler.handler = settings_get_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register settings GET handler");
        return ret;
    }

    // Settings POST handler
    uri_handler.uri = "/api/settings";
    uri_handler.method = HTTP_POST;
    uri_handler.handler = settings_post_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register settings POST handler");
        return ret;
    }

    // OTA handler
    uri_handler.uri = "/api/ota";
    uri_handler.method = HTTP_POST;
    uri_handler.handler = ota_post_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register OTA handler");
        return ret;
    }

    // Reset handler
    uri_handler.uri = "/api/reset";
    uri_handler.method = HTTP_POST;
    uri_handler.handler = reset_post_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register reset handler");
        return ret;
    }

    // WiFi scan handlers
    uri_handler.uri = "/api/wifi/scan";
    uri_handler.method = HTTP_GET;
    uri_handler.handler = wifi_scan_start_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register WiFi scan handler");
        return ret;
    }

    uri_handler.uri = "/api/wifi/status";
    uri_handler.method = HTTP_GET;
    uri_handler.handler = wifi_scan_status_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register WiFi status handler");
        return ret;
    }

    // Register wildcard handler for all other paths
    uri_handler.uri = "/*"; // Wildcard for all other paths
    uri_handler.method = HTTP_GET;
    uri_handler.handler = generic_file_wrapper;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register wildcard handler");
        return ret;
    }

    // Add a catch-all handler for any other methods to prevent parsing errors
    uri_handler.uri = "/*";
    uri_handler.method = HTTP_POST;
    uri_handler.handler = catch_all_handler;
    uri_handler.user_ctx = NULL;
    ret = httpd_register_uri_handler(server, &uri_handler);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register catch-all POST handler");
        return ret;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "HTTP server started successfully");
    return ESP_OK;
}

// Update stop_webserver with improved cleanup
void stop_webserver()
{
    if (server)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stopping HTTP server");

        // Take mutex to prevent new connections
        if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            server_state.is_restarting = true;
            xSemaphoreGive(server_state.mutex);
        }

        // Wait for active connections to complete
        int retry_count = 0;
        while (server_state.active_connections > 0 && retry_count < 50)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            retry_count++;
        }

        // Force close all connections
        httpd_stop(server);
        server = NULL;

        // Clean up error tracking
        if (http_error_stats.mutex)
        {
            vSemaphoreDelete(http_error_stats.mutex);
            http_error_stats.mutex = NULL;
        }

        // Reset server state
        if (server_state.mutex && xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            server_state.active_connections = 0;
            server_state.is_restarting = false;
            xSemaphoreGive(server_state.mutex);
        }

        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "HTTP server stopped");
    }
}

// Handler for starting WiFi scan
esp_err_t wifi_scan_start_handler(httpd_req_t *req)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi scan start request received");

    // Clear any previous results
    ap_count = 0;
    wifi_scan_done = false;

    if (!wifi_scan_running)
    {
        esp_err_t result = start_wifi_scan();
        if (result != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start scan: %s", esp_err_to_name(result));
            wifi_scan_done = true;
            wifi_scan_running = false;

            // Return error response
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Failed to start WiFi scan\"}", HTTPD_RESP_USE_STRLEN);
            return ESP_FAIL;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Scan started successfully");
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Scan already running, ignoring request");
    }

    // Return success response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\",\"message\":\"WiFi scan started\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for getting WiFi scan status and results
esp_err_t wifi_scan_status_handler(httpd_req_t *req)
{
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi scan status request received");

    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "scanning", wifi_scan_running);
    cJSON_AddBoolToObject(root, "scan_done", wifi_scan_done);

    // Add networks array if scan is done
    if (wifi_scan_done && ap_count > 0)
    {
        cJSON *networks = cJSON_CreateArray();

// Create a map to track unique SSIDs and their strongest signal
// We'll use a simple array-based approach since we have limited memory
#define MAX_UNIQUE_NETWORKS MAX_AP_SCAN
        struct
        {
            char ssid[33];
            int8_t rssi;
            uint8_t authmode;
            int index; // Index in ap_records
        } unique_networks[MAX_UNIQUE_NETWORKS];
        int unique_count = 0;

        // First pass: find unique networks with strongest signal
        for (int i = 0; i < ap_count; i++)
        {
            // Convert SSID bytes to proper string (it might not be null-terminated)
            char ssid_str[33] = {0}; // 32 bytes for SSID + null terminator
            memcpy(ssid_str, (char *)ap_records[i].ssid, sizeof(ap_records[i].ssid));
            ssid_str[sizeof(ap_records[i].ssid) - 1] = '\0';

            // Skip empty SSIDs
            if (strlen(ssid_str) == 0)
            {
                continue;
            }

            // Check if we've seen this SSID before
            bool found = false;
            for (int j = 0; j < unique_count; j++)
            {
                if (strcmp(unique_networks[j].ssid, ssid_str) == 0)
                {
                    // Found a duplicate SSID, keep the one with stronger signal
                    if (ap_records[i].rssi > unique_networks[j].rssi)
                    {
                        unique_networks[j].rssi = ap_records[i].rssi;
                        unique_networks[j].authmode = ap_records[i].authmode;
                        unique_networks[j].index = i;
                    }
                    found = true;
                    break;
                }
            }

            // If not found and we have space, add it to unique networks
            if (!found && unique_count < MAX_UNIQUE_NETWORKS)
            {
                strncpy(unique_networks[unique_count].ssid, ssid_str, sizeof(unique_networks[unique_count].ssid) - 1);
                unique_networks[unique_count].rssi = ap_records[i].rssi;
                unique_networks[unique_count].authmode = ap_records[i].authmode;
                unique_networks[unique_count].index = i;
                unique_count++;
            }
        }

        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Found %d unique networks out of %d total networks", unique_count, ap_count);

        // Second pass: create JSON objects for unique networks
        for (int i = 0; i < unique_count; i++)
        {
            cJSON *network = cJSON_CreateObject();

            cJSON_AddStringToObject(network, "ssid", unique_networks[i].ssid);
            cJSON_AddNumberToObject(network, "rssi", unique_networks[i].rssi);

            // Calculate signal strength percentage
            int rssi_percentage = 2 * (100 + unique_networks[i].rssi);
            if (rssi_percentage > 99)
                rssi_percentage = 99;
            if (rssi_percentage < 0)
                rssi_percentage = 0;
            cJSON_AddNumberToObject(network, "signal_strength", rssi_percentage);

            // Add authentication mode
            bool requires_password = (unique_networks[i].authmode != WIFI_AUTH_OPEN);
            cJSON_AddBoolToObject(network, "requires_password", requires_password);

            cJSON_AddItemToArray(networks, network);
        }

        cJSON_AddItemToObject(root, "networks", networks);
        cJSON_AddNumberToObject(root, "count", cJSON_GetArraySize(networks));
    }
    else
    {
        cJSON_AddNumberToObject(root, "count", 0);
    }

    // Convert JSON to string and send response
    char *json_str = cJSON_PrintUnformatted(root); // Reduced memory usage
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, HTTPD_RESP_USE_STRLEN);

    // Clean up
    cJSON_free(json_str);
    cJSON_Delete(root);

    return ESP_OK;
}

// Initialize settings server
esp_err_t init_settings_server()
{
    esp_err_t ret;

    // Initialize memory tracking mutex
    memory_stats.mutex = xSemaphoreCreateMutex();
    if (memory_stats.mutex == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create memory tracking mutex");
        return ESP_FAIL;
    }

    // Initialize server state mutex
    server_state.mutex = xSemaphoreCreateMutex();
    if (server_state.mutex == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create server state mutex");
        vSemaphoreDelete(memory_stats.mutex);
        return ESP_FAIL;
    }

    // Initialize server state
    if (xSemaphoreTake(server_state.mutex, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        server_state.active_connections = 0;
        server_state.is_restarting = false;
        xSemaphoreGive(server_state.mutex);
    }

    // Initialize WiFi scan semaphore
    scan_semaphore = xSemaphoreCreateBinary();
    if (scan_semaphore == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create WiFi scan semaphore");
        vSemaphoreDelete(memory_stats.mutex);
        vSemaphoreDelete(server_state.mutex);
        return ESP_FAIL;
    }

    // Register WiFi event handler for scan results
    ret = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &wifi_scan_event_handler, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to register WiFi event handler");
        vSemaphoreDelete(memory_stats.mutex);
        vSemaphoreDelete(server_state.mutex);
        vSemaphoreDelete(scan_semaphore);
        return ret;
    }

    // Start the webserver
    ret = start_webserver();
    if (ret != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start webserver");
        vSemaphoreDelete(memory_stats.mutex);
        vSemaphoreDelete(server_state.mutex);
        vSemaphoreDelete(scan_semaphore);
        return ret;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Web server started");
    return ESP_OK;
}

// In the start_webserver function, add the new endpoint
// Extended settings are now handled by the main settings_post_handler