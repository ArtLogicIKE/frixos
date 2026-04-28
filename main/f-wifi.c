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
#include <strings.h>
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

uint64_t weather_delay_ms = 30 * 60 * 1000; // 30 minutes — twice an hour, polite for met.no
esp_timer_handle_t location_timer;
uint64_t location_delay_ms = 100; // Start with 0.1 seconds

#define UPDATE_SERVER_BASE "http://update.artlogic.gr:8080"

// Add these global variables at the top with other globals
static bool is_forecast_request = false;
static double forecast_high = -100.0;
static double forecast_low = 100.0;
static int forecast_temp_count = 0;

// Met.no streaming-parser state. We never hold the whole response in memory:
// instead we feed bytes into the shared 4 KB wifi_http_buffer, and as soon as
// the buffer would overflow we process every fully-contained timeseries entry
// (each begins with the marker {"time":"...") and shift the trailing partial
// entry back to offset 0. See metno_process_buffer().
static bool   is_metno_request = false;
static int    metno_entry_idx = 0;
static int    metno_today_yday = 0;
static int    metno_today_year = 0;
static double metno_day_hi[3] = { -1000.0, -1000.0, -1000.0 };
static double metno_day_lo[3] = {  1000.0,  1000.0,  1000.0 };
static char   metno_pending_lm[64] = {0};   // Last-Modified seen during in-flight request
static char   metno_last_modified[64] = {0};// Last-Modified saved from previous successful response
static int    metno_last_sunrise_yday = -1;
static int    metno_last_sunrise_year = -1;

// NTP server functionality removed - using default servers only

// Function prototypes
void ota_update_timer_callback(void *arg);
void mdns_announcement_timer_callback(void *arg);
static void wifi_active_hours_timer_callback(void *arg);
static bool is_wifi_active_hours(void);
static void shutdown_mdns_on_link_loss(void);
static void copy_mdns_hostname_label(char *dst, size_t dst_sz);

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

// Single mDNS host label for mdns_hostname_set (no ".local" suffix; see ESP-IDF mDNS docs)
static void copy_mdns_hostname_label(char *dst, size_t dst_sz)
{
    if (dst_sz == 0)
        return;
    strncpy(dst, eeprom_hostname, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
    size_t n = strlen(dst);
    if (n >= 6 && strcasecmp(dst + n - 6, ".local") == 0)
        dst[n - 6] = '\0';
}

// Stop mDNS when STA loses IP or disconnects so clients do not cache stale records / broken stack state
static void shutdown_mdns_on_link_loss(void)
{
    if (mdns_announcement_timer != NULL)
        esp_timer_stop(mdns_announcement_timer);

    if (mdns_initialized)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Stopping mDNS (link down or IP lost)");
        mdns_free();
        mdns_initialized = false;
    }
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

    char mdns_host[sizeof(eeprom_hostname)];
    copy_mdns_hostname_label(mdns_host, sizeof(mdns_host));
    if (mdns_host[0] == '\0')
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "mDNS: empty hostname, skipping init");
        return;
    }

    // Initialize mDNS
    esp_err_t err = mdns_init();
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to initialize mDNS: %s", esp_err_to_name(err));
        return;
    }

    // Set hostname
    err = mdns_hostname_set(mdns_host);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set mDNS hostname: %s", esp_err_to_name(err));
        mdns_free();
        return;
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "mDNS hostname set to: %s", mdns_host);
    }

    // Set default instance
    err = mdns_instance_name_set("Frixos Configuration Web Server");
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to set mDNS instance name: %s", esp_err_to_name(err));
        mdns_free();
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

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "mDNS initialized. Hostname: %s.local", mdns_host);
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

            shutdown_mdns_on_link_loss();

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
        if (event_id == IP_EVENT_STA_LOST_IP)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "STA lost IP, stopping mDNS");
            shutdown_mdns_on_link_loss();
        }
        else if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            snprintf(internal_ip, sizeof(internal_ip), IPSTR, IP2STR(&event->ip_info.ip));
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            retry_count = 0;
            if (manufacturer_mode) // if in manufacturer mode, use custom device name
            {
                uint8_t mac[6];
                esp_efuse_mac_get_default(mac);
                snprintf(eeprom_hostname, sizeof(eeprom_hostname), "frixos-%02X%02X%02X",
                         mac[3], mac[4], mac[5]);
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
    wifi_get_metno_weather();

    // we might as well calculate the moon phase too
    // we do it here, with every weather, but also when NTP is synchronized
    moon_icon_index = get_moon_index();
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Moon phase: %d", moon_icon_index);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Weather next in %.1f minutes.", (weather_delay_ms / (1000 * 60.0)));
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
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, &wifi_event_handler, NULL));

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

