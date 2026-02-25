/*
 * WiFi and Network Management for Frixos
 * 
 * This module handles:
 * - WiFi connection management
 * - DHCP NTP server detection and usage
 * - mDNS service initialization
 * - Weather and location data retrieval
 * - OTA update management
 * 
 * DHCP NTP Feature:
 * When receiving a DHCP address, the system checks for NTP servers in DHCP options.
 * If local network NTP servers are detected, they are used instead of public servers.
 * This improves time synchronization accuracy in enterprise environments.
 */

#include "frixos.h"
#include "config.h"
#include "f-integrations.h"

#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "mdns.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/ip4_addr.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"

#include "f-wifi.h"
#include "f-time.h"
#include "f-provisioning.h"
#include "f-ota.h"
#include "f-membuffer.h"
#include "f-json.h"

int response_len = 0;
char *wifi_http_buffer = NULL;

static const char *TAG = "f-wifi";
volatile bool wifi_connected = false; // volatile as it may be update within an ISR
bool wifi_first_connect = false;
bool mdns_initialized = false;
bool manufacturer_mode = false;
// Added for OTA updates
#define OTA_CHECK_DELAY_MS (7 * 1000) // 7 seconds delay
esp_timer_handle_t ota_update_timer = NULL;
// Added for mDNS periodic announcements
#define MDNS_ANNOUNCEMENT_INTERVAL_MS (20 * 1000) // 20 seconds
esp_timer_handle_t mdns_announcement_timer = NULL;
// Added for WiFi Active Hours
#define WIFI_ACTIVE_HOURS_CHECK_INTERVAL_MS (10 * 60 * 1000) // 10 minutes
esp_timer_handle_t wifi_active_hours_timer = NULL;
volatile bool wifi_disabled_by_active_hours = false; // Track if WiFi is disabled due to active hours

// Global Variables
char my_ip[18], city[64], country[64], internal_ip[18];
double dlat = 0.0, dlon = 0.0;

// Weather Variables
char icon_today[16] = "";
time_t sunrise = 0, sunset = 0;
bool weather_valid = false;
esp_timer_handle_t weather_timer = NULL;
double weather_high, weather_low;
char greeting[64] = "Hello";

uint64_t weather_delay_ms = 5 * 60 * 60 * 1000; // Start with 5 hours
esp_timer_handle_t location_timer;
uint64_t location_delay_ms = 100; // Start with 0.1 seconds

#define WEATHER_UPDATE_MS (3 * 60 * 60 * 1000 * 1000ULL) // time in us (microseconds), not milliseconds. 3 hours
#define UPDATE_SERVER_BASE "http://update.artlogic.gr:8080"

// Add these global variables at the top with other globals
static bool is_forecast_request = false;
static double forecast_high = -100.0;
static double forecast_low = 100.0;
static int forecast_temp_count = 0;

// NTP server functionality removed - using default servers only

// Function prototypes
void ota_update_timer_callback(void *arg);
void mdns_announcement_timer_callback(void *arg);
static void wifi_active_hours_timer_callback(void *arg);
static bool is_wifi_active_hours(void);

// Function to check if WiFi is connected
bool is_wifi_connected(void)
{
    wifi_ap_record_t ap_info;
    return (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK);
}



