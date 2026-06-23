#include <string.h>
#include "f-screen-layout-bin.h"
#include "frixos.h"
#include "esp_log.h"

static const char *TAG = "f-screen-bin";

_Static_assert(sizeof(screen_widget_t) == 13, "screen_widget_t wire size changed");
_Static_assert(sizeof(screen_graph_cfg_t) == 88, "screen_graph_cfg_t wire size changed");
_Static_assert(sizeof(screen_layout_profile_t) == 1820, "screen_layout_profile_t wire size changed");
_Static_assert(sizeof(screen_layout_bin_header_t) == 64, "screen_layout_bin_header_t size changed");
_Static_assert(FRIXOS_SCREEN_LAYOUT_WIRE_SIZE == 3704, "screen_layout_wire_t size changed");

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

bool screen_layout_wire_encode(uint8_t *out, size_t out_size, size_t *written)
{
    if (!out || out_size < FRIXOS_SCREEN_LAYOUT_WIRE_SIZE)
        return false;

    screen_layout_wire_t *wire = (screen_layout_wire_t *)out;
    memset(wire, 0, sizeof(*wire));

    wire->header.magic = FRIXOS_SCREEN_BIN_MAGIC;
    wire->header.format = FRIXOS_SCREEN_BIN_FORMAT;
    wire->header.layout_version = eeprom_screen_layout.version;
    wire->header.scroll_delay = eeprom_screen_layout.scroll_delay;
    wire->header.day_color_filter = eeprom_color_filter[0];
    wire->header.night_color_filter = eeprom_color_filter[1];
    copy_font_field(wire->header.day_font, sizeof(wire->header.day_font), eeprom_font[0]);
    copy_font_field(wire->header.night_font, sizeof(wire->header.night_font), eeprom_font[1]);
    copy_font_field(wire->header.day_aux_font, sizeof(wire->header.day_aux_font), eeprom_aux_font[0]);
    copy_font_field(wire->header.night_aux_font, sizeof(wire->header.night_aux_font), eeprom_aux_font[1]);
    wire->header.w = LCD_H_RES;
    wire->header.h = LCD_V_RES;

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
