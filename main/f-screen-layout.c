#include <string.h>
#include "frixos.h"

static const char *const k_default_static_texts[SCREEN_STATIC_TEXT_COUNT] = {
    "UV [uv]",
    "[pressure]",
    "Wind [wind]",
    "Gust [gust]",
    "Rain [precip]",
    "",
    "",
    "",
};

static void screen_layout_init_static_texts(screen_layout_profile_t *profile)
{
  for (int i = 0; i < SCREEN_STATIC_TEXT_COUNT; i++)
  {
    screen_widget_t *w = &profile->widget[SCREEN_ELEM_TEXT_1 + i];
    w->enabled = 0;
    w->x = 45;
    w->y = 90;
    w->z = (uint8_t)((1 + i) <= 4 ? (1 + i) : 4);
    w->font = 0;
    w->width = 80;
    w->align = SCREEN_MSG_ALIGN_LEFT;
    w->color_r = 255;
    w->color_g = 255;
    w->color_b = 255;
    w->bg_r = 0;
    w->bg_g = 0;
    w->bg_b = 0;
    strncpy(profile->static_text[i], k_default_static_texts[i], SCREEN_STATIC_TEXT_LENGTH - 1);
    profile->static_text[i][SCREEN_STATIC_TEXT_LENGTH - 1] = '\0';
  }
}

void screen_layout_apply_factory_defaults(screen_layout_t *layout)
{
  memset(layout, 0, sizeof(*layout));
  layout->version = FRIXOS_SCREEN_LAYOUT_VERSION;
  layout->scroll_delay = eeprom_scroll_delay > 0 ? eeprom_scroll_delay : 65;

  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    screen_layout_profile_t *p = &layout->profile[profile];
    p->widget[SCREEN_ELEM_GLUCOSE_LEVEL] = (screen_widget_t){.enabled = 1, .x = 27, .y = 27, .z = 4};
    p->widget[SCREEN_ELEM_GLUCOSE_TREND] = (screen_widget_t){.enabled = 1, .x = 42, .y = 30, .z = 4};
    p->widget[SCREEN_ELEM_WIFI_OFF] = (screen_widget_t){.enabled = 1, .x = 22, .y = 27, .z = 4};
    p->widget[SCREEN_ELEM_WEATHER] = (screen_widget_t){.enabled = 1, .x = 54, .y = 24, .z = 3};
    p->widget[SCREEN_ELEM_MOON] = (screen_widget_t){.enabled = 1, .x = 87, .y = 29, .z = 3};
    p->widget[SCREEN_ELEM_TIME] = (screen_widget_t){.enabled = 1, .x = 22, .y = 47, .z = 1};
    p->widget[SCREEN_ELEM_AMPM] = (screen_widget_t){.enabled = 1, .x = 101, .y = 54, .z = 2};
    p->widget[SCREEN_ELEM_TIME_AUX] = (screen_widget_t){.enabled = 0, .x = 22, .y = 47, .z = 1};
    p->widget[SCREEN_ELEM_DIGIT_LABEL] = (screen_widget_t){
        .enabled = 0,
        .x = 22,
        .y = 84,
        .z = 1,
        .font = 0,
        .width = 80,
        .align = SCREEN_MSG_ALIGN_LEFT,
        .color_r = 255,
        .color_g = 255,
        .color_b = 255,
        .bg_r = 0,
        .bg_g = 0,
        .bg_b = 0,
    };
    p->widget[SCREEN_ELEM_DIGIT_LABEL_AUX] = (screen_widget_t){
        .enabled = 0,
        .x = 22,
        .y = 84,
        .z = 1,
        .font = 0,
        .width = 80,
        .align = SCREEN_MSG_ALIGN_LEFT,
        .color_r = 255,
        .color_g = 255,
        .color_b = 255,
        .bg_r = 0,
        .bg_g = 0,
        .bg_b = 0,
    };
    p->digit_label_text[0] = '\0';
    p->digit_label_aux_text[0] = '\0';
    p->widget[SCREEN_ELEM_MESSAGE] = (screen_widget_t){
        .enabled = 1,
        .x = 0,
        .y = 86,
        .z = 0,
        .font = eeprom_msg_font <= 4 ? eeprom_msg_font : 0,
        .align = SCREEN_MSG_ALIGN_LEFT,
        .color_r = eeprom_msg_red[profile],
        .color_g = eeprom_msg_green[profile],
        .color_b = eeprom_msg_blue[profile],
        .bg_r = 0,
        .bg_g = 0,
        .bg_b = 0,
    };
    p->widget[SCREEN_ELEM_GRAPH] = (screen_widget_t){
        .enabled = 0,
        .x = 4,
        .y = 38,
        .z = 1,
        .color_r = 0,
        .color_g = 220,
        .color_b = 255, // cyan line
        .bg_r = 0,
        .bg_g = 0,
        .bg_b = 0,
    };
    p->graph = (screen_graph_cfg_t){
        .token = "",
        .interval_min = 5,
        .points = 60,
        .width = 90,
        .height = 40,
        .flags = GRAPH_FLAG_AUTOSCALE | GRAPH_FLAG_SHOW_AXIS | GRAPH_FLAG_SHOW_VALUE | GRAPH_FLAG_BACKFILL,
        .band_low = GRAPH_VAL_UNSET,
        .band_high = GRAPH_VAL_UNSET,
        .y_min = GRAPH_VAL_UNSET,
        .y_max = GRAPH_VAL_UNSET,
        .band_r = 40,
        .band_g = 60,
        .band_b = 40,
        .warn_r = 255,
        .warn_g = 80,
        .warn_b = 80,
        .axis_r = 120,
        .axis_g = 120,
        .axis_b = 120,
    };
    strncpy(p->scroll_text, eeprom_message, SCROLL_MSG_LENGTH - 1);
    p->scroll_text[SCROLL_MSG_LENGTH - 1] = '\0';
    screen_layout_init_static_texts(p);
  }
}

void screen_layout_sync_legacy_eeprom(const screen_layout_t *layout)
{
  if (!layout)
    return;

  strncpy(eeprom_message, layout->profile[0].scroll_text, sizeof(eeprom_message) - 1);
  eeprom_message[sizeof(eeprom_message) - 1] = '\0';
  eeprom_scroll_delay = layout->scroll_delay > 0 ? layout->scroll_delay : 65;

  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    const screen_widget_t *msg = &layout->profile[profile].widget[SCREEN_ELEM_MESSAGE];
    eeprom_msg_red[profile] = msg->color_r;
    eeprom_msg_green[profile] = msg->color_g;
    eeprom_msg_blue[profile] = msg->color_b;
  }

  eeprom_msg_font = layout->profile[0].widget[SCREEN_ELEM_MESSAGE].font <= 4
                        ? layout->profile[0].widget[SCREEN_ELEM_MESSAGE].font
                        : 0;
  eeprom_quiet_scroll = layout->profile[0].widget[SCREEN_ELEM_MESSAGE].enabled;
}

void screen_layout_ensure_valid(void)
{
  if (eeprom_screen_layout.version == FRIXOS_SCREEN_LAYOUT_VERSION)
    return;

  screen_layout_apply_factory_defaults(&eeprom_screen_layout);
  screen_layout_sync_legacy_eeprom(&eeprom_screen_layout);
}