// Function to URL-encode a string.
// The caller MUST ensure the output buffer is large enough.
// Unsafe characters (non-alphanumeric except -_.~) are converted to %XX.
void url_encode_string(const char *input, char *output)
{
    if (!input || !output)
        return;

    // First pass: calculate the required length
    size_t needed_len = 0;
    for (const char *p = input; *p; p++)
    {
        // Check if the character is alphanumeric or one of the safe characters (-_.~)
        if (isalnum((unsigned char)*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~')
        {
            needed_len += 1;
        }
        else
        {
            needed_len += 3; // %XX
        }
    }
    needed_len++; // For null terminator

    // Note: We assume the caller provided a buffer of at least needed_len.
    // A safer version might check output buffer size or allocate dynamically.

    // Second pass: build the encoded string
    char *dst = output;
    for (const char *p = input; *p; p++)
    {
        if (isalnum((unsigned char)*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~')
        {
            *dst++ = *p; // Safe character, copy directly
        }
        else
        {
            // Unsafe character, encode as %XX
            snprintf(dst, 4, "%%%02X", (unsigned char)*p);
            dst += 3;
        }
    }
    *dst = '\0'; // Null terminate
}

// Initialize mDNS service
void initialize_mdns(void)
{
    if (mdns_initialized)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "mDNS already initialized, stopping previous instance");
        mdns_free();
        mdns_initialized = false;
    }

    // Initialize mDNS
    esp_err_t err = mdns_init();
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize mDNS: %s", esp_err_to_name(err));
        return;
    }

    // Set hostname
    err = mdns_hostname_set(eeprom_hostname);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set mDNS hostname: %s", esp_err_to_name(err));
        return;
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "mDNS hostname set to: %s", eeprom_hostname);
    }

    // Set default instance
    err = mdns_instance_name_set("Frixos Configuration Web Server");
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set mDNS instance name: %s", esp_err_to_name(err));
        return;
    }

    char device_name[64];
    snprintf(device_name, sizeof(device_name), "frixos projection clock %s", eeprom_hostname);

    // Add service with more parameters
    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"device", device_name},
        {"version", version}};

    err = mdns_service_add(NULL, "_http", "_tcp", 80,
                           serviceTxtData, sizeof(serviceTxtData) / sizeof(serviceTxtData[0]));
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to add mDNS service: %s", esp_err_to_name(err));
        mdns_free();
        mdns_initialized = false;
        return;
    }

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "mDNS initialized. Hostname: %s.local", eeprom_hostname);
    mdns_initialized = true;
}

// OTA update timer callback
void ota_update_timer_callback(void *arg)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA Update Timer");

    // Trigger the OTA update thread
    f_ota_trigger_update();

    // Restart the timer for the next check interval
    if (ota_update_timer != NULL)
    {
        esp_err_t err = esp_timer_start_once(ota_update_timer, (uint64_t)(UPDATE_CHECK_INTERVAL * (uint64_t)(1000000))); // Convert seconds to microseconds
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to restart OTA update timer: %s", esp_err_to_name(err));
        }
    }
}

// mDNS announcement timer callback
void mdns_announcement_timer_callback(void *arg)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "mDNS Announcement Timer triggered");

    if (wifi_connected && mdns_initialized)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "WiFi connected, sending mDNS announcement");
        mdns_service_instance_name_set("_http", "_tcp", "Frixos Configuration Web Server");
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "mDNS announcement sent for %s.local", eeprom_hostname);
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "WiFi disconnected or mDNS not initialized, skipping announcement");
    }

    // Restart the timer for the next announcement
    if (mdns_announcement_timer != NULL)
    {
        esp_err_t err = esp_timer_start_once(mdns_announcement_timer, MDNS_ANNOUNCEMENT_INTERVAL_MS * 1000);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to restart mDNS announcement timer: %s", esp_err_to_name(err));
        }
    }
}

// Helper function to check if WiFi should be active based on active hours
static bool is_wifi_active_hours(void)
{
    // If both start and end are 0, WiFi is always ON (no restrictions)
    if (eeprom_wifi_start == 0 && eeprom_wifi_end == 0)
    {
        return true;
    }
    
    // If start == end (non-zero), WiFi is always ON (24/7)
    if (eeprom_wifi_start == eeprom_wifi_end && eeprom_wifi_start != 0)
    {
        return true;
    }
    
    // Check if time is valid (year > 2025)
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    if (timeinfo.tm_year + 1900 <= 2025)
    {
        // Time not valid, don't disable WiFi
        return true;
    }
    
    int current_hour = timeinfo.tm_hour;
    
    // WiFi is OFF if current_hour > wifi_end AND current_hour < wifi_start
    // WiFi is ON if current_hour >= wifi_start AND current_hour <= wifi_end
    if (eeprom_wifi_start <= eeprom_wifi_end)
    {
        // Normal case: start <= end (e.g., 8-14 means 8:00 to 14:59)
        return (current_hour >= eeprom_wifi_start && current_hour <= eeprom_wifi_end);
    }
    else
    {
        // Wrap-around case: start > end (e.g., 14-8 means 14:00-23:59 and 0:00-8:59)
        return (current_hour >= eeprom_wifi_start || current_hour <= eeprom_wifi_end);
    }
}

