#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "f-screen-layout-bin.h"
#include "f-layout-migrate.h"
#include "frixos.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_littlefs.h"

#define SCREEN_LAYOUT_NVS_NAMESPACE "frixos"
#define SCREEN_LAYOUT_NVS_KEY "screen_layout" // blob of layouts before they moved to a file
#define SCREEN_DEFAULT_LAYOUT_FILE_PATH "/spiffs/layouts/Default.layout.bin"

static const char *TAG = "f-screen-bin";

_Static_assert(sizeof(screen_widget_t) == 13, "screen_widget_t wire size changed");
_Static_assert(sizeof(screen_graph_cfg_t) == 88, "screen_graph_cfg_t wire size changed");
_Static_assert(sizeof(screen_layout_profile_t) == 795, "screen_layout_profile_t wire size changed");
_Static_assert(sizeof(legacy_profile_v10_t) == LEGACY_PROFILE_V10_SIZE, "legacy profile size changed");
_Static_assert(sizeof(screen_layout_bin_header_t) == 64, "screen_layout_bin_header_t size changed");
_Static_assert(FRIXOS_SCREEN_LAYOUT_WIRE_SIZE == 1654, "screen_layout_wire_t size changed");
_Static_assert(sizeof(text_widget_cfg_t) == 216, "text_widget_cfg_t wire size changed");
_Static_assert(sizeof(widget_meta_t) == 516, "widget_meta_t wire size changed");
_Static_assert(FRIXOS_SCREEN_LAYOUT_EXT_SIZE == 3980, "screen_layout_ext_wire_t size changed");

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static void sanitize_widget(screen_widget_t *w, screen_element_id_t id, bool preserve_legacy_message)
{
    w->enabled = w->enabled ? 1 : 0;
    w->x = (uint8_t)clamp_int(w->x, 0, 127);
    w->y = (uint8_t)clamp_int(w->y, 0, 127);
    w->z = (uint8_t)clamp_int(w->z, 0, 4);

    if (id == SCREEN_ELEM_MESSAGE && !preserve_legacy_message)
        w->enabled = 0; // reserved wire slot; scrolling is configured on a text widget
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

static void sanitize_profile(screen_layout_profile_t *profile, bool preserve_legacy_message)
{
    for (int i = 0; i < SCREEN_ELEM_COUNT; i++)
        sanitize_widget(&profile->widget[i], (screen_element_id_t)i, preserve_legacy_message);

    profile->scroll_text[SCROLL_MSG_LENGTH - 1] = '\0';
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
    header->reserved[0] = (uint8_t)clamp_int(eeprom_weather_theme[0], 0, 12);
    header->reserved[1] = (uint8_t)clamp_int(eeprom_weather_theme[1], 0, 12);
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
    eeprom_weather_theme[0] = 0;
    eeprom_weather_theme[1] = 0;
}

void screen_layout_ext_header(screen_layout_ext_header_t *header)
{
    memset(header, 0, sizeof(*header));
    header->magic = FRIXOS_SCREEN_EXT_MAGIC;
    header->version = FRIXOS_SCREEN_EXT_VERSION;
    header->count = TEXT_WIDGET_COUNT;
    header->item_size = sizeof(text_widget_cfg_t);
}

static bool ext_header_ok(const screen_layout_ext_header_t *h, bool allow_v2)
{
    const bool version_ok = h->version == FRIXOS_SCREEN_EXT_VERSION ||
                            (allow_v2 && (h->version == FRIXOS_SCREEN_EXT_VERSION_V3 ||
                                          h->version == FRIXOS_SCREEN_EXT_VERSION_V2));
    return h->magic == FRIXOS_SCREEN_EXT_MAGIC && version_ok &&
           h->count == TEXT_WIDGET_COUNT && h->item_size == sizeof(text_widget_cfg_t);
}

esp_err_t screen_layout_ext_decode(screen_layout_ext_wire_t *ext)
{
    if (!ext_header_ok(&ext->header, false))
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Bad layout ext section (magic 0x%08lx v%u)",
                    (unsigned long)ext->header.magic, ext->header.version);
        return ESP_ERR_INVALID_ARG;
    }
    text_widgets_sanitize(&ext->widgets);
    return ESP_OK;
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
    screen_layout_ext_header_t ext;
    screen_layout_ext_header(&ext);
    bool ok = fwrite(&header, 1, sizeof(header), f) == sizeof(header) &&
              fwrite(eeprom_screen_layout.profile, 1, profile_bytes, f) == profile_bytes &&
              fwrite(&ext, 1, sizeof(ext), f) == sizeof(ext) &&
              fwrite(&eeprom_text_widgets, 1, sizeof(eeprom_text_widgets), f) == sizeof(eeprom_text_widgets);
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

// Layouts older than the unified text widget (version <= 10) carry a different profile.
static bool load_legacy_v10(FILE *f, const screen_layout_bin_header_t *header)
{
    const size_t file_size = sizeof(*header) + FRIXOS_SCREEN_LAYOUT_PROFILES * sizeof(legacy_profile_v10_t);
    fseek(f, 0, SEEK_END);
    if ((size_t)ftell(f) < file_size)
        return false;
    fseek(f, (long)sizeof(*header), SEEK_SET);

    legacy_profile_v10_t *legacy = malloc(FRIXOS_SCREEN_LAYOUT_PROFILES * sizeof(*legacy));
    if (!legacy)
        return false;
    bool ok = fread(legacy, sizeof(*legacy), FRIXOS_SCREEN_LAYOUT_PROFILES, f) == FRIXOS_SCREEN_LAYOUT_PROFILES;
    if (ok)
    {
        memset(&eeprom_screen_layout, 0, sizeof(eeprom_screen_layout));
        screen_layout_migrate_v10(legacy, &eeprom_screen_layout, &eeprom_text_widgets);
        screen_layout_migrate_v11(&eeprom_screen_layout, &eeprom_text_widgets);
        eeprom_screen_layout.scroll_delay = (uint8_t)clamp_int(header->scroll_delay, 30, 255);
    }
    free(legacy);
    return ok;
}

