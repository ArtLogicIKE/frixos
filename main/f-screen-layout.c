#include <string.h>
#include "frixos.h"
#include "f-text-widget.h"

void screen_layout_apply_factory_defaults(screen_layout_t *layout)
{
  memset(layout, 0, sizeof(*layout));
  layout->version = FRIXOS_SCREEN_LAYOUT_VERSION;
  layout->scroll_delay = eeprom_scroll_delay > 0 ? eeprom_scroll_delay : 60;

  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    screen_layout_profile_t *p = &layout->profile[profile];
    p->widget[SCREEN_ELEM_GLUCOSE_LEVEL] = (screen_widget_t){.enabled = 1, .x = 27, .y = 27, .z = 4};
    p->widget[SCREEN_ELEM_GLUCOSE_TREND] = (screen_widget_t){.enabled = 1, .x = 42, .y = 30, .z = 4};
    p->widget[SCREEN_ELEM_WIFI_OFF] = (screen_widget_t){.enabled = 1, .x = 22, .y = 27, .z = 4};
    // Weather visibility is now owned by this widget's .enabled flag (see the
    // show_object() gate in f-display.c). Seed it from the legacy quiet_weather
    // setting so a v66->v67 upgrade preserves a user who hid the forecast.
    p->widget[SCREEN_ELEM_WEATHER] = (screen_widget_t){.enabled = eeprom_quiet_weather ? 1 : 0, .x = 54, .y = 24, .z = 3};
    p->widget[SCREEN_ELEM_MOON] = (screen_widget_t){.enabled = 1, .x = 87, .y = 29, .z = 3};
    p->widget[SCREEN_ELEM_MESSAGE] = (screen_widget_t){0};
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
        .width = 80,
        .height = 36,
        .flags = GRAPH_FLAG_AUTOSCALE | GRAPH_FLAG_SHOW_AXIS | GRAPH_FLAG_SHOW_XAXIS | GRAPH_FLAG_SHOW_VALUE | GRAPH_FLAG_BACKFILL,
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

    // User icon slots: disabled by default, centred so they are visible when
    // first enabled. Source is S:/icon<N>.jpg (uploaded by the user).
    for (int i = 0; i < SCREEN_ICON_COUNT; i++)
      p->widget[SCREEN_ELEM_ICON_1 + i] = (screen_widget_t){.enabled = 0, .x = 48, .y = 48, .z = 2};
  }
  text_widgets_defaults(&eeprom_text_widgets);
  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    text_widget_cfg_t *scroll = &eeprom_text_widgets.widget[profile][2];
    scroll->enabled = eeprom_quiet_scroll ? 1 : 0;
    scroll->x = 0;
    scroll->y = 86;
    scroll->z = 0;
    scroll->width = LCD_H_RES;
    scroll->family = TW_FAMILY_SANS;
    scroll->size = 16;
    scroll->flags = TW_FLAG_SCROLL | TW_FLAG_SHOW_UNITS | TW_FLAG_COLOR_OVERRIDE;
    scroll->color_r = scroll->color_g = scroll->color_b = 255;
    eeprom_text_widgets.meta.instanced[profile] |= (uint16_t)(1u << 2);
    strncpy(eeprom_text_widgets.meta.name[profile][2], "Scroll", WIDGET_NAME_LEN - 1);
  }
}

void screen_layout_sync_legacy_eeprom(const screen_layout_t *layout)
{
  if (!layout)
    return;

  strncpy(eeprom_message, layout->profile[0].scroll_text, sizeof(eeprom_message) - 1);
  eeprom_message[sizeof(eeprom_message) - 1] = '\0';
  eeprom_scroll_delay = layout->scroll_delay > 0 ? layout->scroll_delay : 60;

  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    bool scroll_enabled = false;
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
      if (w->enabled && (w->flags & TW_FLAG_SCROLL))
      {
        scroll_enabled = true;
        break;
      }
    }
    if (profile == 0)
      eeprom_quiet_scroll = scroll_enabled;
  }
  eeprom_quiet_weather = layout->profile[0].widget[SCREEN_ELEM_WEATHER].enabled;
}

/* Reverse of screen_layout_sync_legacy_eeprom for the message field: legacy
 * POST /api/settings {"p16":...} clients (HA, scripts) must update the layout
 * engine's scroll_text, which is what the display actually renders. */
void screen_layout_apply_legacy_message(const char *message)
{
  if (!message)
    message = "";

  /* Ensure a valid layout exists without syncing legacy mirrors back into
   * eeprom_message — the caller has already written the new p16 value. */
  if (eeprom_screen_layout.version != FRIXOS_SCREEN_LAYOUT_VERSION)
    screen_layout_apply_factory_defaults(&eeprom_screen_layout);

  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    strncpy(eeprom_screen_layout.profile[profile].scroll_text, message, SCROLL_MSG_LENGTH - 1);
    eeprom_screen_layout.profile[profile].scroll_text[SCROLL_MSG_LENGTH - 1] = '\0';
  }
}

void screen_layout_set_scroll_enabled(bool enabled)
{
  for (int profile = 0; profile < FRIXOS_SCREEN_LAYOUT_PROFILES; profile++)
  {
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
      if (w->flags & TW_FLAG_SCROLL)
        w->enabled = enabled ? 1 : 0;
    }
  }
  eeprom_quiet_scroll = enabled ? 1 : 0;
}

void screen_layout_ensure_valid(void)
{
  if (eeprom_screen_layout.version == FRIXOS_SCREEN_LAYOUT_VERSION)
    return;

  screen_layout_apply_factory_defaults(&eeprom_screen_layout);
  screen_layout_sync_legacy_eeprom(&eeprom_screen_layout);
}
