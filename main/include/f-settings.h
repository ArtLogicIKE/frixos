/**
 * ESP-IDF Web Configuration Interface
 * Header file for settings management
 */

#ifndef F_SETTINGS_H
#define F_SETTINGS_H

#include "frixos.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_event.h"

// SPIFFS configuration
#define SPIFFS_BASE_PATH "/spiffs"


// URL handlers for webserver
esp_err_t settings_post_handler(httpd_req_t *req);
esp_err_t status_api_handler(httpd_req_t *req);
esp_err_t ota_post_handler(httpd_req_t *req);
esp_err_t reset_post_handler(httpd_req_t *req);
esp_err_t root_get_handler(httpd_req_t *req);
esp_err_t generic_file_handler(httpd_req_t *req);
// Portal API handler (Option 114) removed

// WiFi scanning handlers
esp_err_t wifi_scan_start_handler(httpd_req_t *req);
esp_err_t wifi_scan_status_handler(httpd_req_t *req);

// For direct access to settings JSON
esp_err_t send_json_settings(httpd_req_t *req);

// Init functions for webserver
esp_err_t init_settings_server(void);
esp_err_t start_webserver(void);
void stop_webserver(void);

// Helper functions
void restart_device(void *pvParameter);

// External functions that we'll use
extern esp_err_t write_nvs_parameters(void);

#endif // F_SETTINGS_H