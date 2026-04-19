#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <dirent.h>
#include "time.h"
#include <stdarg.h>    // Add this for va_list, vsnprintf
#include <sys/param.h> // Add this for min/max macros

// Define min macro if not already defined
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

#include "frixos.h"
#include "f-display.h"
#include "f-wifi.h"
#include "f-pwm.h"
#include "f-ota.h"
#include "f-provisioning.h"
#include "f-integrations.h"
#include "f-membuffer.h"

#include "libs/fsdrv/lv_fsdrv.h"

// Add necessary NVS includes if they are missing
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_timer.h"

// Boot fail count for auto-rescue mode (3 failed boots -> rescue once)
#define BOOT_FAIL_THRESHOLD 3
#define BOOT_SUCCESS_DELAY_SEC 10
#define NVS_BOOT_NAMESPACE "frixos"
#define NVS_BOOT_FAIL_KEY "boot_fail_count"

static int rescue_mode_this_boot = 0; // Set by check_boot_fail_count() when 3 consecutive failed boots

// versioning variables
const char app[10] = "Frixos";
const char version[10] = "2.2";
static const char *TAG = "frixos main"; // in case we use ESP_LOGE -rror/W-arning/I-info (also D-ebug/V-erbose)
const int fwversion = 64;
const int rescuemode = 0; // 0 = normal, 1 = rescue mode
const char revision[] = "E";

// Mutex for HTTP operations
SemaphoreHandle_t http_mutex = NULL;

// Define constants based on Arduino EEPROM structure/usage
#define ARDUINO_NVS_NAMESPACE "eeprom" // Placeholder - Verify this namespace from Arduino code
#define ARDUINO_NVS_KEY "eeprom"       // Placeholder - Verify this key from Arduino code
#define EEPROM_SIZE 512                // ACTUAL EEPROM_SIZE from old code
#define EEPROM_SIG_1 0xF0              // As mentioned in comments
#define EEPROM_NAMESPACE "frixos"      // Define the NEW NVS namespace

/* Local setting types - avoid conflict with ESP-IDF nvs_type_t (NVS_TYPE_*) */
typedef enum {
    SETTING_TYPE_STR,
    SETTING_TYPE_U8,
    SETTING_TYPE_U16,
    SETTING_TYPE_U32,
    SETTING_TYPE_BLOB
} setting_type_t;

typedef struct {
    const char *key;
    setting_type_t type;
    void *ptr;
    size_t size;  // Used for string max length or blob size
} nvs_setting_t;

LV_FONT_DECLARE(lv_font_montserrat_8);
LV_FONT_DECLARE(lv_font_montserrat_10);
LV_FONT_DECLARE(lv_font_montserrat_12);
LV_FONT_DECLARE(lv_font_montserrat_14);

// EEPROM parameters and default values
char eeprom_hostname[33] = "frixos";
char eeprom_wifi_ssid[33] = "";
char eeprom_wifi_pass[64] = "";
uint8_t eeprom_wifi_start = 0;                                     // WiFi Active Hours Start (0-23), default 0
uint8_t eeprom_wifi_end = 0;                                       // WiFi Active Hours End (0-23), default 0
char eeprom_lat[12] = "", my_lat[12] = "";                         // "48.123456";
char eeprom_lon[12] = "", my_lon[12] = "";                         // "16.123456";
char eeprom_timezone[TZ_LENGTH] = "", my_timezone[TZ_LENGTH] = ""; // EET-2EEST,M3.5.0/3,M10.5.0/4";
char eeprom_font[2][12] = {"bold", "light"};                       // [0] = day font, [1] = night font
float eeprom_lux_sensitivity = 6.0;
float eeprom_lux_threshold = 16.0;
uint8_t eeprom_brightness_LED[2] = {100, 30}; // in pctuint8_t eeprom_dim_disable = 0;
uint8_t eeprom_dim_disable = 0;
uint8_t eeprom_fahrenheit = 1;
uint8_t eeprom_12hour = 1;
uint8_t eeprom_quiet_scroll = 1;
uint8_t eeprom_quiet_weather = 1;
uint8_t eeprom_show_leading_zero = 0;     // Show leading zero for single digit hour
uint8_t eeprom_dots_breathe = 0;          // Disable breathing effect for time dots (0=show, 1=don't show)
uint8_t eeprom_color_filter[2] = {0, 0};  // [0] = day, [1] = night
uint8_t eeprom_msg_red[2] = {255, 255};   // Default to white color for both day and night
uint8_t eeprom_msg_green[2] = {255, 255}; // Default to white color for both day and night
uint8_t eeprom_msg_blue[2] = {255, 255};  // Default to white color for both day and night
uint8_t eeprom_msg_font = 0;              // Default to Frixos 8 (0=Frixos8, 1=Montserrat8, 2=Montserrat10, 3=Montserrat12, 4=Montserrat14)
uint8_t eeprom_ofs_x = 22;
uint8_t eeprom_ofs_y = 22;
uint8_t eeprom_rotation = 3;        // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
uint8_t eeprom_mirroring = 0;       // 0 = normal, 1 = mirrored
uint8_t eeprom_show_grid = 0;       // 0 = no grid, 1 = show grid
uint8_t eeprom_update_firmware = 1; // yes, auto update firmware
uint8_t eeprom_dark_theme = 1;      // Default to dark theme (1 = dark, 0 = light)
uint8_t eeprom_language = 0;        // Default to English (0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da, 7=pl)
uint8_t eeprom_scroll_speed = 10;   // Default scroll speed in pixels per second
uint8_t eeprom_scroll_delay = 65;   // Default scroll delay in milliseconds (30-500)
char eeprom_message[SCROLL_MSG_LENGTH] = "[device]: [greeting] [day], [date] [mon], now [temp] today [high]-[low], hum. [hum], sun [rise]-[set]";

