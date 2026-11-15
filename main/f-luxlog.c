#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_attr.h"
#include "esp_rom_crc.h"
#include "freertos/FreeRTOS.h"
#include "f-luxlog.h"

#define LUXLOG_MAGIC 0x4C55583Au
#define LUXLOG_SLOTS 144 // 24 h
#define LUXLOG_STEP_S 600
#define LUXLOG_EMPTY 0xFFFFu
#define LUXLOG_MAX 0xFFFEu // lux x10, so readings above 6553.4 lux are clamped

typedef struct
{
    uint32_t magic;
    uint32_t slot;              // newest slot, as epoch / LUXLOG_STEP_S
    uint16_t v[LUXLOG_SLOTS];   // lux x10 by slot % LUXLOG_SLOTS
    uint32_t crc;
} luxlog_rec_t;

// RTC_NOINIT survives software, panic and watchdog resets; the CRC rejects it after power loss.
static RTC_NOINIT_ATTR luxlog_rec_t rec;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static double slot_sum;
static uint32_t slot_cnt;

static uint32_t rec_crc(const luxlog_rec_t *r)
{
    return esp_rom_crc32_le(0, (const uint8_t *)r, offsetof(luxlog_rec_t, crc));
}

static void clear_all(uint32_t slot)
{
    rec.magic = LUXLOG_MAGIC;
    rec.slot = slot;
    for (int i = 0; i < LUXLOG_SLOTS; i++)
        rec.v[i] = LUXLOG_EMPTY;
}

void luxlog_add(double lux, time_t now)
{
    uint32_t slot = (uint32_t)(now / LUXLOG_STEP_S);
    if (lux < 0)
        lux = 0;
    portENTER_CRITICAL(&mux);
    bool valid = rec.magic == LUXLOG_MAGIC && rec.crc == rec_crc(&rec);
    if (valid && slot < rec.slot && rec.slot - slot < 6) // small clock step back: keep the history
    {
        portEXIT_CRITICAL(&mux);
        return;
    }
    if (!valid || slot < rec.slot || slot - rec.slot >= LUXLOG_SLOTS)
    {
        clear_all(slot);
        slot_sum = 0;
        slot_cnt = 0;
    }
    else if (slot != rec.slot)
    {
        for (uint32_t s = rec.slot + 1; s < slot; s++) // slots with no readings (clock was off)
            rec.v[s % LUXLOG_SLOTS] = LUXLOG_EMPTY;
        rec.slot = slot;
        slot_sum = 0;
        slot_cnt = 0;
    }
    slot_sum += lux;
    slot_cnt++;
    double avg10 = slot_sum / slot_cnt * 10.0 + 0.5;
    rec.v[slot % LUXLOG_SLOTS] = avg10 >= LUXLOG_MAX ? LUXLOG_MAX : (uint16_t)avg10;
    rec.crc = rec_crc(&rec);
    portEXIT_CRITICAL(&mux);
}

void luxlog_add_json(cJSON *root)
{
    static luxlog_rec_t copy; // only the httpd task calls this
    portENTER_CRITICAL(&mux);
    copy = rec;
    portEXIT_CRITICAL(&mux);
    if (copy.magic != LUXLOG_MAGIC || copy.crc != rec_crc(&copy))
        return;

    char buf[LUXLOG_SLOTS * 8 + 64]; // "6553.4," at most 7 chars per slot
    int n = snprintf(buf, sizeof(buf), "{\"step\":%d,\"end\":%lu,\"v\":[", LUXLOG_STEP_S,
                     (unsigned long)copy.slot * LUXLOG_STEP_S);
    for (int i = 0; i < LUXLOG_SLOTS; i++)
    {
        uint16_t x = copy.v[(copy.slot + 1 + i) % LUXLOG_SLOTS]; // oldest first
        if (x == LUXLOG_EMPTY)
            n += snprintf(buf + n, sizeof(buf) - n, "%snull", i ? "," : "");
        else
            n += snprintf(buf + n, sizeof(buf) - n, "%s%u.%u", i ? "," : "", x / 10, x % 10);
    }
    snprintf(buf + n, sizeof(buf) - n, "]}");
    cJSON_AddRawToObject(root, "luxhist", buf);
}
