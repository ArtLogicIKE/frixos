#ifndef FRIXOS_H
#define FRIXOS_H    // Guard to prevent multiple inclusion  

/*
MENUCONFIG
Flash Size - 8MB
Partition Table - custom
Memory allocation - default
Allow HTTP for OTA
CPU Frequency - 240MHz
Place FreeRTOS functions into Flash
Ensure WiFi IRAM speed optimization NOT checked
File system on top of posix API
Set an upper cased letter on which the drive will accessible (e.g. 65 for 'A') -- 83 (S)
Set the working directory "/spiffs"

ESP Timer Stack Size -> 8192, otherwise OTA Update Timer will not work


Enable Bluetooth
Host NimBLE only
Disable 'Enable BLE Secure connection flag'


Wifi
disable Software controls WiFi/Bluetooth coexistence
disable Enable enterprise option

ESP System Settings
Main task stack size
8192 from 4096


*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include <sys/time.h>

#include "esp_system.h"
#include "esp_mac.h"
#include "esp_check.h"
#include "esp_spiffs.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"

#include "ltr303.h"

#include "lvgl.h"
#include "font/lv_font.h"

// Generic App Stuff
extern const char app[];
extern const char version[];
extern const int fwversion;
extern const char revision[];

extern ltr303_dev_t ltr_dev;
extern char ltr303_gain;
extern char ltr303_integration_time;

// NVS - EEPROM parameters
#define EEPROM_NAMESPACE "frixos"

#define TZ_LENGTH 128
#define SCROLL_MSG_LENGTH 512

extern char eeprom_hostname[33]; 
extern char eeprom_wifi_ssid[33];
extern char eeprom_wifi_pass[64];
extern uint8_t eeprom_wifi_start;  // WiFi Active Hours Start (0-23)
extern uint8_t eeprom_wifi_end;    // WiFi Active Hours End (0-23)
extern char eeprom_lat[12], my_lat[12];                         // "48.123456";
extern char eeprom_lon[12], my_lon[12];                         // "16.123456";
extern char eeprom_timezone[TZ_LENGTH], my_timezone[TZ_LENGTH]; // EET-2EEST,M3.5.0/3,M10.5.0/4";
extern char eeprom_font[2][12]; // [0] = day font, [1] = night font
extern float eeprom_lux_sensitivity;
extern float eeprom_lux_threshold;
extern uint8_t eeprom_brightness_LED[2]; // in pct
extern uint8_t  eeprom_dim_disable;
extern uint8_t  eeprom_fahrenheit;
extern uint8_t  eeprom_12hour;
extern uint8_t  eeprom_quiet_scroll;
extern uint8_t  eeprom_quiet_weather;
extern uint8_t  eeprom_show_leading_zero;
extern uint8_t  eeprom_dots_breathe;  // Enable breathing effect for time dots (0=disabled, 1=enabled)
extern uint8_t  eeprom_color_filter[2]; // [0] = day filter, [1] = night filter
extern uint8_t  eeprom_msg_red[2];      // [0] = day red, [1] = night red
extern uint8_t  eeprom_msg_green[2];    // [0] = day green, [1] = night green
extern uint8_t  eeprom_msg_blue[2];     // [0] = day blue, [1] = night blue
extern uint8_t  eeprom_msg_font;        // Message font size (0=Frixos8, 1=Montserrat8, 2=Montserrat10, 3=Montserrat12, 4=Montserrat14)
extern uint8_t  eeprom_ofs_x;
extern uint8_t  eeprom_ofs_y;
extern uint8_t  eeprom_rotation;
extern uint8_t  eeprom_mirroring;
extern uint8_t  eeprom_show_grid;
extern uint8_t  eeprom_update_firmware;
extern uint8_t  eeprom_dark_theme;
extern uint8_t  eeprom_language;  // Language index: 0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da, 7=pl
extern uint8_t  eeprom_scroll_speed;  // Scroll speed in pixels per second
extern uint8_t  eeprom_scroll_delay;  // Scroll delay in milliseconds (30-200)
extern char eeprom_message[SCROLL_MSG_LENGTH];
extern char eeprom_ha_url[200];      // Home Assistant server URL
extern char eeprom_ha_token[255];    // Home Assistant long-lived access token
extern uint16_t eeprom_ha_refresh_mins;  // Home Assistant refresh interval in seconds
extern bool manufacturer_mode; // true if we are in manufacturer mode

// Add Stock Quote Service variables
extern char eeprom_stock_key[64];    // Finnhub API key
extern uint16_t eeprom_stock_refresh_mins;  // Stock quote refresh interval in seconds

// Dexcom settings
extern uint8_t eeprom_dexcom_region;  // 0=disabled, 1=US, 2=Japan, 3=Rest of World
extern uint16_t eeprom_glucose_high;   // High glucose threshold in mg/dL
extern uint16_t eeprom_pwm_frequency;  // PWM frequency in Hz (range 10-5000)
extern uint16_t eeprom_max_power;      // Max power (range 1-1023)

// LibreLinkUp settings
extern uint8_t eeprom_libre_region;    // 0=disabled, 1=US, 2=Japan, 3=Rest of World

// Unified glucose data structure (shared by both Dexcom and Freestyle)
typedef struct {
    float current_gl_mgdl;
    float previous_gl_mgdl;
    float gl_diff;
    int trend_arrow;  // 0=down fast, 1=down, 2=stable, 3=up, 4=up fast, -1=no arrow
    time_t timestamp;
} glucose_data_t;

extern glucose_data_t glucose_data;  // Unified glucose data storage

// Unified glucose formatting function (used by both Dexcom and Freestyle)
void format_glucose_token(char *buffer, size_t buffer_size);
// Get plain glucose reading (just the number, no formatting)
void get_glucose_reading_plain(char *buffer, size_t buffer_size);

// Shared glucose monitoring settings (used by both Dexcom and Libre)
extern char eeprom_glucose_username[64];
extern char eeprom_glucose_password[64];
extern uint8_t eeprom_glucose_refresh;
extern uint8_t eeprom_glucose_unit;  // Glucose display unit: 0=mg/dL, 1=mmol/L
extern uint16_t glucose_validity_duration;
extern uint8_t eeprom_sec_time;
extern uint8_t eeprom_sec_cgm;
extern char eeprom_libre_patient_id[64];
extern char eeprom_libre_token[512];
extern char libre_account_id[64];
extern char eeprom_libre_region_url[128];
extern char eeprom_ns_url[101];  // Nightscout URL (max 100 chars), NVS key ns_url

// Power On Hours tracking
extern uint32_t eeprom_poh;           // Power on hours counter (saved to EEPROM)
extern uint32_t current_poh;          // Current runtime POH counter (not saved to EEPROM)
extern time_t last_poh_save;          // Last time POH was saved to EEPROM
extern uint16_t eeprom_glucose_low;    // Low glucose threshold in mg/dL

extern int weather_icon_index;
extern int moon_icon_index;
extern int time_just_validated, time_valid;
extern uint8_t font_index;
extern time_t last_weather_update;  // Store timestamp of last weather update
extern time_t last_time_update;     // Store timestamp of last time sync
extern bool settings_updated;       // Flag to indicate non-critical settings were updated
extern bool weather_has_updated;    // Flag to indicate weather data has been updated
extern bool integration_tokens_updated;

extern time_t first_time_sync;

extern time_t ota_start_time;
extern bool ota_update_in_progress;
extern bool ota_updating_message;

// IP address display on boot
extern bool show_ip_on_boot;
extern bool ip_message_set;
extern int64_t ip_display_start_time;  // Changed to int64_t for esp_timer_get_time()
extern char boot_ip_address[18];

#define IP_DISPLAY_DURATION_SEC 15

// TFT Displat ST7735S
/* LCD size */
#define LCD_H_RES   (128)
#define LCD_V_RES   (128)
#define LCD_DRAW_BUFF_DOUBLE (1)  // Enable double buffering to prevent tearing
#define LCD_DRAW_BUFF_HEIGHT (8) // Optimized buffer size for smooth animations