// Forward declaration — defined in the met.no section below.
static int metno_process_buffer(char *buf, int buf_used, bool final);

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ON_HEADER:
        if (is_metno_request && evt->header_key && evt->header_value &&
            strcasecmp(evt->header_key, "Last-Modified") == 0)
        {
            strlcpy(metno_pending_lm, evt->header_value, sizeof(metno_pending_lm));
        }
        break;
    case HTTP_EVENT_ERROR:
    case HTTP_EVENT_REDIRECT:
    case HTTP_EVENT_ON_CONNECTED:
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

            int incoming = evt->data_len;
            const char *src = (const char *)evt->data;

            if (is_metno_request)
            {
                // Streaming met.no: consume the incoming chunk in pieces, draining
                // the buffer whenever it is too full to accept more bytes.
                while (incoming > 0)
                {
                    int slack = (HTTP_BUFFER_SIZE - 1) - response_len;
                    if (slack <= 0)
                    {
                        // Buffer full. Process every complete entry and shift
                        // the partial trailing entry to offset 0.
                        int kept = metno_process_buffer(wifi_http_buffer, response_len, false);
                        if (kept <= 0 || kept >= response_len)
                        {
                            // No marker found in 4 KB → entry too big. Drop and resync.
                            ESP_LOG_WEB(ESP_LOG_ERROR, TAG,
                                        "Met.no entry > %d bytes, dropping buffer", HTTP_BUFFER_SIZE);
                            response_len = 0;
                        }
                        else
                        {
                            int tail = response_len - kept;
                            memmove(wifi_http_buffer, wifi_http_buffer + kept, tail);
                            response_len = tail;
                            wifi_http_buffer[response_len] = '\0';
                        }
                        slack = (HTTP_BUFFER_SIZE - 1) - response_len;
                        if (slack <= 0) { response_len = 0; slack = HTTP_BUFFER_SIZE - 1; }
                    }
                    int take = (incoming < slack) ? incoming : slack;
                    memcpy(wifi_http_buffer + response_len, src, take);
                    response_len += take;
                    wifi_http_buffer[response_len] = '\0';
                    src += take;
                    incoming -= take;
                }
            }
            else if ((response_len + incoming) < HTTP_BUFFER_SIZE)
            {
                memcpy(wifi_http_buffer + response_len, src, incoming);
                response_len += incoming;
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
 * Validate POSIX TZ string before passing to setenv/tzset/localtime.
 * Invalid strings (e.g. "GMT +2" with space) can crash newlib on ESP-IDF.
 * POSIX format: std offset[dst...] - no spaces, std 3+ chars, offset [+-]hh[:mm[:ss]]
 * Returns true if string is safe to use.
 */
bool validate_timezone(const char *tz_str)
{
    if (tz_str == NULL)
        return false;
    /* Empty is valid - caller will use UTC fallback */
    if (tz_str[0] == '\0')
        return true;

    for (size_t i = 0; tz_str[i] != '\0'; i++)
    {
        char c = tz_str[i];
        /* Reject spaces - main cause of tzset/localtime crashes */
        if (c == ' ' || c == '\t')
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone: spaces not allowed in POSIX TZ (e.g. use GMT+2 not GMT +2)");
            return false;
        }
        /* Reject control chars */
        if ((unsigned char)c < 0x20)
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone: control character 0x%02x", (unsigned char)c);
            return false;
        }
        /* Valid POSIX TZ chars: letters, digits, + - : , . / < > */
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
              c == '+' || c == '-' || c == ':' || c == ',' || c == '.' || c == '/' || c == '<' || c == '>'))
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone: illegal character '%c' (0x%02x)", c, (unsigned char)c);
            return false;
        }
    }
    return true;
}

/**
 * Fetch Weather Data from OpenWeatherMap (legacy, kept for reference / fallback).
 * No longer wired into the weather timer — wifi_get_metno_weather() is used instead.
 */
bool wifi_get_openweather(void)
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

