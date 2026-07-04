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

// Update server configuration. The #ifndef guard lets a test build override
// the server without touching source (e.g. -DUPDATE_SERVER_BASE=...).
// Single source of truth - f-wifi.c (timezone lookups) uses this too.
#ifndef UPDATE_SERVER_BASE
#define UPDATE_SERVER_BASE "http://update.artlogic.gr:8080"
#endif
#define UPDATE_CHECK_INTERVAL (8 * 3600) // 8 hours in seconds
#define UPDATE_TIMEOUT_MS 30000 // 30 seconds timeout
#define MAX_RETRIES 1
// Idle update checks between quiet manifest re-fetches (web-file-only
// publishes reach a device within ~QUIET_REFRESH_EVERY_CHECKS * 8h).
#define QUIET_REFRESH_EVERY_CHECKS 10

// Update status codes (reported to the update server; append only)
typedef enum {
    UPDATE_SUCCESS = 0,
    UPDATE_ERROR_NO_UPDATE,
    UPDATE_ERROR_DOWNLOAD,
    UPDATE_ERROR_VERIFY,
    UPDATE_ERROR_INSTALL,
    UPDATE_ERROR_ROLLBACK,
    UPDATE_ERROR_SIGNATURE // manifest missing/unsigned/bad signature or downgrade
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
void f_ota_trigger_reinstall(void);

#endif // F_OTA_H 