/* LCD settings */
#define LCD_SPI_NUM         (SPI3_HOST)
#define LCD_PIXEL_CLK_HZ    (40 * 1000 * 1000)
#define LCD_CMD_BITS        (8)
#define LCD_PARAM_BITS      (8)
#define LCD_COLOR_SPACE     (ESP_LCD_COLOR_SPACE_BGR)
#define LCD_BITS_PER_PIXEL  (16)
#define LCD_BL_ON_LEVEL     (1)

/* LCD pins */
#define LCD_GPIO_SCLK       (GPIO_NUM_18)
#define LCD_GPIO_MOSI       (GPIO_NUM_23)
#define LCD_GPIO_RST        (GPIO_NUM_4)
#define LCD_GPIO_DC         (GPIO_NUM_2)
#define LCD_GPIO_CS         (GPIO_NUM_5)
#define LCD_GPIO_BL         (GPIO_NUM_26)


// I2C Stuff
#define I2C_MASTER_SDA_IO   (GPIO_NUM_21)
#define I2C_MASTER_SCL_IO   (GPIO_NUM_22)
#define I2C_MASTER_FREQ_HZ  (100*1000) // 100 kHz


#define DEFAULT_LOG_BUFFER_SIZE 1024  // Reduced from 2048 to free ~1KB heap for SSL connections


#define MAX_AP_SCAN 30 // Maximum number of APs to scan and display
#define MIN_FREE_HEAP 4096  // Minimum free heap before rejecting requests

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif


// Circular buffer for logging
typedef struct {
    char *buffer;       // Dynamically allocated buffer
    int capacity;       // Size of the buffer
    int head;           // Current write position
    int size;           // Current number of bytes used
    bool full;          // Flag to track if buffer is full
} CircularLog;

extern CircularLog weblog;

// Mutex for HTTP operations
extern SemaphoreHandle_t http_mutex;

// Functions for the circular log
bool init_circular_log(CircularLog *log, int capacity);
void append_to_log(CircularLog *log, const char *message);
void get_log_content(CircularLog *log, char *output, int max_len);
void free_circular_log(CircularLog *log); // Function to free allocated memory

// internal function to read LTR303 brightness sensor
double ltr303_get_frixos_lux();

void startup_read_eeprom(void);
// Function to write all parameters to NVS
esp_err_t write_nvs_parameters(void);
void ESP_LOGI_STACK(const char *tag, const char *msg);
void ESP_LOG_WEB(esp_log_level_t level, const char *tag, const char *format, ...);

#endif