// =====================================================================
// Met.no Locationforecast 2.0 + Sunrise 3.0 integration
// =====================================================================
// No API key. Mandatory unique User-Agent (WEATHER_USER_AGENT). HTTPS only;
// TLS verification is skipped via custom_crt_bundle_attach (same as Dexcom/
// Freestyle/Nightscout). Conditional GET via Last-Modified is honored: a 304
// keeps the previously parsed values valid.
//
// The "complete" payload is far larger than the 4 KB shared buffer, so we
// stream-parse: bytes are appended to wifi_http_buffer; whenever it would
// overflow we run metno_process_buffer() which finds every fully-contained
// timeseries entry (each starts with {"time":"...) and processes it via
// process_one_metno_entry(), then shifts the partial trailing entry back to
// offset 0. Each entry fits in 4 KB; we never hold the full document.
// All streaming state (entry index, day high/low buckets, Last-Modified) lives
// in static globals declared near the top of this file alongside the existing
// is_forecast_request flag.

// Compute UTC time_t from individual UTC date/time components without relying
// on timegm() (not exposed by ESP-IDF newlib).
static time_t metno_make_utc_time(int year, int mon, int mday, int hh, int mm, int ss)
{
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long days = 0;
    for (int y = 1970; y < year; y++)
    {
        bool leap = (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0));
        days += leap ? 366 : 365;
    }
    for (int m = 1; m < mon; m++)
    {
        days += dim[m - 1];
        if (m == 2)
        {
            bool leap = (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
            if (leap) days += 1;
        }
    }
    days += mday - 1;
    return (time_t)((long long)days * 86400LL + (long long)hh * 3600 + (long long)mm * 60 + ss);
}

// Parse an ISO-8601 timestamp. Accepts "...Z" and "...±HH:MM" forms, with or
// without seconds. Returns 0 on parse failure.
static time_t metno_parse_iso8601(const char *s)
{
    if (!s || !*s) return 0;
    int year, mon, mday, hh, mm, ss = 0;
    char tz_sign = 0;
    int  off_h = 0, off_m = 0;

    // Pattern 1: with seconds, "Z"
    if (sscanf(s, "%d-%d-%dT%d:%d:%dZ", &year, &mon, &mday, &hh, &mm, &ss) == 6)
        return metno_make_utc_time(year, mon, mday, hh, mm, ss);
    // Pattern 2: with seconds and offset
    if (sscanf(s, "%d-%d-%dT%d:%d:%d%c%d:%d",
               &year, &mon, &mday, &hh, &mm, &ss, &tz_sign, &off_h, &off_m) == 9 &&
        (tz_sign == '+' || tz_sign == '-'))
    {
        time_t utc_naive = metno_make_utc_time(year, mon, mday, hh, mm, ss);
        long off = (long)off_h * 3600 + (long)off_m * 60;
        return (tz_sign == '+') ? (utc_naive - off) : (utc_naive + off);
    }
    // Pattern 3: no seconds, "Z"
    if (sscanf(s, "%d-%d-%dT%d:%dZ", &year, &mon, &mday, &hh, &mm) == 5)
        return metno_make_utc_time(year, mon, mday, hh, mm, 0);
    // Pattern 4: no seconds, with offset
    if (sscanf(s, "%d-%d-%dT%d:%d%c%d:%d",
               &year, &mon, &mday, &hh, &mm, &tz_sign, &off_h, &off_m) == 8 &&
        (tz_sign == '+' || tz_sign == '-'))
    {
        time_t utc_naive = metno_make_utc_time(year, mon, mday, hh, mm, 0);
        long off = (long)off_h * 3600 + (long)off_m * 60;
        return (tz_sign == '+') ? (utc_naive - off) : (utc_naive + off);
    }
    return 0;
}

// Map a met.no symbol_code (e.g. "partlycloudy_day", "lightrainshowers_night",
// "heavysnowandthunder") to the existing 0-7 icon-sprite index used by
// f-display.c. Order matters: thunder dominates snow/sleet which dominates
// rain, so the most specific weather appears.
int map_metno_symbol(const char *symbol)
{
    if (!symbol || !*symbol) return -1;
    if (strstr(symbol, "thunder") != NULL) return 5;
    if (strstr(symbol, "snow") != NULL || strstr(symbol, "sleet") != NULL) return 6;
    if (strstr(symbol, "rain") != NULL) return 4;
    if (strncmp(symbol, "fog", 3) == 0) return 7;
    if (strncmp(symbol, "cloudy", 6) == 0) return 3;
    if (strncmp(symbol, "partlycloudy", 12) == 0) return 2;
    if (strncmp(symbol, "fair", 4) == 0) return 1;
    if (strncmp(symbol, "clearsky", 8) == 0) return 0;
    return -1;
}