// WiFi Active Hours timer callback
static void wifi_active_hours_timer_callback(void *arg)
{
    bool should_be_on = is_wifi_active_hours();
    bool currently_connected = is_wifi_connected();
        
    if (!should_be_on && currently_connected)
    {
        // Should be OFF but currently connected - disconnect WiFi
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi Active Hours: Disabling WiFi (current hour outside active hours)");
        wifi_disabled_by_active_hours = true;
        esp_wifi_disconnect();
    }
    else if (should_be_on && !currently_connected && wifi_disabled_by_active_hours)
    {
        // Should be ON but currently disconnected due to active hours - connect WiFi
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi Active Hours: Enabling WiFi (current hour within active hours)");
        wifi_disabled_by_active_hours = false;
        esp_wifi_connect();
    }
}

// WiFi event handler
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    static int retry_count = 0;
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "WiFi station mode started");
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi disconnected, retry %d", retry_count);
            wifi_connected = false;

            // Stop OTA update timer when disconnected
            if (ota_update_timer != NULL)
            {
                esp_timer_stop(ota_update_timer);
                ota_update_timer = NULL;
            }

            // Stop mDNS announcement timer when disconnected
            if (mdns_announcement_timer != NULL)
            {
                esp_timer_stop(mdns_announcement_timer);
            }

            // Don't reconnect if WiFi was intentionally disabled due to active hours
            if (!wifi_disabled_by_active_hours)
            {
                vTaskDelay(pdMS_TO_TICKS(5000)); // Small 5 sec delay before retry
                esp_wifi_connect();
                retry_count++;
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi intentionally disabled due to active hours - not reconnecting");
            }
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            snprintf(internal_ip, sizeof(internal_ip), IPSTR, IP2STR(&event->ip_info.ip));
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            retry_count = 0;
            if (manufacturer_mode) // if in manufacturer mode, use custom device name
            {
                // Append last 3 bytes of MAC address to hostname
                uint8_t mac[6];
                esp_efuse_mac_get_default(mac);
                snprintf(eeprom_hostname, sizeof(eeprom_hostname), "frixos-%02X", mac[5]);
            }
            wifi_connected = true;
            
            // Set up IP address display on first boot connection
            if (!wifi_first_connect)
            {
                snprintf(boot_ip_address, sizeof(boot_ip_address), "%s", internal_ip);
                show_ip_on_boot = true;
                ip_display_start_time = 0; // Will be set when message is displayed
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Will display IP address %s for %d seconds", boot_ip_address, IP_DISPLAY_DURATION_SEC);
            }

            initialize_mdns();

            // Create and start OTA update timer if not already created
            if (ota_update_timer == NULL)
            {
                esp_timer_create_args_t ota_timer_args = {
                    .callback = &ota_update_timer_callback,
                    .name = "ota_update_timer"};
                ESP_ERROR_CHECK(esp_timer_create(&ota_timer_args, &ota_update_timer));
            }

            // Start OTA update timer with initial delay
            esp_err_t err = esp_timer_start_once(ota_update_timer, OTA_CHECK_DELAY_MS * 1000);
            if (err != ESP_OK)
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to start OTA update timer: %s", esp_err_to_name(err));
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "OTA update timer started, first check in %d seconds", OTA_CHECK_DELAY_MS / 1000);
            }

            // Create and start mDNS announcement timer if not already created
            if (mdns_announcement_timer == NULL)
            {
                esp_timer_create_args_t mdns_timer_args = {
                    .callback = &mdns_announcement_timer_callback,
                    .name = "mdns_announcement_timer"};
                ESP_ERROR_CHECK(esp_timer_create(&mdns_timer_args, &mdns_announcement_timer));
            }

            // Start mDNS announcement timer with initial delay of 1 second
            err = esp_timer_start_once(mdns_announcement_timer, 1000 * 1000); // 1 second in microseconds
            if (err != ESP_OK)
            {
                ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to start mDNS announcement timer: %s", esp_err_to_name(err));
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "mDNS announcement timer started, first announcement in 1 second");
            }
            
            // If WiFi reconnected and we're within active hours, trigger integrations
            if (is_wifi_active_hours())
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi reconnected within active hours - triggering integrations");
                extern void force_integration_update(void);
                force_integration_update();
            }
            wifi_disabled_by_active_hours = false; // Reset flag on successful connection
        }
    }
}