static const nvs_setting_t settings_table[] = {
    {"wifi_ssid", SETTING_TYPE_STR, eeprom_wifi_ssid, sizeof(eeprom_wifi_ssid)},
    {"wifi_pass", SETTING_TYPE_STR, eeprom_wifi_pass, sizeof(eeprom_wifi_pass)},
    {"hostname", SETTING_TYPE_STR, eeprom_hostname, sizeof(eeprom_hostname)},
    {"latitude", SETTING_TYPE_STR, eeprom_lat, sizeof(eeprom_lat)},
    {"longitude", SETTING_TYPE_STR, eeprom_lon, sizeof(eeprom_lon)},
    {"timezone", SETTING_TYPE_STR, eeprom_timezone, sizeof(eeprom_timezone)},
    {"dayfont", SETTING_TYPE_STR, eeprom_font[0], sizeof(eeprom_font[0])},
    {"nightfont", SETTING_TYPE_STR, eeprom_font[1], sizeof(eeprom_font[1])},
    {"dim_disable", SETTING_TYPE_U8, &eeprom_dim_disable, 0},
    {"fahrenheit", SETTING_TYPE_U8, &eeprom_fahrenheit, 0},
    {"12hour", SETTING_TYPE_U8, &eeprom_12hour, 0},
    {"wifi_start", SETTING_TYPE_U8, &eeprom_wifi_start, 0},
    {"wifi_end", SETTING_TYPE_U8, &eeprom_wifi_end, 0},
    {"quiet_scroll", SETTING_TYPE_U8, &eeprom_quiet_scroll, 0},
    {"quiet_weather", SETTING_TYPE_U8, &eeprom_quiet_weather, 0},
    {"lead_zero", SETTING_TYPE_U8, &eeprom_show_leading_zero, 0},
    {"dots_breathe", SETTING_TYPE_U8, &eeprom_dots_breathe, 0},
    {"color_filter", SETTING_TYPE_U8, &eeprom_color_filter[0], 0},
    {"msg_red", SETTING_TYPE_U8, &eeprom_msg_red[0], 0},
    {"msg_green", SETTING_TYPE_U8, &eeprom_msg_green[0], 0},
    {"msg_blue", SETTING_TYPE_U8, &eeprom_msg_blue[0], 0},
    {"night_filter", SETTING_TYPE_U8, &eeprom_color_filter[1], 0},
    {"night_msg_red", SETTING_TYPE_U8, &eeprom_msg_red[1], 0},
    {"night_msg_green", SETTING_TYPE_U8, &eeprom_msg_green[1], 0},
    {"night_msg_blue", SETTING_TYPE_U8, &eeprom_msg_blue[1], 0},
    {"msg_font", SETTING_TYPE_U8, &eeprom_msg_font, 0},
    {"offset_x", SETTING_TYPE_U8, &eeprom_ofs_x, 0},
    {"offset_y", SETTING_TYPE_U8, &eeprom_ofs_y, 0},
    {"rotation", SETTING_TYPE_U8, &eeprom_rotation, 0},
    {"mirroring", SETTING_TYPE_U8, &eeprom_mirroring, 0},
    {"show_grid", SETTING_TYPE_U8, &eeprom_show_grid, 0},
    {"update_firm", SETTING_TYPE_U8, &eeprom_update_firmware, 0},
    {"dark_theme", SETTING_TYPE_U8, &eeprom_dark_theme, 0},
    {"scroll_speed", SETTING_TYPE_U8, &eeprom_scroll_speed, 0},
    {"scroll_delay", SETTING_TYPE_U8, &eeprom_scroll_delay, 0},
    {"language", SETTING_TYPE_U8, &eeprom_language, 0},
    {"brightness", SETTING_TYPE_BLOB, eeprom_brightness_LED, sizeof(eeprom_brightness_LED)},
    {"lux_sens", SETTING_TYPE_BLOB, &eeprom_lux_sensitivity, sizeof(eeprom_lux_sensitivity)},
    {"lux_thresh", SETTING_TYPE_BLOB, &eeprom_lux_threshold, sizeof(eeprom_lux_threshold)},
    {"message", SETTING_TYPE_STR, eeprom_message, sizeof(eeprom_message)},
    {"ha_url", SETTING_TYPE_STR, eeprom_ha_url, sizeof(eeprom_ha_url)},
    {"ha_token", SETTING_TYPE_STR, eeprom_ha_token, sizeof(eeprom_ha_token)},
    {"ha_refresh", SETTING_TYPE_U16, &eeprom_ha_refresh_mins, 0},
    {"stock_key", SETTING_TYPE_STR, eeprom_stock_key, sizeof(eeprom_stock_key)},
    {"stock_refresh", SETTING_TYPE_U16, &eeprom_stock_refresh_mins, 0},
    {"dexcom_region", SETTING_TYPE_U8, &eeprom_dexcom_region, 0},
    {"glucose_high", SETTING_TYPE_U16, &eeprom_glucose_high, 0},
    {"glucose_low", SETTING_TYPE_U16, &eeprom_glucose_low, 0},
    {"libre_region", SETTING_TYPE_U8, &eeprom_libre_region, 0},
    {"ns_url", SETTING_TYPE_STR, eeprom_ns_url, sizeof(eeprom_ns_url)},
    {"cgm_username", SETTING_TYPE_STR, eeprom_glucose_username, sizeof(eeprom_glucose_username)},
    {"cgm_password", SETTING_TYPE_STR, eeprom_glucose_password, sizeof(eeprom_glucose_password)},
    {"cgm_refresh", SETTING_TYPE_U8, &eeprom_glucose_refresh, 0},
    {"cgm_validity", SETTING_TYPE_U16, &glucose_validity_duration, 0},
    {"sec_time", SETTING_TYPE_U8, &eeprom_sec_time, 0},
    {"sec_cgm", SETTING_TYPE_U8, &eeprom_sec_cgm, 0},
    {"cgm_unit", SETTING_TYPE_U8, &eeprom_glucose_unit, 0},
    {"pwm_frequency", SETTING_TYPE_U32, &eeprom_pwm_frequency, 0},
    {"max_power", SETTING_TYPE_U16, &eeprom_max_power, 0},
    {"poh", SETTING_TYPE_U32, &eeprom_poh, 0},
};
#define SETTINGS_COUNT (sizeof(settings_table) / sizeof(settings_table[0]))