// Bucket a parsed timeseries time_t into one of three day slots: 0=today,
// 1=tomorrow, 2=day-after, all in local time. Returns -1 if outside that
// window (so we ignore later forecast entries when computing day high/low).
static int metno_day_bucket(time_t et)
{
    if (et <= 0) return -1;
    struct tm el;
    localtime_r(&et, &el);
    if (el.tm_year == metno_today_year && el.tm_yday == metno_today_yday) return 0;
    if (el.tm_year == metno_today_year && el.tm_yday == metno_today_yday + 1) return 1;
    if (el.tm_year == metno_today_year && el.tm_yday == metno_today_yday + 2) return 2;
    // Year-end wrap-around
    if (el.tm_year == metno_today_year + 1 && metno_today_yday >= 364 && el.tm_yday == 0) return 1;
    if (el.tm_year == metno_today_year + 1 && metno_today_yday >= 363 && el.tm_yday <= 1) return 2;
    return -1;
}

// Pull a fresh value out of a single met.no timeseries entry's text. The
// JSON parser uses strstr("\"key\""), so a partial-name key like "wind_speed"
// will not match "wind_speed_of_gust" — substrings are guarded by the quotes.
// idx is the entry's 0-based position in the timeseries array.
static void process_one_metno_entry(const char *entry, int idx)
{
    char val[64];

    // time → day bucket (used for high/low across today + 2 days)
    get_value_from_JSON_string(entry, "time", val, sizeof(val), NULL);
    time_t et = (val[0] && strcmp(val, "-") != 0) ? metno_parse_iso8601(val) : 0;
    int day = metno_day_bucket(et);

    // air_temperature → bucket high/low; first entry also drives weather_temp
    get_value_from_JSON_string(entry, "air_temperature", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0)
    {
        double t = strtod(val, NULL);
        if (day >= 0)
        {
            if (t > metno_day_hi[day]) metno_day_hi[day] = t;
            if (t < metno_day_lo[day]) metno_day_lo[day] = t;
        }
        if (idx == 0) weather_temp = t;
    }

    // First entry also carries the current humidity / wind / pressure / icon
    // / precipitation / UV. Later entries are temperature-only for high/low.
    if (idx != 0) return;

    get_value_from_JSON_string(entry, "relative_humidity", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0) weather_humidity = strtod(val, NULL);

    get_value_from_JSON_string(entry, "wind_speed", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0) weather_wind_speed_mps = strtod(val, NULL);

    get_value_from_JSON_string(entry, "wind_speed_of_gust", val, sizeof(val), NULL);
    weather_gust_mps = (strcmp(val, "-") != 0) ? strtod(val, NULL) : 0.0;

    get_value_from_JSON_string(entry, "wind_from_direction", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0)
    {
        int d = (int)(strtod(val, NULL) + 0.5);
        d %= 360;
        if (d < 0) d += 360;
        weather_wind_dir_deg = d;
    }

    get_value_from_JSON_string(entry, "air_pressure_at_sea_level", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0)
    {
        double new_p = strtod(val, NULL);
        if (weather_pressure_prev_hpa > 0.0)
        {
            double diff = new_p - weather_pressure_prev_hpa;
            if      (diff >  0.5) weather_pressure_trend =  1;
            else if (diff < -0.5) weather_pressure_trend = -1;
            else                  weather_pressure_trend =  0;
        }
        weather_pressure_prev_hpa = (weather_pressure_hpa > 0.0) ? weather_pressure_hpa : new_p;
        weather_pressure_hpa = new_p;
    }

    get_value_from_JSON_string(entry, "symbol_code", val, sizeof(val), NULL);
    if (strcmp(val, "-") != 0)
    {
        int sidx = map_metno_symbol(val);
        if (sidx >= 0)
        {
            weather_icon_index = sidx;
            strlcpy(icon_today, val, sizeof(icon_today));
        }
    }

    get_value_from_JSON_string(entry, "precipitation_amount", val, sizeof(val), NULL);
    weather_precip_mm = (strcmp(val, "-") != 0) ? strtod(val, NULL) : 0.0;

    get_value_from_JSON_string(entry, "probability_of_precipitation", val, sizeof(val), NULL);
    weather_precip_prob = (strcmp(val, "-") != 0) ? strtod(val, NULL) : 0.0;

    get_value_from_JSON_string(entry, "ultraviolet_index_clear_sky_max", val, sizeof(val), NULL);
    weather_uv = (strcmp(val, "-") != 0) ? strtod(val, NULL) : 0.0;
}