int map_weather_icon(const char *icon)
{
    if (strncmp(icon, "01", 2) == 0)
        return 0;
    if (strncmp(icon, "02", 2) == 0)
        return 1;
    if (strncmp(icon, "03", 2) == 0)
        return 2;
    if (strncmp(icon, "04", 2) == 0)
        return 3;
    if (strncmp(icon, "09", 2) == 0)
        return 4;
    if (strncmp(icon, "10", 2) == 0)
        return 4;
    if (strncmp(icon, "11", 2) == 0)
        return 5;
    if (strncmp(icon, "13", 2) == 0)
        return 6;
    if (strncmp(icon, "50", 2) == 0)
        return 7;
    return -1;
}

void weather_timer_callback(void *arg)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "ESP Weather timer triggered.");
    wifi_get_weather();

    // we might as well calculate the moon phase too
    // we do it here, with every weather, but also when NTP is synchronized
    moon_icon_index = get_moon_index();
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Moon phase: %d", moon_icon_index);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Weather next in %.1f hours.", (weather_delay_ms / (1000 * 60 * 60.0)));
    // Restart the timer with the new delay
    ESP_ERROR_CHECK(esp_timer_start_once(weather_timer, weather_delay_ms * 1000)); // Convert ms to microseconds

    // Your task logic here
}

void location_timer_callback(void *arg)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "ESP Location timer triggered.");
    if (wifi_get_location()) // If successful
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Location acquired successfully, stopping retries.");
        
        // Use default NTP servers
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Initializing NTP with default servers");
        sync_time_with_ntp();
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Could not acquire location, next attempt in %.0f seconds.", location_delay_ms / 1000.0);
        // Didn't work this time
        // Increase delay for next attempt (10s, 20s, 30s, ...)
        location_delay_ms += 10000;
        // Restart the timer with the new delay
        ESP_ERROR_CHECK(esp_timer_start_once(location_timer, location_delay_ms * 1000)); // Convert ms to microseconds
    }

    // if this is the first connection, launch the weather timer
    if (wifi_first_connect == false)
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "First connection, launching weather timer");
        wifi_first_connect = true; // first connection done
        // launch the weather timer
        ESP_ERROR_CHECK(esp_timer_start_once(weather_timer, 100 * 1000)); // start in 100ms
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Not first connection, skipping weather timer, but restarting NTP");
        
        // Use default NTP servers
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Initializing NTP with default servers");
        sync_time_with_ntp();
    }
}