static esp_err_t nvs_read_setting(nvs_handle_t handle, const nvs_setting_t *setting)
{
  esp_err_t err = ESP_OK;
  size_t size = setting->size;
  switch (setting->type)
  {
  case SETTING_TYPE_STR:
    err = nvs_get_str(handle, setting->key, (char *)setting->ptr, &size);
    break;
  case SETTING_TYPE_U8:
    err = nvs_get_u8(handle, setting->key, (uint8_t *)setting->ptr);
    break;
  case SETTING_TYPE_U16:
    err = nvs_get_u16(handle, setting->key, (uint16_t *)setting->ptr);
    break;
  case SETTING_TYPE_U32:
    err = nvs_get_u32(handle, setting->key, (uint32_t *)setting->ptr);
    break;
  case SETTING_TYPE_BLOB:
    err = nvs_get_blob(handle, setting->key, setting->ptr, &size);
    break;
  }
  return err;
}

static esp_err_t nvs_write_setting(nvs_handle_t handle, const nvs_setting_t *setting)
{
  esp_err_t err = ESP_OK;
  switch (setting->type)
  {
  case SETTING_TYPE_STR:
    err = nvs_set_str(handle, setting->key, (const char *)setting->ptr);
    break;
  case SETTING_TYPE_U8:
    err = nvs_set_u8(handle, setting->key, *(uint8_t *)setting->ptr);
    break;
  case SETTING_TYPE_U16:
    err = nvs_set_u16(handle, setting->key, *(uint16_t *)setting->ptr);
    break;
  case SETTING_TYPE_U32:
    err = nvs_set_u32(handle, setting->key, *(uint32_t *)setting->ptr);
    break;
  case SETTING_TYPE_BLOB:
    err = nvs_set_blob(handle, setting->key, setting->ptr, setting->size);
    break;
  }
  return err;
}

// Add Home Assistant integration parameters
char eeprom_ha_url[200] = {0};
char eeprom_ha_token[255] = {0};
uint16_t eeprom_ha_refresh_mins = 1;

// Add Stock Quote Service variables
char eeprom_stock_key[64] = {0};
uint16_t eeprom_stock_refresh_mins = 5;

// Dexcom settings
uint8_t eeprom_dexcom_region = 0;     // 0=disabled, 1=US, 2=Japan, 3=Rest of World
uint16_t eeprom_glucose_high = 175;   // Default high threshold in mg/dL
uint16_t eeprom_glucose_low = 70;     // Default low threshold in mg/dL
uint8_t eeprom_glucose_unit = 0;      // Glucose display unit: 0=mg/dL, 1=mmol/L
uint32_t eeprom_pwm_frequency = 200;  // Default PWM frequency in Hz (range 10-1000000)
uint16_t eeprom_max_power = MAX_DUTY; // Default max power (range 1-1023)