static bool load_layout_v11(FILE *f, const screen_layout_bin_header_t *header)
{
    if (fread(eeprom_screen_layout.profile, 1, sizeof(eeprom_screen_layout.profile), f) !=
        sizeof(eeprom_screen_layout.profile))
        return false;

    memset(&eeprom_text_widgets, 0, sizeof(eeprom_text_widgets));
    screen_layout_ext_header_t ext;
    size_t got = fread(&ext, 1, sizeof(ext), f);
    if (got == 0 && feof(f))
        text_widgets_defaults(&eeprom_text_widgets);
    else
    {
        if (got != sizeof(ext) || !ext_header_ok(&ext, true))
            return false;
        const size_t want = ext.version >= FRIXOS_SCREEN_EXT_VERSION_V3 ? sizeof(eeprom_text_widgets)
                                                                        : TEXT_WIDGETS_NO_META_SIZE;
        if (fread(&eeprom_text_widgets, 1, want, f) != want)
            return false;
    }
    eeprom_screen_layout.scroll_delay = (uint8_t)clamp_int(header->scroll_delay, 30, 255);
    screen_layout_migrate_v11(&eeprom_screen_layout, &eeprom_text_widgets);
    return true;
}

void screen_layout_file_load(void)
{
    const char *layout_path = SCREEN_LAYOUT_FILE_PATH;
    FILE *f = fopen(layout_path, "rb");
    bool from_default = false;
    if (!f)
    {
        layout_path = SCREEN_DEFAULT_LAYOUT_FILE_PATH;
        f = fopen(layout_path, "rb");
        from_default = f != NULL;
        if (!f)
        {
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No layout file, using factory defaults");
            apply_factory_layout();
            screen_layout_nvs_erase();
            return;
        }
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "No current layout, loading bundled Default.layout");
    }

    screen_layout_bin_header_t header;
    bool ok = fread(&header, 1, sizeof(header), f) == sizeof(header) &&
              header.magic == FRIXOS_SCREEN_BIN_MAGIC &&
              header.format == FRIXOS_SCREEN_BIN_FORMAT &&
              header.layout_version <= FRIXOS_SCREEN_LAYOUT_VERSION;
    bool migrated = false;
    if (ok && header.layout_version <= 10)
    {
        ok = load_legacy_v10(f, &header);
        migrated = ok;
    }
    else if (ok && header.layout_version == 11)
    {
        ok = load_layout_v11(f, &header);
        migrated = ok;
    }
    else if (ok)
    {
        screen_layout_ext_header_t ext;
        ok = fread(eeprom_screen_layout.profile, 1, sizeof(eeprom_screen_layout.profile), f) ==
                 sizeof(eeprom_screen_layout.profile) &&
             fread(&ext, 1, sizeof(ext), f) == sizeof(ext) && ext_header_ok(&ext, true);
        if (ok)
        {
            // Version 2 files have no meta section; version 3+ includes it.
            const size_t want = ext.version >= FRIXOS_SCREEN_EXT_VERSION_V3 ? sizeof(eeprom_text_widgets)
                                                                            : TEXT_WIDGETS_NO_META_SIZE;
            memset(&eeprom_text_widgets, 0, sizeof(eeprom_text_widgets));
            ok = fread(&eeprom_text_widgets, 1, want, f) == want;
        }
        eeprom_screen_layout.scroll_delay = (uint8_t)clamp_int(header.scroll_delay, 30, 255);
    }
    fclose(f);

    if (!ok)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Invalid %s, using factory defaults", layout_path);
        if (!from_default)
            unlink(layout_path);
        apply_factory_layout();
        screen_layout_nvs_erase();
        return;
    }

    eeprom_screen_layout.version = FRIXOS_SCREEN_LAYOUT_VERSION;
    eeprom_screen_layout.reserved = 0;
    eeprom_weather_theme[0] = (uint8_t)clamp_int(header.reserved[0], 0, 12);
    eeprom_weather_theme[1] = (uint8_t)clamp_int(header.reserved[1], 0, 12);
    for (int pi = 0; pi < FRIXOS_SCREEN_LAYOUT_PROFILES; pi++)
        sanitize_profile(&eeprom_screen_layout.profile[pi], false);
    text_widgets_sanitize(&eeprom_text_widgets);
    text_widgets_instance_from_layout(&eeprom_text_widgets, &eeprom_screen_layout);
    screen_layout_sync_legacy_eeprom(&eeprom_screen_layout);
    screen_layout_nvs_erase();
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Loaded layout from %s", layout_path);

    if (from_default && screen_layout_file_save() != ESP_OK)
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Failed to save bundled Default.layout as current layout");

    if (migrated)
    {
        // Persist right away so the conversion runs once, then drop the old schedule keys.
        if (screen_layout_file_save() == ESP_OK)
        {
            screen_layout_migrate_v10_done();
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Migrated layout to the text widgets");
        }
    }
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

    const bool preserve_legacy_message = wire->header.layout_version == 11;
    for (int pi = 0; pi < FRIXOS_SCREEN_LAYOUT_PROFILES; pi++)
        sanitize_profile(&layout->profile[pi], preserve_legacy_message);

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
    eeprom_weather_theme[0] = (uint8_t)clamp_int(wire->header.reserved[0], 0, 12);
    eeprom_weather_theme[1] = (uint8_t)clamp_int(wire->header.reserved[1], 0, 12);

    return ESP_OK;
}
