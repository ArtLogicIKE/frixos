#ifndef F_TEXT_WIDGET_H
#define F_TEXT_WIDGET_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "frixos.h"

// Unified text/value widget: one LVGL label per instance rendering a token template
// ("[temp]", "[time]", "Gluc [CGM:reading]") with either a bitmap font family
// (8-36 px) or a digit sprite theme.
#define TEXT_WIDGET_COUNT 8
#define TEXT_WIDGET_TEXT_LEN 64
#define TEXT_WIDGET_SLOTS 3 // contents a widget can alternate between
#define TEXT_WIDGET_DURATION_DEFAULT 10
#define TEXT_WIDGET_DURATION_MAX 3600
#define TEXT_WIDGET_SIZE_MIN 8
#define TEXT_WIDGET_SIZE_MAX 36

// family: bitmap families first, then digit sprite themes.
#define TW_FAMILY_SANS 0
#define TW_FAMILY_BOLD 1
#define TW_FAMILY_SPRITE_THEME 2 // the profile's digit theme (day/night font)
#define TW_FAMILY_SPRITE_AUX 3   // the profile's aux digit theme
#define TW_FAMILY_SPRITE_FIRST 4 // 4 + index in the theme list (bold, light, lcd, ...)

#define TW_FLAG_SHOW_UNITS 0x01     // keep the unit that follows a token's number
#define TW_FLAG_TABULAR 0x02        // equal-width digits (bitmap families)
#define TW_FLAG_TRANSPARENT_BG 0x04 // no background fill (bitmap families)
#define TW_FLAG_COLOR_OVERRIDE 0x08 // use the configured text/sprite tint instead of defaults
#define TW_FLAG_SCROLL 0x10         // scroll long text through the profile's scroll_text buffer

typedef struct __attribute__((packed))
{
  uint8_t enabled;
  uint8_t x, y, z;
  uint8_t width; // px, 0 = fit the rendered content
  uint8_t align; // SCREEN_MSG_ALIGN_*
  uint8_t family;
  uint8_t size;  // px, TEXT_WIDGET_SIZE_MIN..MAX
  uint8_t flags; // TW_FLAG_*
  uint8_t slot_count; // 1..TEXT_WIDGET_SLOTS; above 1 the contents rotate
  uint8_t color_r, color_g, color_b;
  uint8_t bg_r, bg_g, bg_b;
  uint16_t duration[TEXT_WIDGET_SLOTS]; // seconds each content stays up (rotation only)
  uint8_t reserved[2];
  char text[TEXT_WIDGET_SLOTS][TEXT_WIDGET_TEXT_LEN];
} text_widget_cfg_t;

// Editor-only data kept with the layout: which text/icon slots exist (enabled or not) and
// their user-given names. The display never reads it.
#define WIDGET_NAME_LEN 16
#define WIDGET_META_SLOTS (TEXT_WIDGET_COUNT + SCREEN_ICON_COUNT) // text 0..7, then icons 8..15

typedef struct __attribute__((packed))
{
  uint16_t instanced[FRIXOS_SCREEN_LAYOUT_PROFILES]; // bit i set: slot i exists
  char name[FRIXOS_SCREEN_LAYOUT_PROFILES][WIDGET_META_SLOTS][WIDGET_NAME_LEN];
} widget_meta_t;

typedef struct __attribute__((packed))
{
  text_widget_cfg_t widget[FRIXOS_SCREEN_LAYOUT_PROFILES][TEXT_WIDGET_COUNT];
  widget_meta_t meta;
} text_widgets_layout_t;

// Bytes of text_widgets_layout_t before meta (the whole payload of ext version 2).
#define TEXT_WIDGETS_NO_META_SIZE offsetof(text_widgets_layout_t, meta)

extern text_widgets_layout_t eeprom_text_widgets;

void text_widgets_defaults(text_widgets_layout_t *l);
void text_widgets_sanitize(text_widgets_layout_t *l);
// Mark every enabled text/icon widget of `layout` as an existing slot.
void text_widgets_instance_from_layout(text_widgets_layout_t *l, const screen_layout_t *layout);

// Display side. init/apply/raise must run under the LVGL lock; refresh takes it itself.
void text_widgets_init(void);
// Rebuild fonts, styles and positions for `profile`; `visible` gates every instance.
void text_widgets_apply(uint8_t profile, bool visible);
// Re-expand tokens and update labels whose text changed.
void text_widgets_refresh(uint8_t profile, bool visible);
// ~1 Hz: switch rotating widgets to their next content when its duration has elapsed.
void text_widgets_rotate_tick(uint8_t profile, bool visible);
// Bring the instances of `profile` on layer z to the foreground (z-order pass).
void text_widgets_raise_layer(uint8_t profile, uint8_t z);

// Expand a template; with show_units false only the numeric part of each token is kept.
void text_widget_render(const char *tpl, bool show_units, char *out, size_t out_size);

// Widget-only tokens. [colon]: a colon that animates like the legacy time dots (cross-fading
// dots on digit themes, once-per-second blink on bitmap fonts), steady when "disable breathing"
// is set. [clock]: the time as digits with that colon, honouring the 12/24 h and leading-zero
// settings. [ampm12]: am/pm, empty in 24 h mode.
#define TW_TOKEN_COLON "[colon]"
#define TW_TOKEN_CLOCK "[clock]"
#define TW_TOKEN_AMPM12 "[ampm12]"
#define TW_COLON_ANIM_UTF8 "\xEE\x80\x80" /* U+E000, see SPRITE_FONT_COLON_ANIM */

// Advance the colon animation (top dot opacity 0..255); takes the LVGL lock itself.
void text_widgets_colon_tick(uint8_t top_dot_opacity);

#endif
