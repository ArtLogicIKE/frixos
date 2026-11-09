#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_attr.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_core_dump.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "frixos.h"
#include "f-ota.h"
#include "f-wifi.h"
#include "f-reboot.h"

static const char *TAG = "REBOOT";

// RTC_NOINIT memory survives software, panic, watchdog and brownout resets, not power loss.
#define REBOOT_REC_MAGIC 0x52425431u
typedef struct
{
    uint32_t magic;
    uint32_t up_s;     // uptime at the last tick
    uint32_t min_heap; // lowest free heap at the last tick
    char cause[20];
    char detail[48];
} reboot_rec_t;
static RTC_NOINIT_ATTR reboot_rec_t rec;

// What this boot reports (filled by f_reboot_capture).
static char rep_cause[sizeof(rec.cause)];
static char rep_detail[sizeof(rec.detail)];
static char rep_crash[200];
static uint32_t rep_up_s, rep_min_heap;
static esp_reset_reason_t rep_rst;
static bool reported = false;
static bool prev_rec_valid = false; // previous boot ran f-reboot (not older firmware)

static const char *reset_reason_name(esp_reset_reason_t r)
{
    switch (r)
    {
    case ESP_RST_POWERON: return "poweron";
    case ESP_RST_EXT: return "ext";
    case ESP_RST_SW: return "sw";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "int_wdt";
    case ESP_RST_TASK_WDT: return "task_wdt";
    case ESP_RST_WDT: return "wdt";
    case ESP_RST_DEEPSLEEP: return "deepsleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unknown";
    }
}

void f_reboot_restart(const char *cause, const char *detail)
{
    rec.magic = REBOOT_REC_MAGIC;
    strlcpy(rec.cause, cause ? cause : "", sizeof(rec.cause));
    strlcpy(rec.detail, detail ? detail : "", sizeof(rec.detail));
    rec.up_s = (uint32_t)(esp_timer_get_time() / 1000000);
    rec.min_heap = esp_get_minimum_free_heap_size();
    esp_restart();
}

void f_reboot_tick(void)
{
    rec.magic = REBOOT_REC_MAGIC;
    rec.up_s = (uint32_t)(esp_timer_get_time() / 1000000);
    rec.min_heap = esp_get_minimum_free_heap_size();
}

static void capture_crash(void)
{
    esp_core_dump_summary_t *s = malloc(sizeof(*s));
    if (s && esp_core_dump_get_summary(s) == ESP_OK)
    {
        int n = snprintf(rep_crash, sizeof(rep_crash), "%s pc %08lx bt", s->exc_task, (unsigned long)s->exc_pc);
        for (uint32_t i = 0; i < s->exc_bt_info.depth && i < 8 && n > 0 && n < (int)sizeof(rep_crash); i++)
            n += snprintf(rep_crash + n, sizeof(rep_crash) - n, " %08lx", (unsigned long)s->exc_bt_info.bt[i]);
        if (n > 0 && n < (int)sizeof(rep_crash))
        {
            char reason[64];
            if (esp_core_dump_get_panic_reason(reason, sizeof(reason)) == ESP_OK)
                snprintf(rep_crash + n, sizeof(rep_crash) - n, " | %s", reason);
        }
    }
    free(s);
    // Report each dump once; a later software restart must not resend it.
    esp_core_dump_image_erase();
}

void f_reboot_capture(void)
{
    rep_rst = esp_reset_reason();
    if (rec.magic == REBOOT_REC_MAGIC && rep_rst != ESP_RST_POWERON)
    {
        prev_rec_valid = true;
        rec.cause[sizeof(rec.cause) - 1] = '\0';
        rec.detail[sizeof(rec.detail) - 1] = '\0';
        strlcpy(rep_cause, rec.cause, sizeof(rep_cause));
        strlcpy(rep_detail, rec.detail, sizeof(rep_detail));
        rep_up_s = rec.up_s;
        rep_min_heap = rec.min_heap;
    }
    if (rep_rst == ESP_RST_PANIC || rep_rst == ESP_RST_INT_WDT || rep_rst == ESP_RST_TASK_WDT)
        capture_crash();

    rec.magic = REBOOT_REC_MAGIC;
    rec.cause[0] = '\0';
    rec.detail[0] = '\0';
    rec.up_s = 0;
    rec.min_heap = 0;
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Reset %s cause %s %s up %lus heap %lu %s", reset_reason_name(rep_rst),
                rep_cause[0] ? rep_cause : "-", rep_detail, (unsigned long)rep_up_s,
                (unsigned long)rep_min_heap, rep_crash);
}

// Privacy: report only what helps troubleshoot. Owner actions (restart button,
// settings save, manual upload), OTA results (already sent by /update_result),
// provisioning timeouts, power-on and external resets stay on the device's log.
static bool worth_reporting(void)
{
    switch (rep_rst)
    {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
        return true;
    case ESP_RST_SW:
        // No record: the restart came from firmware without f-reboot (first boot
        // after an update), not from an unexplained restart in this firmware.
        if (!prev_rec_valid)
            return false;
        break;
    default:
        return false;
    }
    static const char *const routine[] = {"user", "settings", "fw_upload", "ota_ok", "ota_failed", "ota_files", "provision_timeout"};
    for (size_t i = 0; i < sizeof(routine) / sizeof(routine[0]); i++)
        if (strcmp(rep_cause, routine[i]) == 0)
            return false;
    return true; // our failure recoveries, and software restarts with no recorded cause
}

void f_reboot_report(void)
{
    if (reported || !wifi_connected)
        return;
    if (!worth_reporting())
    {
        reported = true;
        return;
    }

    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    char cause[sizeof(rep_cause) * 3], detail[sizeof(rep_detail) * 3], crash[sizeof(rep_crash) * 3];
    url_encode_string(rep_cause, cause);
    url_encode_string(rep_detail, detail);
    url_encode_string(rep_crash, crash);

    char url[1024];
    snprintf(url, sizeof(url),
             "%s/reboot_report?mac=%02X%02X%02X%02X%02X%02X&fw=%d&rst=%s&cause=%s&detail=%s&up=%lu&heap=%lu&crash=%s",
             UPDATE_SERVER_BASE, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], fwversion,
             reset_reason_name(rep_rst), cause, detail, (unsigned long)rep_up_s,
             (unsigned long)rep_min_heap, crash);

    esp_http_client_config_t config = {
        .url = url,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
        return;
    if (esp_http_client_perform(client) == ESP_OK && esp_http_client_get_status_code(client) == 200)
        reported = true;
    else
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Reboot report failed, will retry");
    esp_http_client_cleanup(client);
}