void wifi_task(void *pvParameters)
{
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Wifi task (wifi_task) started.");

    // Start the OTA update thread
    f_ota_start_update_thread();

    // Register event handlers for WiFi events
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Create a one-shot timer for location retrieval
    esp_timer_create_args_t location_timer_args = {
        .callback = &location_timer_callback,
        .name = "location_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&location_timer_args, &location_timer));

    esp_timer_create_args_t weather_timer_args = {
        .callback = &weather_timer_callback,
        .name = "weather_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&weather_timer_args, &weather_timer));

    // Create WiFi Active Hours timer
    esp_timer_create_args_t wifi_active_hours_timer_args = {
        .callback = &wifi_active_hours_timer_callback,
        .name = "wifi_active_hours_timer"};
    ESP_ERROR_CHECK(esp_timer_create(&wifi_active_hours_timer_args, &wifi_active_hours_timer));
    
    // Start WiFi Active Hours timer (first check after 10 minutes, then periodic)
    esp_err_t err = esp_timer_start_periodic(wifi_active_hours_timer, WIFI_ACTIVE_HOURS_CHECK_INTERVAL_MS * 1000); // Convert to microseconds
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start WiFi Active Hours timer: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi Active Hours timer started - checking every 10 minutes");
    }

    int location_check_started = 0;

    while (1)
    {
        // only start trying to get location after WiFi is connected
        if (wifi_connected && !location_check_started)
        {
            location_check_started = 1;
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "New internet connection - initializing location.");
            // Start the location timer (first attempt in 10s)
            ESP_ERROR_CHECK(esp_timer_start_once(location_timer, location_delay_ms * 1000));
        }

        taskYIELD();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Tick every second
    }
    vTaskDelete(NULL);
}

// Add this new function before http_event_handler
static void process_forecast_chunk(const char *chunk, int len) {
    char value_buffer[64];
    char *remaining = (char *)chunk;
    
    while (remaining && (remaining - chunk) < len) {
        char *temp_str = get_value_from_JSON_string(remaining, "temp_min", value_buffer, sizeof(value_buffer), &remaining);
        if (strcmp(temp_str, "-") != 0) {
            double temp_value = strtod(temp_str, NULL);
            forecast_temp_count++;
            if (temp_value < forecast_low) {
                forecast_low = temp_value;
            }
        }
        
        temp_str = get_value_from_JSON_string(remaining, "temp_max", value_buffer, sizeof(value_buffer), &remaining);
        if (strcmp(temp_str, "-") != 0) {
            double temp_value = strtod(temp_str, NULL);
            forecast_temp_count++;
            if (temp_value > forecast_high) {
                forecast_high = temp_value;
            }
        }
    }
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
    case HTTP_EVENT_REDIRECT:
    case HTTP_EVENT_ON_CONNECTED:
    case HTTP_EVENT_ON_HEADER:
    case HTTP_EVENT_DISCONNECTED:
    case HTTP_EVENT_HEADERS_SENT:
        break;

    case HTTP_EVENT_ON_DATA:
        if (is_forecast_request) {
            // Process forecast data incrementally
            process_forecast_chunk(evt->data, evt->data_len);
        } else {
            // Get a shared buffer if we don't have one
            if (wifi_http_buffer == NULL)
            {
                wifi_http_buffer = get_shared_buffer(HTTP_BUFFER_SIZE, "HTTP_RESPONSE");
                if (wifi_http_buffer == NULL)
                {
                    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to get shared buffer for HTTP response");
                    return ESP_FAIL;
                }
                response_len = 0;
            }

            if ((response_len + evt->data_len) < HTTP_BUFFER_SIZE)
            {
                memcpy(wifi_http_buffer + response_len, evt->data, evt->data_len);
                response_len += evt->data_len;
                wifi_http_buffer[response_len] = '\0';
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Response buffer overflow!");
                response_len = 0;
            }
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HTTP Event: FINISH");
        break;
    }
    return ESP_OK;
}

/**
 * Validate latitude/longitude string
 * Returns true if the string is a valid numeric coordinate within valid range
 */
bool validate_coordinate(const char *coord_str, bool is_latitude)
{
    if (coord_str == NULL || strlen(coord_str) == 0)
    {
        return false;
    }

    // Check if string contains only valid characters: digits, decimal point, and optional minus sign
    for (int i = 0; coord_str[i] != '\0'; i++)
    {
        char c = coord_str[i];
        if (!((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+'))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid coordinate contains non-numeric character: '%c' in '%s'", c, coord_str);
            return false;
        }
    }

    // Try to parse as double
    char *endptr;
    double value = strtod(coord_str, &endptr);
    
    // Check if entire string was consumed (valid number)
    if (*endptr != '\0')
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid coordinate format: '%s' (unparsed: '%s')", coord_str, endptr);
        return false;
    }

    // Check range
    if (is_latitude)
    {
        if (value < -90.0 || value > 90.0)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Latitude out of range: %.7f (must be -90 to 90)", value);
            return false;
        }
    }
    else
    {
        if (value < -180.0 || value > 180.0)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Longitude out of range: %.7f (must be -180 to 180)", value);
            return false;
        }
    }

    return true;
}

