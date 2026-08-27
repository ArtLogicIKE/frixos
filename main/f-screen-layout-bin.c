#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "f-screen-layout-bin.h"
#include "frixos.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_littlefs.h"

#define SCREEN_LAYOUT_NVS_NAMESPACE "frixos"
#define SCREEN_LAYOUT_NVS_KEY "screen_layout"

static const char *TAG = "f-screen-bin";

_Static_assert(sizeof(screen_widget_t) == 13, "screen_widget_t wire size changed");
_Static_assert(sizeof(screen_graph_cfg_t) == 88, "screen_graph_cfg_t wire size changed");
_Static_assert(sizeof(screen_layout_profile_t) == 1924, "screen_layout_profile_t wire size changed"); // +8 icon widgets
_Static_assert(sizeof(screen_layout_bin_header_t) == 64, "screen_layout_bin_header_t size changed");
_Static_assert(FRIXOS_SCREEN_LAYOUT_WIRE_SIZE == 3912, "screen_layout_wire_t size changed");

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static void sanitize_widget(screen_widget_t *w, screen_element_id_t id)
{
    w->enabled = w->enabled ? 1 : 0;
    w->x = (uint8_t)clamp_int(w->x, 0, 127);
    w->y = (uint8_t)clamp_int(w->y, 0, 127);
    w->z = (uint8_t)clamp_int(w->z, 0, 4);

    if (!screen_elem_is_text(id))
        return;

    w->font = (uint8_t)clamp_int(w->font, 0, 5); // 0-4 = 8..16pt, 5 = 5pt tiny
    w->color_r = w->color_r;
    w->color_g = w->color_g;
    w->color_b = w->color_b;
    w->bg_r = w->bg_r;
    w->bg_g = w->bg_g;
    w->bg_b = w->bg_b;

    if (screen_elem_has_text_layout(id))
    {
        w->width = (uint8_t)clamp_int(w->width, 0, 127);
        w->align = (uint8_t)clamp_int(w->align, 0, SCREEN_MSG_ALIGN_RIGHT);
    }
}

static void sanitize_graph(screen_graph_cfg_t *g)
{
    g->token[GRAPH_TOKEN_LEN - 1] = '\0';
    g->interval_min = (uint16_t)clamp_int(g->interval_min, 1, 1440);
    g->points = (uint8_t)clamp_int(g->points, 2, GRAPH_MAX_POINTS);
    g->width = (uint8_t)clamp_int(g->width, GRAPH_MIN_W, GRAPH_MAX_W);
    g->height = (uint8_t)clamp_int(g->height, GRAPH_MIN_H, GRAPH_MAX_H);
    // band_low/high and y_min/y_max are signed graph-unit values (GRAPH_VAL_UNSET allowed) — no clamp.
}

static void sanitize_profile(screen_layout_profile_t *profile)
{
    for (int i = 0; i < SCREEN_ELEM_COUNT; i++)
        sanitize_widget(&profile->widget[i], (screen_element_id_t)i);

    profile->scroll_text[SCROLL_MSG_LENGTH - 1] = '\0';
    for (int i = 0; i < SCREEN_STATIC_TEXT_COUNT; i++)
        profile->static_text[i][SCREEN_STATIC_TEXT_LENGTH - 1] = '\0';
    profile->digit_label_text[SCREEN_STATIC_TEXT_LENGTH - 1] = '\0';
    profile->digit_label_aux_text[SCREEN_STATIC_TEXT_LENGTH - 1] = '\0';
    sanitize_graph(&profile->graph);
}

static void copy_font_field(char *dst, size_t dst_len, const char *src)
{
    if (!dst || dst_len == 0)
        return;
    if (!src)
    {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, dst_len - 1);
    dst[dst_len - 1] = '\0';
}

static void fill_wire_header(screen_layout_bin_header_t *header)
{
    memset(header, 0, sizeof(*header));
    header->magic = FRIXOS_SCREEN_BIN_MAGIC;
    header->format = FRIXOS_SCREEN_BIN_FORMAT;
    header->layout_version = eeprom_screen_layout.version;
    header->scroll_delay = eeprom_screen_layout.scroll_delay;
    header->day_color_filter = eeprom_color_filter[0];
    header->night_color_filter = eeprom_color_filter[1];
    copy_font_field(header->day_font, sizeof(header->day_font), eeprom_font[0]);
    copy_font_field(header->night_font, sizeof(header->night_font), eeprom_font[1]);
    copy_font_field(header->day_aux_font, sizeof(header->day_aux_font), eeprom_aux_font[0]);
    copy_font_field(header->night_aux_font, sizeof(header->night_aux_font), eeprom_aux_font[1]);
    header->w = LCD_H_RES;
    header->h = LCD_V_RES;
}