// LibreLinkUp settings
uint8_t eeprom_libre_region = 0; // 0=disabled, 1=US, 2=Japan, 3=Rest of World

// Shared glucose monitoring settings (used by both Dexcom and Libre)
char eeprom_glucose_username[64] = {0};
char eeprom_glucose_password[64] = {0};
uint8_t eeprom_glucose_refresh = 5;      // Default to 5 minutes
uint16_t glucose_validity_duration = 60; // Default to 60 minutes
uint8_t eeprom_sec_time = 25;            // Alternate time display duration (0-120 seconds)
uint8_t eeprom_sec_cgm = 5;              // Alternate CGM display duration (0-120 seconds)
char eeprom_libre_patient_id[64] = {0};
char eeprom_libre_token[512] = {0};
char libre_account_id[64] = {0};
char eeprom_libre_region_url[128] = {0};
char eeprom_ns_url[101] = {0}; // Nightscout URL (max 100 chars), NVS key ns_url

// Unified glucose data storage (shared by both Dexcom and Freestyle)
glucose_data_t glucose_data = {0};

// Power On Hours tracking
uint32_t eeprom_poh = 0;  // Power on hours counter
uint32_t current_poh = 0; // Current runtime POH counter (not saved to EEPROM)
time_t last_poh_save = 0; // Last time POH was saved to EEPROM

int weather_icon_index = -1;
int moon_icon_index = -1;
int time_just_validated = -1, time_valid = 0;
time_t last_weather_update = 0;      // Store timestamp of last weather update
time_t last_time_update = 0;         // Store timestamp of last time sync
bool settings_updated = false;       // Flag to indicate non-critical settings were updated
bool weather_has_updated = false;    // Flag to indicate weather data has been updated
bool ota_update_in_progress = false; // Flag to indicate OTA update is in progress
bool ota_updating_message = false;

// IP address display on boot
bool show_ip_on_boot = false;
bool ip_message_set = false;
int64_t ip_display_start_time = 0; // Changed to int64_t for esp_timer_get_time()
char boot_ip_address[18] = "";
uint8_t font_index = 0;
double weather_temp, weather_humidity;

i2c_master_bus_handle_t i2c_bus;
ltr303_dev_t ltr_dev;
char ltr303_gain = 7;             // gain of x7
char ltr303_integration_time = 3; // 500ms

CircularLog weblog = {0}; // Explicitly initialize to zero

#include <dirent.h>
#include <stdio.h>

void list_files(const char *path)
{
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    printf("Failed to open directory: %s\n", path);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL)
  {
    printf("Found file: %s\n", entry->d_name);
  }

  closedir(dir);
}

bool init_circular_log(CircularLog *log, int capacity)
{
  ESP_LOGI(TAG, "Initializing circular log with capacity %d", capacity);

  log->buffer = (char *)malloc(capacity);
  if (log->buffer == NULL)
  {
    // Handle allocation failure
    ESP_LOGE(TAG, "Failed to allocate memory for weblog buffer! Requested size: %d", capacity);
    log->capacity = 0;
    return false; // Indicate failure
  }

  ESP_LOGI(TAG, "Successfully allocated %d bytes for weblog buffer", capacity);

  log->capacity = capacity;
  log->head = 0;
  log->size = 0;
  log->full = false;
  memset(log->buffer, 0, capacity);

  ESP_LOGI(TAG, "Circular log initialized successfully");
  return true; // Indicate success
}

void free_circular_log(CircularLog *log)
{
  if (log->buffer != NULL)
  {
    free(log->buffer);
    log->buffer = NULL;
    log->capacity = 0;
    log->head = 0;
    log->size = 0;
    log->full = false;
  }
}

void append_to_log(CircularLog *log, const char *message)
{
  if (log->buffer == NULL || log->capacity == 0 || message == NULL)
    return; // Don't append if not initialized or message is null

  int len = strlen(message);
  for (int i = 0; i < len; i++)
  {
    log->buffer[log->head] = message[i];
    log->head = (log->head + 1) % log->capacity;

    if (!log->full)
    {
      log->size++;
      if (log->size == log->capacity)
        log->full = true;
    }
  }
}

void get_log_content(CircularLog *log, char *output, int max_len)
{
  if (log->buffer == NULL || log->capacity == 0)
  {
    output[0] = '\0';
    return; // Return empty string if not initialized
  }

  int start = log->full ? log->head : 0;
  int count = log->full ? log->capacity : log->size;
  int output_idx = 0;

  // Copy data from the circular buffer to the output buffer
  for (int i = 0; i < count && output_idx < max_len - 1; i++)
  {
    output[output_idx++] = log->buffer[start];
    start = (start + 1) % log->capacity;
  }
  output[output_idx] = '\0'; // Null-terminate the output string
}