/**
 * Fetch Weather Data from OpenWeatherMap
 */
bool wifi_get_weather(void)
{
    bool result = false;
    char weather_url[512];

    // Validate coordinates before constructing URL
    if (!validate_coordinate(my_lat, true))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid latitude value: '%s'. Skipping weather fetch.", my_lat);
        return false;
    }

    if (!validate_coordinate(my_lon, false))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Invalid longitude value: '%s'. Skipping weather fetch.", my_lon);
        return false;
    }

    // Get current weather
    snprintf(weather_url, sizeof(weather_url),
             "http://api.openweathermap.org/data/2.5/weather?lat=%s&lon=%s&appid=%s&units=metric",
             my_lat, my_lon, WEATHER_API_KEY);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Fetching Weather from %.10s...", weather_url);

    esp_http_client_config_t config = {
        .url = weather_url,
        .event_handler = http_event_handler,
        .timeout_ms = 5000,
        .buffer_size = HTTP_BUFFER_SIZE,
        .buffer_size_tx = HTTP_BUFFER_SIZE,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200 && wifi_http_buffer != NULL)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Weather data received successfully");

            char value_buffer[64];

            // Get weather icon from the first weather array item
            get_value_from_JSON_string(wifi_http_buffer, "icon", icon_today, sizeof(icon_today), NULL);
            if (strcmp(icon_today, "-") != 0)
                weather_icon_index = map_weather_icon(icon_today);

            // Get main weather data
            char *temp = get_value_from_JSON_string(wifi_http_buffer, "temp", value_buffer, sizeof(value_buffer), NULL);
            if (strcmp(temp, "-") != 0)
                weather_temp = strtod(temp, NULL);

            char *humidity = get_value_from_JSON_string(wifi_http_buffer, "humidity", value_buffer, sizeof(value_buffer), NULL);
            if (strcmp(humidity, "-") != 0)
                weather_humidity = strtod(humidity, NULL);

            // Get sunrise/sunset from sys
            char *sunrise_str = get_value_from_JSON_string(wifi_http_buffer, "sunrise", value_buffer, sizeof(value_buffer), NULL);
            if (strcmp(sunrise_str, "-") != 0)
                sunrise = (time_t)strtoll(sunrise_str, NULL, 10);

            char *sunset_str = get_value_from_JSON_string(wifi_http_buffer, "sunset", value_buffer, sizeof(value_buffer), NULL);
            if (strcmp(sunset_str, "-") != 0)
                sunset = (time_t)strtoll(sunset_str, NULL, 10);

            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Weather data: Icon %s (index %d), Temp: %.1f°C, Humidity: %.1f%%",
                        icon_today, weather_icon_index, weather_temp, weather_humidity);
            time(&last_weather_update);
            weather_valid = true;
            weather_has_updated = true;
            result = true;
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request failed, Status Code: %d", status_code);
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request error: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);

    // Get forecast
    snprintf(weather_url, sizeof(weather_url),
             "http://api.openweathermap.org/data/2.5/forecast?lat=%s&lon=%s&appid=%s&units=metric&cnt=8",
             my_lat, my_lon, WEATHER_API_KEY);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Fetching Forecast from: %.10s...", weather_url);

    // Reset forecast tracking variables
    is_forecast_request = true;
    forecast_high = -100.0;
    forecast_low = 100.0;
    forecast_temp_count = 0;

    // Use a larger buffer for forecast data
    config.buffer_size = HTTP_BUFFER_SIZE;
    config.buffer_size_tx = HTTP_BUFFER_SIZE;
    client = esp_http_client_init(&config);
    err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Forecast data processed successfully");
            weather_high = forecast_high;
            weather_low = forecast_low;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Found %d temperature readings, High: %.1f°C, Low: %.1f°C", 
                forecast_temp_count, weather_high, weather_low);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request failed for Forecast, Status Code: %d", status_code);
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request error for Forecast: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    is_forecast_request = false;

    // Clean up the buffer after both requests are done
    if (wifi_http_buffer != NULL)
    {
        release_shared_buffer(wifi_http_buffer);
        wifi_http_buffer = NULL;
    }
    response_len = 0;

    return result;
}

