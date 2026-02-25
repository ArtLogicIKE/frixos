#ifndef F_OTA_H
#define F_OTA_H

#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_log.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Update server configuration
#define UPDATE_SERVER_BASE "http://update.artlogic.gr:8080"
#define UPDATE_CHECK_INTERVAL (8 * 3600) // 8 hours in seconds
#define UPDATE_TIMEOUT_MS 30000 // 30 seconds timeout
#define MAX_RETRIES 1

// Update status codes
typedef enum {
    UPDATE_SUCCESS = 0,
    UPDATE_ERROR_NO_UPDATE,
    UPDATE_ERROR_DOWNLOAD,
    UPDATE_ERROR_VERIFY,
    UPDATE_ERROR_INSTALL,
    UPDATE_ERROR_ROLLBACK
} update_status_t;

// Update progress callback type
typedef void (*update_progress_callback_t)(int progress, const char* message);

// Function declarations
void f_ota_verify(void);
void f_ota_check_update(void);
void f_ota_set_progress_callback(update_progress_callback_t callback);
void f_ota_report_status(update_status_t status, const char *error_msg);
void f_ota_start_update_thread(void);
void f_ota_stop_update_thread(void);
void f_ota_trigger_update(void);

#endif // F_OTA_H 