// New function to log to console and weblog array
void ESP_LOG_WEB(esp_log_level_t level, const char *tag, const char *format, ...)
{
  // Check if weblog is properly initialized before using it
  if (weblog.buffer == NULL || weblog.capacity == 0)
  {
    // If weblog is not initialized, just log to console and return
    va_list args_console;
    va_start(args_console, format);
    esp_log_writev(level, tag, format, args_console);
    va_end(args_console);
    return;
  }

  // only log to weblog if level is less than INFO (WARN, ERROR).
  if (level <= ESP_LOG_INFO)
  {
    // Get current time for timestamp
    time_t now;
    struct tm timeinfo;
    char timestamp[10]; // Buffer for "HH:MM "

    time(&now);
    localtime_r(&now, &timeinfo);

    // Check if we have a valid time (year > 2020 as a simple heuristic)
    if (timeinfo.tm_year + 1900 > 2020)
    {
      // Format time as "HH:MM "
      snprintf(timestamp, sizeof(timestamp), "%02d:%02d ",
               timeinfo.tm_hour, timeinfo.tm_min);
    }
    else
    {
      // If time is not set yet, use "--:-- " placeholder
      strcpy(timestamp, "--:-- ");
    }

    // Format the new log message with timestamp for weblog
    va_list args_web;
    va_start(args_web, format);

    // Format the full message with timestamp into a temporary buffer
    char log_message[256];
    char *p = log_message;

    // Copy timestamp
    strcpy(p, timestamp);
    p += strlen(timestamp);

    // Format the message after timestamp
    vsnprintf(p, sizeof(log_message) - strlen(timestamp), format, args_web);

    // Add to circular buffer
    append_to_log(&weblog, log_message);
    append_to_log(&weblog, "\n");

    va_end(args_web);
  }

  // Format and print the log message to console using ESP_LOGI mechanism
  // We need to re-start the va_list as it might have been consumed by vsnprintf
  va_list args_console;
  va_start(args_console, format);
  esp_log_writev(level, tag, format, args_console);
  va_end(args_console);

  // Print an extra newline for better readability in console, along with the current thread/core info
  char thread_info_prefix[64];
  snprintf(thread_info_prefix, sizeof(thread_info_prefix), " [%s@%d]\n",
           pcTaskGetName(NULL), // Get current task name
           xPortGetCoreID());   // Get current core ID

  esp_log_write(level, tag, thread_info_prefix);
}

void ESP_LOGI_STACK(const char *tag, const char *msg)
{
  ESP_LOG_WEB(ESP_LOG_INFO, tag, "%s heap %lu stack %u", msg, esp_get_free_heap_size(), uxTaskGetStackHighWaterMark(NULL));
}