// Drain wifi_http_buffer up to the last fully-contained timeseries entry.
// Returns the offset of the partial trailing entry (its starting marker)
// which the caller must keep for the next chunk. If `final` is true, the
// trailing entry is treated as complete (last entry of the response).
// Returns buf_used if the whole buffer was consumed.
static int metno_process_buffer(char *buf, int buf_used, bool final)
{
    static const char marker[] = "{\"time\":\"";
    static const int  marker_len = 9;

    if (buf_used <= 0) return buf_used;
    buf[buf_used] = '\0';  // strstr needs a NUL; caller leaves one byte slack

    int pos = 0;
    while (pos < buf_used)
    {
        char *m = strstr(buf + pos, marker);
        if (!m) return buf_used;            // no markers anywhere → keep nothing? actually drop everything before
        int m_off = m - buf;
        char *next = strstr(m + marker_len, marker);
        int entry_end;
        if (next)
        {
            entry_end = (int)(next - buf);
        }
        else if (final)
        {
            entry_end = buf_used;
        }
        else
        {
            // No further marker — this entry is still being received.
            return m_off;
        }
        char saved = buf[entry_end];
        buf[entry_end] = '\0';
        process_one_metno_entry(buf + m_off, metno_entry_idx);
        buf[entry_end] = saved;
        metno_entry_idx++;
        pos = entry_end;
    }
    return buf_used;
}

/**
 * Fetch sunrise/sunset for the device coordinates from
 * api.met.no/weatherapi/sunrise/3.0/sun. The response is tiny (~250 bytes)
 * so it fits comfortably in the shared 4 KB wifi_http_buffer with no
 * streaming. Stores results in `sunrise`/`sunset` as Unix UTC time_t
 * (display layer uses localtime_r). Called at startup and once per local
 * calendar day from wifi_get_metno_weather().
 */
bool wifi_get_metno_sunrise(void)
{
    if (!validate_coordinate(my_lat, true) || !validate_coordinate(my_lon, false))
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Sunrise: invalid coordinates, skipping");
        return false;
    }

    time_t now = 0;
    time(&now);
    struct tm now_local;
    localtime_r(&now, &now_local);

    // YYYY-MM-DD + NUL
    char date_str[11];
    if (strftime(date_str, sizeof(date_str), "%Y-%m-%d", &now_local) == 0)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Sunrise: failed to format local date");
        return false;
    }

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.met.no/weatherapi/sunrise/3.0/sun?lat=%s&lon=%s&date=%s",
             my_lat, my_lon, date_str);

    // Buffered (non-streaming) — uses the regular branch of http_event_handler
    // which fills wifi_http_buffer up to HTTP_BUFFER_SIZE.
    is_metno_request = false;
    is_forecast_request = false;
    if (wifi_http_buffer != NULL)
    {
        release_shared_buffer(wifi_http_buffer);
        wifi_http_buffer = NULL;
    }
    response_len = 0;

    // Serialize TLS connections with the rest of the integrations so the
    // mbedTLS record buffers (~4 KB in + ~4 KB out) are never duplicated.
    if (!acquire_ssl_semaphore("wifi_get_metno_sunrise"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Sunrise: SSL lock failed");
        return false;
    }

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 8000,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_agent = WEATHER_USER_AGENT,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_header(client, "Connection", "close");

    bool ok = false;
    esp_err_t err = esp_http_client_perform(client);
    int status = (err == ESP_OK) ? esp_http_client_get_status_code(client) : -1;
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Sunrise HTTP %d, %d bytes", status, response_len);

    if (err == ESP_OK && status == 200 && wifi_http_buffer && response_len > 0)
    {
        char val[40];
        // Grab the "sunrise":{"time":"..."} block then read its "time".
        char *blk_end = NULL;
        get_value_from_JSON_string(wifi_http_buffer, "sunrise", val, sizeof(val), &blk_end);
        // The above returns the {} block contents but our buffer still contains
        // the raw text. Easier: pull the two "time":"..." occurrences directly
        // — there are exactly two of them in the response, in order: sunrise
        // then sunset.
        char *rem = NULL;
        get_value_from_JSON_string(wifi_http_buffer, "time", val, sizeof(val), &rem);
        time_t r = (val[0] && strcmp(val, "-") != 0) ? metno_parse_iso8601(val) : 0;
        time_t s = 0;
        if (rem)
        {
            get_value_from_JSON_string(rem, "time", val, sizeof(val), NULL);
            if (val[0] && strcmp(val, "-") != 0) s = metno_parse_iso8601(val);
        }
        if (r > 0 && s > 0)
        {
            sunrise = r;
            sunset  = s;
            metno_last_sunrise_yday = now_local.tm_yday;
            metno_last_sunrise_year = now_local.tm_year;
            ok = true;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Sunrise %ld, sunset %ld (date %s)",
                        (long)sunrise, (long)sunset, date_str);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Sunrise: parse failed (r=%ld s=%ld)",
                        (long)r, (long)s);
        }
    }
    else if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Sunrise HTTP error: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    if (wifi_http_buffer != NULL)
    {
        release_shared_buffer(wifi_http_buffer);
        wifi_http_buffer = NULL;
    }
    response_len = 0;
    release_ssl_semaphore();
    return ok;
}

