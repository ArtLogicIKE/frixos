#ifndef F_WIFI_H
#define F_WIFI_H

#include "frixos.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "mdns.h"
#include "lwip/ip4_addr.h"
#include <string.h>
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <time.h>
extern volatile bool wifi_connected;
extern volatile bool wifi_disabled_by_active_hours; // WiFi disabled due to active hours
extern bool wifi_first_connect, weather_valid;
extern time_t sunrise, sunset;
extern double weather_temp, weather_humidity;
extern double weather_high, weather_low;
extern char greeting[64];

// OTA update thread control
extern TaskHandle_t ota_update_task_handle;
extern bool ota_update_pending;
extern SemaphoreHandle_t ota_update_semaphore;

//Generate function prototypes for f-wifi.c here. Include and neccessary #includes
void initialize_mdns(void);
bool is_wifi_connected(void);
bool wifi_get_location(void);
bool wifi_get_weather(void);
void wifi_task(void *pvParameters);
void ota_update_timer_callback(void *arg);
esp_err_t f_http_post(const char* url, const char* data);
void url_encode_string(const char *input, char *output);
bool validate_coordinate(const char *coord_str, bool is_latitude);
bool validate_timezone(const char *tz_str);

// External variables
extern int response_len;
extern esp_err_t http_event_handler(esp_http_client_event_t *evt);
extern char my_ip[18], internal_ip[18];
extern char *wifi_http_buffer;  // Add this to track the current buffer

#endif