void startup_diags(void)
{
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "%s %s (v%d)", app, version, fwversion);

  /* Print chip information */
  esp_chip_info_t chip_info;
  uint32_t flash_size;
  esp_chip_info(&chip_info);
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "%s chip with %d CPU core(s), %s%s%s%s, ",
              CONFIG_IDF_TARGET,
              chip_info.cores,
              (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
              (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
              (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
              (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "silicon v%d.%d, ", major_rev, minor_rev);
  if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Flash size failed");
    return;
  }

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
              (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Min heap %" PRIu32, esp_get_minimum_free_heap_size());
}

/** Check boot fail count and set rescue_mode_this_boot if 3 consecutive failed boots.
 *  Call early in app_main, after nvs_flash_init. */
static void check_boot_fail_count(void)
{
  nvs_handle_t h;
  esp_err_t err = nvs_open(NVS_BOOT_NAMESPACE, NVS_READWRITE, &h);
  if (err != ESP_OK)
  {
    ESP_LOGW(TAG, "Cannot open NVS for boot count: %s", esp_err_to_name(err));
    return;
  }

  uint8_t fail_count = 0;
  nvs_get_u8(h, NVS_BOOT_FAIL_KEY, &fail_count); // ignore error, default 0

  fail_count++;
  nvs_set_u8(h, NVS_BOOT_FAIL_KEY, fail_count);
  nvs_commit(h);

  if (fail_count > BOOT_FAIL_THRESHOLD) // at least BOOT_FAIL_THRESHOLD failed boots, next one resets settings
  {
    rescue_mode_this_boot = 1;
    ESP_LOGW(TAG, "Auto-rescue: %u failed boots, entering rescue mode (reset settings except WiFi)", (unsigned)fail_count);
    nvs_set_u8(h, NVS_BOOT_FAIL_KEY, 0); // reset after entering rescue
    nvs_commit(h);
  }
  else
  {
    ESP_LOGI(TAG, "Boot fail count: %u/%u", (unsigned)fail_count, (unsigned)BOOT_FAIL_THRESHOLD);
  }
  nvs_close(h);
}

/** Clear boot fail count on successful boot. Called by 120s success timer. */
static void clear_boot_fail_count(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_BOOT_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_set_u8(h, NVS_BOOT_FAIL_KEY, 0);
  nvs_commit(h);
  nvs_close(h);
  ESP_LOGI(TAG, "Boot success: cleared fail count");
}

static esp_timer_handle_t s_boot_success_timer = NULL;

static void boot_success_timer_cb(void *arg)
{
  (void)arg;
  clear_boot_fail_count();
  if (s_boot_success_timer)
  {
    esp_timer_delete(s_boot_success_timer);
    s_boot_success_timer = NULL;
  }
}

void startup_read_eeprom(void)
{
  esp_err_t err;

  // 1. Initialize NVS (unchanged)
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "NVS truncated, erasing");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  nvs_handle_t nvs_handle;
  err = nvs_open(EEPROM_NAMESPACE, NVS_READONLY, &nvs_handle);

  if (err != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "NVS open %s: %s", EEPROM_NAMESPACE, esp_err_to_name(err));
    // Consider setting default values here if NVS isn't available
    // set_default_parameters(); // Example function call
  }
  else
  {
    for (int i = 0; i < SETTINGS_COUNT; i++)
    {
      const nvs_setting_t *s = &settings_table[i];

      // if rescue mode (compile-time override or auto-rescue after 3 failed boots), read only SSID and password
      // these are the first two entries in settings_table
      if ((rescuemode == 1 || rescue_mode_this_boot) && i > 1)
        continue;

      err = nvs_read_setting(nvs_handle, s);

      if (err == ESP_ERR_NVS_INVALID_LENGTH && s->type == SETTING_TYPE_U32)
      {
        /* Special case for pwm_frequency migration */
        uint16_t val16;
        if (nvs_get_u16(nvs_handle, s->key, &val16) == ESP_OK)
        {
          *(uint32_t *)s->ptr = val16;
          err = ESP_OK;
        }
      }

      if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
      {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "NVS Read Error %s: %s", s->key, esp_err_to_name(err));
      }
    }

    if (rescuemode == 1 || rescue_mode_this_boot)
    {
      // save all default values back to eeprom
      nvs_close(nvs_handle);
      write_nvs_parameters();
      return;
    }

    // Synchronize global variables from NVS strings
    strcpy(my_lat, eeprom_lat);
    strcpy(my_lon, eeprom_lon);
    strcpy(my_timezone, eeprom_timezone);

    if (eeprom_max_power == 850) // reduce all 850 limit to 750
    {
      eeprom_max_power = MAX_DUTY;
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Reduced max_power from 850 to %u", eeprom_max_power);
    }

    // Initialize current POH counter and last save time
    current_poh = eeprom_poh;
    last_poh_save = time(NULL);

    // Close NVS
    nvs_close(nvs_handle);

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "HTTPD_MAX_REQ_HDR_LEN=%u",
                (unsigned)HTTPD_MAX_REQ_HDR_LEN);

    // 5. Log final parameters
    ESP_LOG_WEB(ESP_LOG_INFO, TAG,
                "Final params:\n"
                "  Network:     hostname=%s, wifi_ssid=%s, wifi_start=%u, wifi_end=%u\n"
                "  Location:    lat=%s, lon=%s, tz=%s\n"
                "  Fonts:       day=%s, night=%s\n"
                "  Display:     dim=%u, F=%u, 12h=%u, q_scr=%u, q_wea=%u, lead_zero=%u, dots_breathe=%u, filter=[%u,%u]\n"
                "  Message:     rgb=([%u,%u,%u],[%u,%u,%u]), font=%u, offset=(%u,%u), rot=%u, mirror=%u, grid=%u\n"
                "  Message txt: %s\n"
                "  UI:          update=%u, dark=%u, lang=%u, scroll_dly=%u, scroll_speed=%u\n"
                "  Brightness:  led=[%u,%u], lux_sens=%.1f, lux_thresh=%.1f\n"
                "  HomeAssist:  url=%s, ha_refresh=%u (token redacted)\n"
                "  Stock:       stock_refresh=%u (key redacted)\n"
                "  Dexcom:      region=%u, refresh=%u, high=%u, low=%u\n"
                "  Libre:       region=%u, ns_url=%s\n"
                "  CGM:         username=%s, validity=%u, sec_time=%u, sec_cgm=%u, unit=%u (password redacted)\n"
                "  PWM:         freq=%u, max_power=%u, poh=%u",
                eeprom_hostname, eeprom_wifi_ssid, eeprom_wifi_start, eeprom_wifi_end,
                eeprom_lat, eeprom_lon, eeprom_timezone,
                eeprom_font[0], eeprom_font[1],
                eeprom_dim_disable, eeprom_fahrenheit, eeprom_12hour,
                eeprom_quiet_scroll, eeprom_quiet_weather,
                eeprom_show_leading_zero, eeprom_dots_breathe,
                eeprom_color_filter[0], eeprom_color_filter[1],
                eeprom_msg_red[0], eeprom_msg_green[0], eeprom_msg_blue[0],
                eeprom_msg_red[1], eeprom_msg_green[1], eeprom_msg_blue[1],
                eeprom_msg_font,
                eeprom_ofs_x, eeprom_ofs_y, eeprom_rotation, eeprom_mirroring,
                eeprom_show_grid,
                eeprom_message,
                eeprom_update_firmware, eeprom_dark_theme, eeprom_language, eeprom_scroll_delay, eeprom_scroll_speed,
                eeprom_brightness_LED[0], eeprom_brightness_LED[1],
                eeprom_lux_sensitivity, eeprom_lux_threshold,
                eeprom_ha_url, eeprom_ha_refresh_mins,
                eeprom_stock_refresh_mins,
                eeprom_dexcom_region, eeprom_glucose_refresh, eeprom_glucose_high, eeprom_glucose_low,
                eeprom_libre_region, eeprom_ns_url,
                eeprom_glucose_username, glucose_validity_duration, eeprom_sec_time, eeprom_sec_cgm, eeprom_glucose_unit,
                eeprom_pwm_frequency, eeprom_max_power, eeprom_poh);
  }

} // end startup_read_eeprom
// Function to write all parameters to NVS
esp_err_t write_nvs_parameters(void)
{
  nvs_handle_t nvs_handle;
  esp_err_t err;

  // Initialize NVS if not initialized
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  // Open NVS using the defined EEPROM_NAMESPACE
  err = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    return err;
  }

  // Backwards compatibility for PWM frequency
  if (eeprom_pwm_frequency == 133)
    eeprom_pwm_frequency = 200;

  // Write all settings from table
  for (int i = 0; i < SETTINGS_COUNT; i++)
  {
    const nvs_setting_t *s = &settings_table[i];

    // Skip hostname in manufacturer mode
    if (manufacturer_mode && strcmp(s->key, "hostname") == 0)
      continue;

    err = nvs_write_setting(nvs_handle, s);
    if (err != ESP_OK)
    {
      ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "NVS Write Error %s: %s", s->key, esp_err_to_name(err));
    }
  }

  // Commit changes
  err = nvs_commit(nvs_handle);
  if (err != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to commit NVS data: %s", esp_err_to_name(err));
  }
  else
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Parameters saved to NVS successfully");
  }

  // Close NVS
  nvs_close(nvs_handle);

  return err;
}