/**
 * Fetch current weather + 3-day forecast from met.no Locationforecast 2.0
 * /complete using a streaming parser over the existing 4 KB shared buffer.
 *
 * Approach: bytes are accumulated into wifi_http_buffer; whenever it fills
 * we call metno_process_buffer() which scans for {"time":"..." entry markers,
 * processes every fully-contained timeseries entry via process_one_metno_entry
 * (which extracts current values from entry 0 and feeds today / +1 / +2 day
 * temperature buckets for high/low from later entries), then shifts the
 * partial trailing entry back to offset 0.
 *
 * Conditional GET: an If-Modified-Since header is sent on every request; on
 * 304 we keep the previously parsed values and just update last_weather_update.
 *
 * Also re-fetches sunrise/sunset on the first call after local-time midnight.
 */
bool wifi_get_metno_weather(void)
{
    if (!validate_coordinate(my_lat, true) || !validate_coordinate(my_lon, false))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Met.no: invalid coordinates '%s','%s'", my_lat, my_lon);
        return false;
    }

    // Snapshot today's local-day index for the streaming parser to bucket against.
    time_t now_t = 0;
    time(&now_t);
    struct tm tnow;
    localtime_r(&now_t, &tnow);
    metno_today_yday = tnow.tm_yday;
    metno_today_year = tnow.tm_year;

    // Refresh sunrise/sunset on the first call of a new local day (or first run).
    // After the sunrise TLS connection is torn down, give mbedTLS / lwIP a brief
    // moment to fully release socket and TLS state before we open another TLS
    // connection to the same host. Without this, back-to-back calls have been
    // observed to return MBEDTLS_ERR_SSL_BAD_INPUT_DATA (-0x7100) on the second
    // call's response read.
    if (metno_last_sunrise_yday < 0 ||
        metno_last_sunrise_year != tnow.tm_year ||
        metno_last_sunrise_yday != tnow.tm_yday)
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "New local day or first run; fetching sunrise");
        wifi_get_metno_sunrise();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    char url[256];
    // Diagnostic B: use the /compact endpoint instead of /complete. Compact has
    // a smaller payload but does NOT include wind_speed_of_gust,
    // ultraviolet_index_clear_sky_max, or probability_of_precipitation, so the
    // [gust], [uv], and probability portion of [precip] tokens will read 0.
    snprintf(url, sizeof(url),
             "https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=%s&lon=%s",
             my_lat, my_lon);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Fetching met.no weather: %.50s...", url);

    // Reset streaming state. Day buckets use sentinels that the final
    // assignment below converts to skip-if-empty.
    metno_entry_idx = 0;
    for (int i = 0; i < 3; i++) { metno_day_hi[i] = -1000.0; metno_day_lo[i] = 1000.0; }
    metno_pending_lm[0] = '\0';

    if (wifi_http_buffer != NULL)
    {
        release_shared_buffer(wifi_http_buffer);
        wifi_http_buffer = NULL;
    }
    response_len = 0;
    is_metno_request = true;
    is_forecast_request = false;

    // Serialize TLS connections so mbedTLS record buffers stay singleton.
    if (!acquire_ssl_semaphore("wifi_get_metno_weather"))
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Met.no: SSL lock failed");
        is_metno_request = false;
        return false;
    }

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
        .crt_bundle_attach = custom_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .user_agent = WEATHER_USER_AGENT,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Accept", "application/json");
    esp_http_client_set_header(client, "Connection", "close");
    if (metno_last_modified[0] != '\0')
        esp_http_client_set_header(client, "If-Modified-Since", metno_last_modified);

    bool result = false;
    esp_err_t err = esp_http_client_perform(client);
    int status = (err == ESP_OK) ? esp_http_client_get_status_code(client) : -1;
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Met.no status %d, parsed %d entries, lm='%s'",
                status, metno_entry_idx, metno_pending_lm);

    if (err == ESP_OK && status == 304)
    {
        // Not modified — existing parsed values are still valid.
        time(&last_weather_update);
        weather_valid = true;
        result = true;
    }
    else if (err == ESP_OK && status == 200)
    {
        // Drain the trailing partial entry as the final entry.
        if (wifi_http_buffer && response_len > 0)
        {
            metno_process_buffer(wifi_http_buffer, response_len, true);
        }

        if (metno_entry_idx > 0)
        {
            // Today's high/low and 3-day high/low from the bucketed instants.
            if (metno_day_hi[0] > -999.0)
            {
                weather_high = metno_day_hi[0];
                weather_low  = metno_day_lo[0];
            }
            double h3 = metno_day_hi[0], l3 = metno_day_lo[0];
            for (int i = 1; i < 3; i++)
            {
                if (metno_day_hi[i] > -999.0 && metno_day_hi[i] > h3) h3 = metno_day_hi[i];
                if (metno_day_lo[i] <  999.0 && metno_day_lo[i] < l3) l3 = metno_day_lo[i];
            }
            if (h3 > -999.0)
            {
                weather_3day_high = h3;
                weather_3day_low  = l3;
            }

            ESP_LOG_WEB(ESP_LOG_INFO, TAG,
                        "Met.no: T=%.1f°C H=%.0f%% W=%.1fm/s@%d° G=%.1f P=%.0fhPa(t%d) PR=%.1fmm/%.0f%% UV=%.1f Hi/Lo=%.1f/%.1f 3d=%.1f/%.1f",
                        weather_temp, weather_humidity,
                        weather_wind_speed_mps, weather_wind_dir_deg,
                        weather_gust_mps,
                        weather_pressure_hpa, weather_pressure_trend,
                        weather_precip_mm, weather_precip_prob, weather_uv,
                        weather_high, weather_low,
                        weather_3day_high, weather_3day_low);

            time(&last_weather_update);
            weather_valid = true;
            weather_has_updated = true;
            result = true;
            if (metno_pending_lm[0] != '\0')
                strlcpy(metno_last_modified, metno_pending_lm, sizeof(metno_last_modified));
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Met.no: no entries parsed from stream");
        }
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Met.no fetch failed: err=%s status=%d",
                    esp_err_to_name(err), status);
    }

    is_metno_request = false;
    esp_http_client_cleanup(client);
    if (wifi_http_buffer != NULL)
    {
        release_shared_buffer(wifi_http_buffer);
        wifi_http_buffer = NULL;
    }
    response_len = 0;
    release_ssl_semaphore();
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
        if (validate_timezone(eeprom_timezone))
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Using EEPROM timezone: %s", eeprom_timezone);
            strcpy(my_timezone, eeprom_timezone);
        }
        else
        {
            ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone in EEPROM: '%s', clearing", eeprom_timezone);
            strcpy(eeprom_timezone, "");
        }
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
                /* Trim trailing newline from response */
                char *nl = strchr(wifi_http_buffer, '\n');
                if (nl) *nl = '\0';
                if (validate_timezone(wifi_http_buffer))
                {
                    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Timezone found: %s is %s", internet_location, wifi_http_buffer);
                    strncpy(my_timezone, wifi_http_buffer, TZ_LENGTH);
                    file = fopen("/spiffs/timezone.txt", "a");
                    if (file)
                    {
                        fprintf(file, "\n%s;%s", internet_location, my_timezone);
                        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Timezone saved to /spiffs/timezone.txt");
                        fclose(file);
                    }
                }
                else
                    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid timezone from server: '%s', ignoring", wifi_http_buffer);
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
