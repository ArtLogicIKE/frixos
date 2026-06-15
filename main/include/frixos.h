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
#include "nvs.h"

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
extern char eeprom_static_ip[16];   // p60: Static IP (empty = DHCP)
extern char eeprom_static_gw[16];   // p61: Default gateway
extern char eeprom_static_nm[16];   // p62: Subnet mask
extern char eeprom_static_dns[40];  // p63: DNS servers, comma-separated
extern uint8_t eeprom_wifi_start;  // WiFi Active Hours Start (0-23)
extern uint8_t eeprom_wifi_end;    // WiFi Active Hours End (0-23)
extern uint8_t eeprom_dim_start;   // Time-of-day dimming start hour (0-23)
extern uint8_t eeprom_dim_end;     // Time-of-day dimming end hour (0-23)
extern char eeprom_lat[12], my_lat[12];                         // "48.123456";
extern char eeprom_lon[12], my_lon[12];                         // "16.123456";
extern char eeprom_timezone[TZ_LENGTH], my_timezone[TZ_LENGTH]; // EET-2EEST,M3.5.0/3,M10.5.0/4";
extern char eeprom_font[2][12];     // [0] = day font, [1] = night font
extern char eeprom_aux_font[2][12]; // [0] = day aux digit font, [1] = night aux
extern float eeprom_lux_sensitivity;
extern float eeprom_lux_threshold;
extern uint8_t eeprom_brightness_LED[2]; // in pct
#define DIM_MODE_BRIGHTNESS 0
#define DIM_MODE_FULL 1
#define DIM_MODE_TIME 2
extern uint8_t eeprom_dim_mode; // p22: 0=brightness, 1=full, 2=time-of-day
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
extern uint8_t  eeprom_msg_font;        // Message font size (0=8pt, 1=10pt, 2=12pt, 3=14pt, 4=16pt)
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
extern uint32_t eeprom_pwm_frequency;  // PWM frequency in Hz (range 10-300000)
extern uint16_t eeprom_max_power;      // Max power (range 1-1023)
extern uint8_t eeprom_board_rev;       // Board revision read from NVS (0=rev A-F, 1=rev H)

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
// Time of latest CGM measurement (12/24hr per settings, localized am/pm suffix)
void format_glucose_time_token(char *buffer, size_t buffer_size);
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
extern uint8_t eeprom_sec_weather;
extern char    eeprom_disp_sched[512];
extern char    eeprom_disp_sched_aux[512];

/* --- Display schedule --- */
typedef enum {
    SLOT_TYPE_TIME         = 0,
    SLOT_TYPE_CGM          = 1,
    SLOT_TYPE_WEATHER_TEMP = 2,
    SLOT_TYPE_HA           = 3,
} slot_type_t;

#define MAX_DISPLAY_SLOTS  8
#define SLOT_ENTITY_LEN    64
#define SLOT_NAME_LEN      26   /* max 25 characters + NUL */

typedef struct {
    slot_type_t type;
    uint16_t    duration;                /* seconds to show this slot */
    char        entity[SLOT_ENTITY_LEN]; /* HA entity ID (SLOT_TYPE_HA only) */
    char        label[8];               /* short unit shown next to digits, e.g. "°F", "W" */
    char        name[SLOT_NAME_LEN];    /* optional label on Digit Label while slot is active */
} display_slot_t;

extern display_slot_t display_schedule[MAX_DISPLAY_SLOTS];
extern int            display_schedule_count;
extern display_slot_t display_schedule_aux[MAX_DISPLAY_SLOTS];
extern int            display_schedule_aux_count;

void parse_display_schedule(const char *json);
void parse_display_schedule_aux(const char *json);
void migrate_schedule_from_legacy(void);

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
extern bool ota_reinstall_in_progress;
extern bool ota_updating_message;

// IP address display on boot
extern bool show_ip_on_boot;
extern bool ip_message_set;
extern int64_t ip_display_start_time;  // Changed to int64_t for esp_timer_get_time()
extern char boot_ip_address[18];

#define IP_DISPLAY_DURATION_SEC 15

// --------------------------
// Screen layout configuration
// --------------------------
// Two profiles: [0]=day, [1]=night. Selected at runtime by font_index (lux-driven).
#define FRIXOS_SCREEN_LAYOUT_VERSION 9
#define FRIXOS_SCREEN_LAYOUT_PROFILES 2
#define SCREEN_STATIC_TEXT_LENGTH 96
#define SCREEN_STATIC_TEXT_COUNT 8