double ltr303_get_frixos_lux()
{
  uint16_t ch0 = 0, ch1 = 0;
  if (!ltr303_get_data(&ltr_dev, &ch0, &ch1))
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "LTR303 Failed reading data. Error=%d", ltr303_get_error(&ltr_dev));
  };

  double lux = 0.0;
  // get reading from ALS sensor
  ltr303_get_lux(ltr303_gain, ltr303_integration_time, ch0, ch1, &lux);
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "LTR303 reading: %.2lf = %d, %d", lux, ch0, ch1);
  return lux;
}

void startup_ltr303(void)
{

  // 3) Initialize the LTR303 device with new driver
  ltr303_init(&ltr_dev, i2c_bus, LTR303_DEFAULT_I2C_ADDR);

  // 4) Check if device is present
  if (!ltr303_begin(&ltr_dev))
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LTR303 not responding!");
    return;
  }

  // 5) Power up sensor
  ltr303_set_power_up(&ltr_dev);

  // set gain
  ltr303_set_control(&ltr_dev, ltr303_gain, true, true);
  // Set measurement rate (integrationTime=3 => 400ms, measurementRate=3 => 500ms)
  ltr303_set_measurement_rate(&ltr_dev, 3, 3);

  uint8_t part_id = 0;
  if (ltr303_get_part_id(&ltr_dev, &part_id))
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "LTR 303 available, ID %d", part_id);
  }
  else
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LTR 303 not available");
  }
}

// OTA progress callback function
void ota_progress_callback(int progress, const char *message)
{
  // don't really care to have any updates; this feels superfluous anyway, should kill it at some point
  // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "OTA Progress: %d%% - %s", progress, message);
}

// POH timer callback function - called every hour
void poh_timer_callback(void *arg)
{
  current_poh++;
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "POH incremented to %u hours", current_poh);

  // Check if we need to save to EEPROM (every 8 hours)
  time_t now = time(NULL);
  // only update POH if we are not in manufacturer mode
  if (!manufacturer_mode)
  {
    if (now - last_poh_save >= 8 * 3600) // 8 hours in seconds
    {
      eeprom_poh = current_poh;
      last_poh_save = now;

      // Save to NVS
      nvs_handle_t nvs_handle;
      esp_err_t err = nvs_open(EEPROM_NAMESPACE, NVS_READWRITE, &nvs_handle);
      if (err == ESP_OK)
      {
        err = nvs_set_u32(nvs_handle, "poh", eeprom_poh);
        if (err == ESP_OK)
        {
          nvs_commit(nvs_handle);
          ESP_LOG_WEB(ESP_LOG_INFO, TAG, "POH saved to EEPROM: %u hours", eeprom_poh);
        }
        else
        {
          ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to save POH to NVS: %s", esp_err_to_name(err));
        }
        nvs_close(nvs_handle);
      }
      else
      {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to open NVS for POH save: %s", esp_err_to_name(err));
      }
    }
  }
}