static void screen_layout_nvs_erase(void)
{
    nvs_handle_t h;
    if (nvs_open(SCREEN_LAYOUT_NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
        return;
    if (nvs_erase_key(h, SCREEN_LAYOUT_NVS_KEY) == ESP_OK)
        nvs_commit(h);
    nvs_close(h);
}

static bool littlefs_is_mounted(void)
{
    size_t total = 0, used = 0;
    return esp_littlefs_info("spiffs", &total, &used) == ESP_OK;
}

static void apply_factory_layout(void)
{
    screen_layout_apply_factory_defaults(&eeprom_screen_layout);
    screen_layout_sync_legacy_eeprom(&eeprom_screen_layout);
}

esp_err_t screen_layout_file_save(void)
{
    if (!littlefs_is_mounted())
        return ESP_ERR_INVALID_STATE;

    screen_layout_ensure_valid();

    screen_layout_bin_header_t header;
    fill_wire_header(&header);

    FILE *f = fopen(SCREEN_LAYOUT_FILE_PATH, "wb");
    if (!f)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Cannot write %s", SCREEN_LAYOUT_FILE_PATH);
        return ESP_FAIL;
    }

    const size_t profile_bytes = sizeof(eeprom_screen_layout.profile);
    bool ok = fwrite(&header, 1, sizeof(header), f) == sizeof(header) &&
              fwrite(eeprom_screen_layout.profile, 1, profile_bytes, f) == profile_bytes;
    fclose(f);
    if (!ok)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed writing %s", SCREEN_LAYOUT_FILE_PATH);
        unlink(SCREEN_LAYOUT_FILE_PATH);
        return ESP_FAIL;
    }

    screen_layout_nvs_erase();
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Layout saved to %s", SCREEN_LAYOUT_FILE_PATH);
    return ESP_OK;
}