void str_replace_char(char *str, char find, char replace) {
    char *ptr;
    while ((ptr = strchr(str, find)) != NULL) {
        *ptr = replace;
    }
}
// Function to fetch geolocation data
bool wifi_get_location()
{
    bool result = false;
    char internet_location[64] = "";

    strcpy(my_timezone, "");
    strcpy(my_lat, "");
    strcpy(my_lon, "");

    // use eeprom defaults, if any
    if (strlen(eeprom_timezone))
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Using EEPROM timezone: %s", eeprom_timezone);
        strcpy(my_timezone, eeprom_timezone);
    }

    if (strlen(eeprom_lat))
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Using EEPROM latitude: %s", eeprom_lat);
        // Validate before using
        if (validate_coordinate(eeprom_lat, true))
        {
            strcpy(my_lat, eeprom_lat);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid latitude in EEPROM: '%s', clearing", eeprom_lat);
            strcpy(my_lat, "");
            // Clear invalid value from eeprom_lat to prevent reuse
            strcpy(eeprom_lat, "");
        }
    }

    if (strlen(eeprom_lon))
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Using EEPROM longitude: %s", eeprom_lon);
        // Validate before using
        if (validate_coordinate(eeprom_lon, false))
        {
            strcpy(my_lon, eeprom_lon);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid longitude in EEPROM: '%s', clearing", eeprom_lon);
            strcpy(my_lon, "");
            // Clear invalid value from eeprom_lon to prevent reuse
            strcpy(eeprom_lon, "");
        }
    }

    // Only perform query if needed (at least one of the values is missing)
    if (strlen(my_timezone) == 0 || strlen(my_lat) == 0 || strlen(my_lon) == 0)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No location data, IP reverse lookup");
        esp_http_client_config_t config = {
            .url = "http://ip-api.com/json/?fields=country,city,lat,lon,timezone,query",
            .event_handler = http_event_handler,
            .timeout_ms = 5000,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
            return false;
        }

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK)
        {
            int status_code = esp_http_client_get_status_code(client);
            if (status_code == 200 && wifi_http_buffer != NULL)
            {
                // Parse JSON response
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "HTTP response %s", wifi_http_buffer);
                cJSON *json = cJSON_Parse(wifi_http_buffer);
                if (json)
                {
                    cJSON *query = cJSON_GetObjectItem(json, "query");
                    cJSON *city_obj = cJSON_GetObjectItem(json, "city");
                    cJSON *country_obj = cJSON_GetObjectItem(json, "country");
                    cJSON *lat = cJSON_GetObjectItem(json, "lat");
                    cJSON *lon = cJSON_GetObjectItem(json, "lon");
                    cJSON *timezone = cJSON_GetObjectItem(json, "timezone");

                    if (query)
                        strcpy(my_ip, query->valuestring);
                    if (city_obj)
                        strcpy(city, city_obj->valuestring);
                    if (country_obj)
                        strcpy(country, country_obj->valuestring);
                    if (lat && strlen(my_lat) == 0)
                    {
                        dlat = lat->valuedouble;
                        sprintf(my_lat, "%2.7lf", dlat);
                        // Validate the coordinate from API
                        if (!validate_coordinate(my_lat, true))
                        {
                            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid latitude from API: %.7f, clearing", dlat);
                            strcpy(my_lat, "");
                        }
                    }
                    if (lon && strlen(my_lon) == 0)
                    {
                        dlon = lon->valuedouble;
                        sprintf(my_lon, "%2.7lf", dlon);
                        // Validate the coordinate from API
                        if (!validate_coordinate(my_lon, false))
                        {
                            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid longitude from API: %.7f, clearing", dlon);
                            strcpy(my_lon, "");
                        }
                    }
                    if (timezone && strlen(my_timezone) == 0)
                        strcpy(internet_location, timezone->valuestring); // this is e.g. Europe/Vienna

                    cJSON_Delete(json);
                }
                else
                {
                    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to parse JSON response");
                }
            }
            else
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request failed, Status Code: %d", status_code);
            }
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "HTTP request error: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
    }

    // Read timezone file
    if (strlen(internet_location) > 0)
    {
        str_replace_char(internet_location, ' ', '_');
        FILE *file = fopen("/spiffs/timezone.txt", "r");
        if (file)
        {
            char line[64];
            while (fgets(line, sizeof(line), file))
            {
                strtok(line, "\r\n"); // Remove newlines
                // replace ' ' with '_' in line
                str_replace_char(line, ' ', '_');

                // Split the line at the ; between location and timezone
                char *loc_part = strtok(line, ";");
                char *tz_part = strtok(NULL, ";");

                if (loc_part && tz_part)
                {
                    // ESP_LOG_WEB(ESP_LOG_INFO,TAG, "Checking location: %s, timezone: %s", loc_part, tz_part);
                    if (strcasecmp(loc_part, internet_location) == 0)
                    {
                        strncpy(my_timezone, tz_part, TZ_LENGTH);
                        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Found timezone: %s for location: %s", my_timezone, internet_location);
                        break;
                    }
                }
            }
            fclose(file);
        }
        // if no timezone found, ask http://update.artlogic.gr:6868/timezone?location=<internet_location>.
        // the response is a simple POSIX timezone string, plain format
        // if found, save to /spiffs/timezone.txt and try again
        // if not found, set timezone to UTC
        if (strlen(my_timezone) == 0 && strlen(internet_location) > 0)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No timezone found, asking update server for timezone");
            char url[128];
            // HTML escape the internet_location
            char escaped_location[64];
            url_encode_string(internet_location, escaped_location);
            snprintf(url, sizeof(url), "%s/timezone?location=%s", UPDATE_SERVER_BASE, escaped_location);
            esp_http_client_config_t config = {
                .url = url,
                .event_handler = http_event_handler,
                .timeout_ms = 5000,
            };

            esp_http_client_handle_t client = esp_http_client_init(&config);
            if (client == NULL)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
                return false;
            }

            esp_err_t err = esp_http_client_perform(client);
            if (err == ESP_OK && wifi_http_buffer != NULL)
            {
                ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Timezone found: %s is %s", internet_location, wifi_http_buffer);
                strncpy(my_timezone, wifi_http_buffer, TZ_LENGTH);
                // add new entry to /spiffs/timezone.txt

                // save it if we got a valid timezone
                if (strlen(my_timezone) > 0)
                {
                    file = fopen("/spiffs/timezone.txt", "a");
                    if (file)
                    {
                        fprintf(file, "\n%s;%s", internet_location, my_timezone);
                        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Timezone saved to /spiffs/timezone.txt");
                        fclose(file);
                    }
                }
            }
            esp_http_client_cleanup(client);
        }
    }
    else
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Missing timezone.txt file");

    // if all else fails, set timezone to UTC
    if (strlen(my_timezone) == 0)
        strcpy(my_timezone, "UTC");

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "External IP: %s", my_ip);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Location: %s/%s at %s, %s, TZ: %s", city, country, my_lat, my_lon, my_timezone);

    // timezone should be more than UTC
    result = (strlen(my_timezone) > 3) || (strlen(my_lat) > 0) || (strlen(my_lon) > 0);

    return result;
}

// HTTP POST utility function
esp_err_t f_http_post(const char *url, const char *data)
{
    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 5000,
        .method = HTTP_METHOD_POST,
        .buffer_size = 1024,
        .buffer_size_tx = 1024};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int content_length = strlen(data);
    int write_len = esp_http_client_write(client, data, content_length);
    if (write_len < 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to write POST data");
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    err = esp_http_client_perform(client);
    esp_http_client_cleanup(client);

    return err;
}