#define SCREEN_MSG_ALIGN_LEFT 0
#define SCREEN_MSG_ALIGN_CENTER 1
#define SCREEN_MSG_ALIGN_RIGHT 2

typedef enum
{
  SCREEN_ELEM_GLUCOSE_LEVEL = 0,
  SCREEN_ELEM_GLUCOSE_TREND = 1,
  SCREEN_ELEM_WIFI_OFF = 2,
  SCREEN_ELEM_WEATHER = 3,
  SCREEN_ELEM_MOON = 4,
  SCREEN_ELEM_TIME = 5,
  SCREEN_ELEM_MESSAGE = 6,
  SCREEN_ELEM_TEXT_1 = 7,
  SCREEN_ELEM_TEXT_2 = 8,
  SCREEN_ELEM_TEXT_3 = 9,
  SCREEN_ELEM_TEXT_4 = 10,
  SCREEN_ELEM_TEXT_5 = 11,
  SCREEN_ELEM_TEXT_6 = 12,
  SCREEN_ELEM_TEXT_7 = 13,
  SCREEN_ELEM_TEXT_8 = 14,
  SCREEN_ELEM_AMPM = 15,
  SCREEN_ELEM_TIME_AUX = 16,
  SCREEN_ELEM_DIGIT_LABEL = 17,
  SCREEN_ELEM_DIGIT_LABEL_AUX = 18,
  SCREEN_ELEM_COUNT
} screen_element_id_t;

typedef struct
{
  uint8_t enabled; // 0/1
  uint8_t x;       // 0..127 absolute screen coordinate
  uint8_t y;       // 0..127 absolute screen coordinate
  uint8_t z;       // 0..4 layer index, higher = on top
  uint8_t font;    // 0..4 (8pt..16pt) for message/text elements
  uint8_t width;   // static text clip width in pixels, 0 = no limit
  uint8_t align;   // SCREEN_MSG_ALIGN_* for message/text elements
  uint8_t color_r;
  uint8_t color_g;
  uint8_t color_b;
  uint8_t bg_r;
  uint8_t bg_g;
  uint8_t bg_b;
} screen_widget_t;

typedef struct
{
  screen_widget_t widget[SCREEN_ELEM_COUNT];
  char scroll_text[SCROLL_MSG_LENGTH];
  char static_text[SCREEN_STATIC_TEXT_COUNT][SCREEN_STATIC_TEXT_LENGTH];
  char digit_label_text[SCREEN_STATIC_TEXT_LENGTH];
  char digit_label_aux_text[SCREEN_STATIC_TEXT_LENGTH];
} screen_layout_profile_t;

typedef struct
{
  uint8_t version;
  uint8_t scroll_delay; // milliseconds, scrolling message only
  uint16_t reserved;
  screen_layout_profile_t profile[FRIXOS_SCREEN_LAYOUT_PROFILES];
} screen_layout_t;

extern screen_layout_t eeprom_screen_layout;

static inline bool screen_elem_is_static_text(screen_element_id_t id)
{
  return id >= SCREEN_ELEM_TEXT_1 && id <= SCREEN_ELEM_TEXT_8;
}

static inline bool screen_elem_is_digit_label(screen_element_id_t id)
{
  return id == SCREEN_ELEM_DIGIT_LABEL || id == SCREEN_ELEM_DIGIT_LABEL_AUX;
}

static inline bool screen_elem_is_text(screen_element_id_t id)
{
  return id == SCREEN_ELEM_MESSAGE || screen_elem_is_static_text(id) || screen_elem_is_digit_label(id);
}

static inline bool screen_elem_has_text_layout(screen_element_id_t id)
{
  return screen_elem_is_static_text(id) || screen_elem_is_digit_label(id);
}

static inline int screen_static_text_index(screen_element_id_t id)
{
  return (int)id - (int)SCREEN_ELEM_TEXT_1;
}

void build_integration_message_corpus(char *out, size_t out_size);
void screen_layout_apply_factory_defaults(screen_layout_t *layout);
void screen_layout_sync_legacy_eeprom(const screen_layout_t *layout);
void screen_layout_ensure_valid(void);

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
#define LCD_RGB_ELEMENT_ORDER (LCD_RGB_ELEMENT_ORDER_BGR)
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


#define DEFAULT_LOG_BUFFER_SIZE 2048  // log buffer for web UI


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