static esp_err_t screen_layout_migrate_from_nvs(void)
{
    nvs_handle_t h;
    if (nvs_open(SCREEN_LAYOUT_NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
        return ESP_ERR_NOT_FOUND;

    size_t sz = 0;
    esp_err_t err = nvs_get_blob(h, SCREEN_LAYOUT_NVS_KEY, NULL, &sz);
    if (err != ESP_OK || sz != sizeof(eeprom_screen_layout))
    {
        nvs_close(h);
        if (err != ESP_ERR_NVS_NOT_FOUND)
            screen_layout_nvs_erase();
        return ESP_ERR_NOT_FOUND;
    }

    sz = sizeof(eeprom_screen_layout);
    err = nvs_get_blob(h, SCREEN_LAYOUT_NVS_KEY, &eeprom_screen_layout, &sz);
    nvs_close(h);
    if (err != ESP_OK)
        return ESP_ERR_NOT_FOUND;

    screen_layout_ensure_valid();
    err = screen_layout_file_save();
    if (err == ESP_OK)
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Migrated screen_layout from NVS to %s", SCREEN_LAYOUT_FILE_PATH);
    else
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Layout in RAM from NVS; file save failed (%s)", esp_err_to_name(err));
    return ESP_OK;
}

void screen_layout_file_load(void)
{
    FILE *f = fopen(SCREEN_LAYOUT_FILE_PATH, "rb");
    if (!f)
    {
        if (screen_layout_migrate_from_nvs() == ESP_OK)
            return;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No layout file, using factory defaults");
        apply_factory_layout();
        return;
    }

    screen_layout_bin_header_t header;
    const size_t profile_bytes = sizeof(eeprom_screen_layout.profile);
    size_t nheader = fread(&header, 1, sizeof(header), f);
    size_t nprof = fread(eeprom_screen_layout.profile, 1, profile_bytes, f);
    fclose(f);

    if (nheader != sizeof(header) || nprof != profile_bytes ||
        header.magic != FRIXOS_SCREEN_BIN_MAGIC ||
        header.format != FRIXOS_SCREEN_BIN_FORMAT ||
        header.layout_version > FRIXOS_SCREEN_LAYOUT_VERSION)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid %s, using factory defaults", SCREEN_LAYOUT_FILE_PATH);
        unlink(SCREEN_LAYOUT_FILE_PATH);
        apply_factory_layout();
        screen_layout_nvs_erase();
        return;
    }

    eeprom_screen_layout.version = FRIXOS_SCREEN_LAYOUT_VERSION;
    eeprom_screen_layout.scroll_delay = (uint8_t)clamp_int(header.scroll_delay, 30, 255);
    eeprom_screen_layout.reserved = 0;
    for (int pi = 0; pi < FRIXOS_SCREEN_LAYOUT_PROFILES; pi++)
        sanitize_profile(&eeprom_screen_layout.profile[pi]);
    screen_layout_sync_legacy_eeprom(&eeprom_screen_layout);
    screen_layout_nvs_erase();
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Loaded layout from %s", SCREEN_LAYOUT_FILE_PATH);
}

void screen_layout_file_remove(void)
{
    if (unlink(SCREEN_LAYOUT_FILE_PATH) == 0)
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Removed %s", SCREEN_LAYOUT_FILE_PATH);
    screen_layout_nvs_erase();
}

bool screen_layout_wire_encode(uint8_t *out, size_t out_size, size_t *written)
{
    if (!out || out_size < FRIXOS_SCREEN_LAYOUT_WIRE_SIZE)
        return false;

    screen_layout_wire_t *wire = (screen_layout_wire_t *)out;
    memset(wire, 0, sizeof(*wire));
    fill_wire_header(&wire->header);

    memcpy(wire->profile, eeprom_screen_layout.profile, sizeof(wire->profile));

    if (written)
        *written = FRIXOS_SCREEN_LAYOUT_WIRE_SIZE;
    return true;
}

esp_err_t screen_layout_wire_decode(const uint8_t *data, size_t len, screen_layout_t *layout,
                                    char day_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char night_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char day_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char night_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    uint8_t *day_color_filter, uint8_t *night_color_filter)
{
    if (!data || len != FRIXOS_SCREEN_LAYOUT_WIRE_SIZE || !layout)
        return ESP_ERR_INVALID_ARG;

    const screen_layout_wire_t *wire = (const screen_layout_wire_t *)data;
    if (wire->header.magic != FRIXOS_SCREEN_BIN_MAGIC)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Bad screen wire magic 0x%08lx", (unsigned long)wire->header.magic);
        return ESP_ERR_INVALID_ARG;
    }
    if (wire->header.format != FRIXOS_SCREEN_BIN_FORMAT)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Unsupported screen wire format %u", wire->header.format);
        return ESP_ERR_INVALID_VERSION;
    }
    if (wire->header.layout_version > FRIXOS_SCREEN_LAYOUT_VERSION)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Screen wire version %u > firmware %u",
                    wire->header.layout_version, FRIXOS_SCREEN_LAYOUT_VERSION);
        return ESP_ERR_INVALID_VERSION;
    }

    memset(layout, 0, sizeof(*layout));
    layout->version = FRIXOS_SCREEN_LAYOUT_VERSION;
    layout->scroll_delay = (uint8_t)clamp_int(wire->header.scroll_delay, 30, 255);
    memcpy(layout->profile, wire->profile, sizeof(layout->profile));

    for (int pi = 0; pi < FRIXOS_SCREEN_LAYOUT_PROFILES; pi++)
        sanitize_profile(&layout->profile[pi]);

    if (day_font)
        copy_font_field(day_font, FRIXOS_SCREEN_BIN_FONT_LEN, wire->header.day_font);
    if (night_font)
        copy_font_field(night_font, FRIXOS_SCREEN_BIN_FONT_LEN, wire->header.night_font);
    if (day_aux_font)
        copy_font_field(day_aux_font, FRIXOS_SCREEN_BIN_FONT_LEN, wire->header.day_aux_font);
    if (night_aux_font)
        copy_font_field(night_aux_font, FRIXOS_SCREEN_BIN_FONT_LEN, wire->header.night_aux_font);
    if (day_color_filter)
        *day_color_filter = (uint8_t)clamp_int(wire->header.day_color_filter, 0, 4);
    if (night_color_filter)
        *night_color_filter = (uint8_t)clamp_int(wire->header.night_color_filter, 0, 4);

    return ESP_OK;
}