void startup_spiffs()
{

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Mounting SPIFFS filesystem...");
  // Mount the SPIFF Filesystem
  esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",        // mount point in the VFS
      .partition_label = NULL,       // if partition label in partition.csv is "spiffs", match it here
      .max_files = 5,                // how many files can be open simultaneously
      .format_if_mount_failed = true // format if cannot mount
  };

  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Couldn't mount SPIFFS filesystem");
  }
  else
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "SPIFFS mounted ok");
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Failed to get SPIFFS partition information (%i)", ret);
    else
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Partition size: total: %d used: %d", total, used);
  }

  // register the filesystem with LVGL so we can open files
  lv_fs_posix_init();
}

void startup_threads()
{
  // Create a task, pinned to core 1, to take care of display stuff
  xTaskCreatePinnedToCore(
      display_task,   /* Task function. */
      "display_task", /* name of task, from f-display.c */
      8192 + 2048,    /* Stack size of task */
      NULL,           /* parameter of the task */
      3,              /* priority of the task */
      NULL,           /* Task handle to keep track of created task */
      1);             /* ping to the APP core */

  // Create a task, pinnned to core 0, to take care of Internet and Web updates
  xTaskCreatePinnedToCore(
      wifi_task,   /* Task function. */
      "wifi_task", /* name of task. */
      8192,        /* Stack size of task */
      NULL,        /* parameter of the task */
      3,           /* priority of the task */
      NULL,        /* Task handle to keep track of created task */
      0);          /* pin to app core*/
}

void startup_poh_timer()
{
  // Create POH timer
  esp_timer_create_args_t poh_timer_args = {
      .callback = &poh_timer_callback,
      .name = "poh_timer"};

  esp_timer_handle_t poh_timer;
  esp_err_t err = esp_timer_create(&poh_timer_args, &poh_timer);
  if (err != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create POH timer: %s", esp_err_to_name(err));
    return;
  }

  // Start POH timer - runs every hour (3600 seconds = 3600000000 microseconds)
  err = esp_timer_start_periodic(poh_timer, 3600000000ULL);
  if (err != ESP_OK)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to start POH timer: %s", esp_err_to_name(err));
    esp_timer_delete(poh_timer);
    return;
  }

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "POH timer started - will increment every hour, save every 8 hours");
}

void app_main(void)
{
  ESP_LOGI(TAG, "Starting Frixos application...");
  ESP_LOGI(TAG, "Initial free heap: %lu bytes", esp_get_free_heap_size());

  // Initialize NVS early for boot fail count (before anything that can crash)
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_LOGW(TAG, "NVS partition needs format. Erasing and re-initializing.");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  check_boot_fail_count();

  // warn level for the serial log; the web log is still going to show INFO and below.
  esp_log_level_set("*", ESP_LOG_INFO);

  // Initialize the circular log buffer
  if (!init_circular_log(&weblog, DEFAULT_LOG_BUFFER_SIZE))
  {
    // Handle initialization failure, perhaps by trying a smaller size or disabling web logging
    ESP_LOGE(TAG, "Weblog initialization failed. Web logging might be unavailable.");
    // Try with a smaller buffer size
    if (!init_circular_log(&weblog, 512))
    {
      ESP_LOGE(TAG, "Weblog initialization failed even with smaller buffer. Disabling web logging.");
    }
  }
  else
  {
    ESP_LOGI(TAG, "Weblog initialized successfully with buffer size %d", DEFAULT_LOG_BUFFER_SIZE);
  }

  ESP_LOGI(TAG, "Free heap after weblog init: %lu bytes", esp_get_free_heap_size());

  init_buffer_management();
  ESP_LOGI(TAG, "Buffer management initialized successfully");
  ESP_LOGI(TAG, "Free heap after buffer management init: %lu bytes", esp_get_free_heap_size());

  // Initialize the default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  http_mutex = xSemaphoreCreateMutex(); // create a mutex for "important" http operations

  startup_diags();

  startup_read_eeprom();
  startup_ltr303();
  startup_lcd();
  startup_lvgl();
  startup_spiffs();
  startup_integrations();
  startup_display();
  startup_threads();   // threads need to start before provisioning, if we want a functioning display
  startup_poh_timer(); // start POH timer

  provision_init();                                   // start wifi connection or provisioning
  f_ota_verify();                                     // so far so good; finalize the last OTA update (if any)
  f_ota_set_progress_callback(ota_progress_callback); // Set up OTA progress callback
  vTaskDelay(pdMS_TO_TICKS(4000));                    // Wait 4 seconds for tasks to initialize
  startup_led_pwm();
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Startup complete");

  // Start 120s success timer: clear boot fail count on successful run
  esp_timer_handle_t success_timer = NULL;
  esp_timer_create_args_t success_args = {
      .callback = boot_success_timer_cb,
      .name = "boot_success"};
  if (esp_timer_create(&success_args, &success_timer) == ESP_OK)
  {
    s_boot_success_timer = success_timer;
    esp_timer_start_once(success_timer, BOOT_SUCCESS_DELAY_SEC * 1000000ULL);
  }
}
