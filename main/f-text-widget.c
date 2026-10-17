#include "f-text-widget.h"
#include "f-display.h"
#include "f-fonts.h"
#include "f-sprite-font.h"
#include "f-time.h"
#include "esp_lvgl_port.h"
#include "esp_timer.h"
#include "draw/lv_draw_rect.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

text_widgets_layout_t eeprom_text_widgets;

// Same order as SCREEN_TIME_FONTS in spiffs/js/screen-editor.js.
static const char *const k_themes[] = {
    "bold", "light", "lcd", "nixie", "robrito", "ficasso",
    "lichten", "kablame", "kablamo", "kaboom", "user1", "user2"};
#define THEME_COUNT (sizeof(k_themes) / sizeof(k_themes[0]))
#define FAMILY_MAX (TW_FAMILY_SPRITE_FIRST + THEME_COUNT - 1)

// Copy of a bitmap font that gives every digit the advance of the widest one (optional),
// so changing values don't shift the text around, and that renders the animated colon.
typedef struct
{
  lv_font_t font;
  const lv_font_t *base;
  uint16_t digit_adv;
  bool tabular;
} tabular_font_t;

#define MAX_COLONS 4

typedef struct
{
  lv_obj_t *label;
  tabular_font_t tab;
  sprite_font_t *sprite; // allocated only while the instance uses a sprite theme
  const lv_font_t *font;
  bool draw_trimmed_bg;
  uint32_t text_hash;
  int8_t slot; // content currently shown (rotation)
  uint8_t colon_count;
  uint16_t colon_idx[MAX_COLONS]; // character index of each animated colon in the label text
} tw_runtime_t;

static tw_runtime_t s_rt[TEXT_WIDGET_COUNT];
// Bitmap fonts blink the colon once per second; sprite fonts cross-fade its two dots.
static volatile bool s_colon_visible = true;

static uint32_t text_next_codepoint(const char *text, size_t text_length, size_t *offset)
{
  const uint8_t first = (uint8_t)text[*offset];
  if (first < 0x80)
  {
    (*offset)++;
    return first;
  }

  uint32_t codepoint;
  size_t length;
  if ((first & 0xE0) == 0xC0)
  {
    codepoint = first & 0x1F;
    length = 2;
  }
  else if ((first & 0xF0) == 0xE0)
  {
    codepoint = first & 0x0F;
    length = 3;
  }
  else if ((first & 0xF8) == 0xF0)
  {
    codepoint = first & 0x07;
    length = 4;
  }
  else
  {
    (*offset)++;
    return first;
  }

  if (text_length - *offset < length)
  {
    (*offset)++;
    return first;
  }

  for (size_t i = 1; i < length; i++)
  {
    const uint8_t continuation = (uint8_t)text[*offset + i];
    if ((continuation & 0xC0) != 0x80)
    {
      (*offset)++;
      return first;
    }
    codepoint = (codepoint << 6) | (continuation & 0x3F);
  }
  *offset += length;
  return codepoint;
}

static int32_t text_widget_bg_height(const tw_runtime_t *rt, const char *text)
{
  if (rt->sprite)
    return rt->font->line_height - rt->font->base_line;

  int32_t bottom = 0;
  const size_t text_length = strlen(text);
  size_t offset = 0;
  while (offset < text_length)
  {
    const uint32_t codepoint = text_next_codepoint(text, text_length, &offset);
    if (codepoint == '\n' || codepoint == '\r')
      continue;

    lv_font_glyph_dsc_t glyph;
    if (!rt->font->get_glyph_dsc(rt->font, &glyph, codepoint, 0) || glyph.box_w == 0 || glyph.box_h == 0)
      continue;
    const int32_t glyph_bottom = rt->font->line_height - rt->font->base_line - glyph.ofs_y;
    if (glyph_bottom > bottom)
      bottom = glyph_bottom;
  }

  if (bottom < 0)
    return 0;
  if (bottom > rt->font->line_height)
    return rt->font->line_height;
  return bottom;
}

static void text_widget_draw_trimmed_bg(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_DRAW_MAIN_BEGIN)
    return;

  tw_runtime_t *rt = lv_event_get_user_data(e);
  if (rt == NULL || !rt->draw_trimmed_bg || rt->font == NULL)
    return;

  lv_obj_t *label = lv_event_get_current_target(e);
  const int32_t height = text_widget_bg_height(rt, lv_label_get_text(label));
  if (height <= 0)
    return;

  lv_area_t bg_coords;
  lv_obj_get_coords(label, &bg_coords);
  bg_coords.y2 = bg_coords.y1 + height - 1;

  lv_draw_rect_dsc_t draw_dsc;
  lv_draw_rect_dsc_init(&draw_dsc);
  draw_dsc.base.layer = lv_event_get_layer(e);
  draw_dsc.bg_color = lv_obj_get_style_bg_color_filtered(label, LV_PART_MAIN);
  draw_dsc.bg_opa = LV_OPA_COVER;
  draw_dsc.radius = lv_obj_get_style_radius(label, LV_PART_MAIN);
  lv_draw_rect(lv_event_get_layer(e), &draw_dsc, &bg_coords);
}

void text_widgets_defaults(text_widgets_layout_t *l)
{
  memset(l, 0, sizeof(*l));
  for (int p = 0; p < FRIXOS_SCREEN_LAYOUT_PROFILES; p++)
  {
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      l->widget[p][i] = (text_widget_cfg_t){
          .enabled = 0,
          .x = 4,
          .y = (uint8_t)(4 + (i % 4) * 30),
          .z = 2,
          .width = 0,
          .align = SCREEN_MSG_ALIGN_LEFT,
          .family = TW_FAMILY_SANS,
          .size = 16,
          .flags = TW_FLAG_SHOW_UNITS | TW_FLAG_TABULAR,
          .slot_count = 1,
          .duration = {TEXT_WIDGET_DURATION_DEFAULT, TEXT_WIDGET_DURATION_DEFAULT, TEXT_WIDGET_DURATION_DEFAULT},
          .color_r = 255,
          .color_g = 255,
          .color_b = 255,
      };
    }
    // Out of the box: the clock in the profile's digit theme, and its AM/PM mark.
    text_widget_cfg_t *clock = &l->widget[p][0];
    clock->enabled = 1;
    clock->x = 22;
    clock->y = 47;
    clock->z = 1;
    clock->width = 0;
    clock->align = SCREEN_MSG_ALIGN_CENTER;
    clock->family = TW_FAMILY_SPRITE_THEME;
    clock->size = TEXT_WIDGET_SIZE_MAX;
    clock->flags = 0;
    strcpy(clock->text[0], TW_TOKEN_CLOCK);
    strcpy(l->meta.name[p][0], "Clock");
    text_widget_cfg_t *ampm = &l->widget[p][1];
    ampm->enabled = 1;
    ampm->x = 101;
    ampm->y = 54;
    ampm->z = 2;
    ampm->width = 25;
    ampm->size = 12;
    ampm->flags = TW_FLAG_SHOW_UNITS | TW_FLAG_TRANSPARENT_BG;
    strcpy(ampm->text[0], TW_TOKEN_AMPM12);
    strcpy(l->meta.name[p][1], "AMPM");
    l->meta.instanced[p] = 0x3; // the two widgets above
  }
}

static uint8_t clamp_u8(int v, int lo, int hi)
{
  return (uint8_t)(v < lo ? lo : (v > hi ? hi : v));
}

void text_widgets_sanitize(text_widgets_layout_t *l)
{
  for (int p = 0; p < FRIXOS_SCREEN_LAYOUT_PROFILES; p++)
  {
    bool scroll_assigned = false;
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      text_widget_cfg_t *w = &l->widget[p][i];
      w->enabled = w->enabled ? 1 : 0;
      w->x = clamp_u8(w->x, 0, LCD_H_RES - 1);
      w->y = clamp_u8(w->y, 0, LCD_V_RES - 1);
      w->z = clamp_u8(w->z, 0, 4);
      w->width = clamp_u8(w->width, 0, LCD_H_RES);
      w->align = clamp_u8(w->align, SCREEN_MSG_ALIGN_LEFT, SCREEN_MSG_ALIGN_RIGHT);
      w->family = clamp_u8(w->family, 0, FAMILY_MAX);
      w->size = clamp_u8(w->size, TEXT_WIDGET_SIZE_MIN, TEXT_WIDGET_SIZE_MAX);
      if (w->flags & TW_FLAG_SCROLL)
      {
        if (scroll_assigned)
          w->flags &= (uint8_t)~TW_FLAG_SCROLL;
        else
        {
          scroll_assigned = true;
          if (w->family > TW_FAMILY_BOLD)
            w->family = TW_FAMILY_SANS;
          if (w->size > 20)
            w->size = 20;
        }
      }
      if (!(w->flags & TW_FLAG_COLOR_OVERRIDE) &&
          (w->color_r != 255 || w->color_g != 255 || w->color_b != 255))
        w->flags |= TW_FLAG_COLOR_OVERRIDE;
      w->flags &= TW_FLAG_SHOW_UNITS | TW_FLAG_TABULAR | TW_FLAG_TRANSPARENT_BG |
                  TW_FLAG_COLOR_OVERRIDE | TW_FLAG_SCROLL;
      w->slot_count = clamp_u8(w->slot_count, 1, TEXT_WIDGET_SLOTS);
      memset(w->reserved, 0, sizeof(w->reserved));
      for (int s = 0; s < TEXT_WIDGET_SLOTS; s++)
      {
        const uint16_t d = w->duration[s];
        w->duration[s] = d == 0 ? TEXT_WIDGET_DURATION_DEFAULT : (d > TEXT_WIDGET_DURATION_MAX ? TEXT_WIDGET_DURATION_MAX : d);
        w->text[s][TEXT_WIDGET_TEXT_LEN - 1] = '\0';
      }
      if (w->enabled)
        l->meta.instanced[p] |= (uint16_t)(1u << i);
    }
    for (int s = 0; s < WIDGET_META_SLOTS; s++)
      l->meta.name[p][s][WIDGET_NAME_LEN - 1] = '\0';
  }
}

void text_widgets_instance_from_layout(text_widgets_layout_t *l, const screen_layout_t *layout)
{
  for (int p = 0; p < FRIXOS_SCREEN_LAYOUT_PROFILES; p++)
  {
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
      if (l->widget[p][i].enabled)
        l->meta.instanced[p] |= (uint16_t)(1u << i);
    for (int i = 0; i < SCREEN_ICON_COUNT; i++)
      if (layout->profile[p].widget[SCREEN_ELEM_ICON_1 + i].enabled)
        l->meta.instanced[p] |= (uint16_t)(1u << (TEXT_WIDGET_COUNT + i));
  }
}

// Keep the leading number of a token value ("21°C" -> "21", "5.6 mmol/l" -> "5.6",
// "12:34pm" -> "12:34"); values that don't start with a number are left alone.
static void strip_units(char *s)
{
  char *p = s;
  while (*p == ' ')
    p++;
  char *q = p;
  if (*q == '-' || *q == '+')
    q++;
  if (*q < '0' || *q > '9')
    return;
  while ((*q >= '0' && *q <= '9') || *q == '.' || *q == ',' || *q == ':')
    q++;
  while (q > p && (q[-1] == '.' || q[-1] == ',' || q[-1] == ':'))
    q--;
  memmove(s, p, (size_t)(q - p));
  s[q - p] = '\0';
}

void text_widget_render(const char *tpl, bool show_units, char *out, size_t out_size)
{
  if (!out || out_size == 0)
    return;
  out[0] = '\0';
  if (!tpl)
    return;

  size_t n = 0;
  while (*tpl && n + 1 < out_size)
  {
    const char *close = (*tpl == '[') ? strchr(tpl, ']') : NULL;
    if (!close || close - tpl >= TEXT_WIDGET_TEXT_LEN)
    {
      out[n++] = *tpl++;
      continue;
    }
    // Expand one token at a time so units can be stripped per value.
    char tok[TEXT_WIDGET_TEXT_LEN];
    char val[TEXT_WIDGET_TEXT_LEN];
    const size_t tl = (size_t)(close - tpl) + 1;
    memcpy(tok, tpl, tl);
    tok[tl] = '\0';
    if (strcmp(tok, TW_TOKEN_COLON) == 0)
    {
      // Same switch as the legacy digits: "disable breathing dots" keeps a steady colon.
      strcpy(val, eeprom_dots_breathe ? ":" : TW_COLON_ANIM_UTF8);
    }
    else if (strcmp(tok, TW_TOKEN_CLOCK) == 0)
    {
      time_t t = time(NULL);
      struct tm lt;
      localtime_r(&t, &lt);
      int hour = lt.tm_hour;
      if (eeprom_12hour)
        hour = hour == 0 ? 12 : (hour > 12 ? hour - 12 : hour);
      snprintf(val, sizeof(val), eeprom_show_leading_zero ? "%02d%s%02d" : "%d%s%02d", hour,
               eeprom_dots_breathe ? ":" : TW_COLON_ANIM_UTF8, lt.tm_min);
    }
    else if (strcmp(tok, TW_TOKEN_AMPM12) == 0)
    {
      time_t t = time(NULL);
      struct tm lt;
      localtime_r(&t, &lt);
      strlcpy(val, eeprom_12hour ? time_localized_ampm(&lt) : "", sizeof(val));
    }
    else
    {
      replace_placeholders(tok, val, sizeof(val));
      if (!show_units && strcmp(val, tok) != 0)
        strip_units(val);
    }
    size_t vl = strnlen(val, out_size - 1 - n);
    while (vl > 0 && val[vl] != '\0' && ((uint8_t)val[vl] & 0xC0) == 0x80)
      vl--; // don't cut a UTF-8 sequence when the output is full
    memcpy(out + n, val, vl);
    n += vl;
    tpl = close + 1;
  }
  out[n] = '\0';
}

static bool tabular_get_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *g, uint32_t letter, uint32_t next)
{
  const tabular_font_t *t = font->user_data;
  if (letter == SPRITE_FONT_COLON_ANIM)
  {
    if (!t->base->get_glyph_dsc(font, g, ':', 0))
      return false;
    if (!s_colon_visible)
      g->box_w = 0; // keeps the advance, draws nothing
    return true;
  }
  if (!t->base->get_glyph_dsc(font, g, letter, next))
    return false;
  if (t->tabular && letter >= '0' && letter <= '9' && g->adv_w < t->digit_adv)
  {
    g->ofs_x += (int16_t)((t->digit_adv - g->adv_w) / 2);
    g->adv_w = t->digit_adv;
  }
  return true;
}

static const lv_font_t *tabular_font(tabular_font_t *t, const lv_font_t *base, bool tabular)
{
  t->tabular = tabular;
  if (t->base != base)
  {
    // The copy keeps base->dsc, so the stock fmt_txt callbacks work on it unchanged.
    t->font = *base;
    t->font.get_glyph_dsc = tabular_get_glyph_dsc;
    t->font.user_data = t;
    t->base = base;
    t->digit_adv = 0;
    for (uint32_t c = '0'; c <= '9'; c++)
    {
      lv_font_glyph_dsc_t g;
      if (base->get_glyph_dsc(base, &g, c, 0) && g.adv_w > t->digit_adv)
        t->digit_adv = g.adv_w;
    }
  }
  return &t->font;
}

static const char *sprite_theme(uint8_t family, uint8_t profile)
{
  if (family == TW_FAMILY_SPRITE_THEME)
    return eeprom_font[profile];
  if (family == TW_FAMILY_SPRITE_AUX)
    return eeprom_aux_font[profile];
  return k_themes[family - TW_FAMILY_SPRITE_FIRST];
}

static void drop_sprite(tw_runtime_t *rt)
{
  if (!rt->sprite)
    return;
  sprite_font_release(rt->sprite);
  free(rt->sprite);
  rt->sprite = NULL;
}

// Returns the font for w and whether it is a sprite theme.
static const lv_font_t *resolve_font(tw_runtime_t *rt, const text_widget_cfg_t *w, uint8_t profile, bool *sprite)
{
  *sprite = false;
  const bool color_override = (w->flags & TW_FLAG_COLOR_OVERRIDE) != 0;
  const uint8_t color_r = color_override ? w->color_r : 255;
  const uint8_t color_g = color_override ? w->color_g : 255;
  const uint8_t color_b = color_override ? w->color_b : 255;
  if (w->family >= TW_FAMILY_SPRITE_THEME)
  {
    // Units and other non-digit characters use the sans font whose capitals match the
    // sprite digits (their ink fills about 34/36 of the cell height).
    const lv_font_t *fallback = font_registry_match_cap((uint8_t)((w->size * 34 + 18) / 36));
    if (!rt->sprite)
      rt->sprite = malloc(sizeof(sprite_font_t));
    if (rt->sprite && sprite_font_setup(rt->sprite, sprite_theme(w->family, profile), w->size, fallback,
              !color_override, color_r, color_g, color_b))
    {
      *sprite = true;
      return &rt->sprite->font;
    }
    drop_sprite(rt);
    return font_registry_get(FONT_FAMILY_SANS, w->size);
  }

  drop_sprite(rt);
  return tabular_font(&rt->tab, font_registry_get(w->family, w->size), (w->flags & TW_FLAG_TABULAR) != 0);
}

void text_widgets_init(void)
{
  sprite_font_decoder_init();
}

void text_widgets_apply(uint8_t profile, bool visible)
{
  if (profile >= FRIXOS_SCREEN_LAYOUT_PROFILES)
    return;

  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    tw_runtime_t *rt = &s_rt[i];
    const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
    const bool color_override = (w->flags & TW_FLAG_COLOR_OVERRIDE) != 0;
    const uint8_t color_r = color_override ? w->color_r : 255;
    const uint8_t color_g = color_override ? w->color_g : 255;
    const uint8_t color_b = color_override ? w->color_b : 255;

    if (!w->enabled)
    {
      if (rt->label)
        lv_obj_add_flag(rt->label, LV_OBJ_FLAG_HIDDEN);
      drop_sprite(rt);
      continue;
    }

    if (w->flags & TW_FLAG_SCROLL)
    {
      if (rt->label)
        lv_obj_add_flag(rt->label, LV_OBJ_FLAG_HIDDEN);
      drop_sprite(rt);
      continue;
    }

    if (!rt->label)
    {
      // Created on first use so unused instances cost no LVGL memory.
      rt->label = lv_label_create(lv_scr_act());
      lv_label_set_long_mode(rt->label, LV_LABEL_LONG_CLIP);
      lv_obj_set_style_pad_all(rt->label, 0, LV_PART_MAIN);
      lv_label_set_text_static(rt->label, "");
      lv_obj_add_event_cb(rt->label, text_widget_draw_trimmed_bg, LV_EVENT_DRAW_MAIN_BEGIN, rt);
    }

    bool sprite;
    const lv_font_t *font = resolve_font(rt, w, profile, &sprite);
    rt->font = font;
    lv_obj_t *l = rt->label;
    lv_obj_set_style_text_font(l, font, LV_PART_MAIN);
    lv_obj_set_style_text_color(l, lv_color_make(color_r, color_g, color_b), LV_PART_MAIN);

    // Sprite cells carry their own (black) background, so the fill is forced to match.
    if (sprite)
    {
      rt->draw_trimmed_bg = true;
      lv_obj_set_style_bg_color(l, lv_color_black(), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, LV_PART_MAIN);
    }
    else
    {
      rt->draw_trimmed_bg = (w->flags & TW_FLAG_TRANSPARENT_BG) == 0;
      lv_obj_set_style_bg_color(l, lv_color_make(w->bg_r, w->bg_g, w->bg_b), LV_PART_MAIN);
      lv_obj_set_style_bg_opa(l, LV_OPA_TRANSP, LV_PART_MAIN);
    }

    lv_text_align_t align = LV_TEXT_ALIGN_LEFT;
    if (w->align == SCREEN_MSG_ALIGN_CENTER)
      align = LV_TEXT_ALIGN_CENTER;
    else if (w->align == SCREEN_MSG_ALIGN_RIGHT)
      align = LV_TEXT_ALIGN_RIGHT;
    lv_obj_set_style_text_align(l, align, LV_PART_MAIN);

    int width = w->width > 0 ? w->width : LCD_H_RES - w->x;
    lv_obj_set_size(l, width > 0 ? width : 1, font->line_height);
    lv_obj_set_pos(l, w->x, w->y);

    if (visible)
      lv_obj_remove_flag(l, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
  }
}

static uint32_t fnv1a(const char *s)
{
  uint32_t h = 2166136261u;
  while (*s)
    h = (h ^ (uint8_t)*s++) * 16777619u;
  return h;
}

// Content to show now. Slots rotate on wall-clock time (every instance stays in step and no
// state is kept). A slot is skipped when it is empty or has nothing to show right now (a value
// token without data renders as "-"), like the old display schedule skipped unavailable slots.
static int current_slot(const text_widget_cfg_t *w)
{
  const int n = w->slot_count < TEXT_WIDGET_SLOTS ? w->slot_count : TEXT_WIDGET_SLOTS;
  bool usable[TEXT_WIDGET_SLOTS] = {false};
  uint32_t total = 0;
  int first = 0;
  bool have_first = false;
  for (int s = 0; s < n; s++)
  {
    if (w->text[s][0] == '\0')
      continue;
    if (n > 1)
    {
      char probe[TEXT_WIDGET_TEXT_LEN * 2];
      text_widget_render(w->text[s], false, probe, sizeof(probe));
      if (probe[0] == '\0' || (probe[0] == '-' && probe[1] == '\0'))
        continue;
    }
    usable[s] = true;
    if (!have_first)
    {
      first = s;
      have_first = true;
    }
    total += w->duration[s];
  }
  if (!have_first)
  {
    // Nothing usable: show the first non-empty slot so the widget is not left blank by accident.
    for (int s = 0; s < n; s++)
      if (w->text[s][0] != '\0')
        return s;
    return 0;
  }
  if (n <= 1 || total == 0)
    return first;
  uint32_t pos = (uint32_t)time(NULL) % total;
  for (int s = 0; s < n; s++)
  {
    if (!usable[s])
      continue;
    if (pos < w->duration[s])
      return s;
    pos -= w->duration[s];
  }
  return first;
}

static void refresh_one(int i, uint8_t profile, bool visible)
{
  static char text[TEXT_WIDGET_TEXT_LEN * 2];
  tw_runtime_t *rt = &s_rt[i];
  const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
  if (!rt->label || !w->enabled)
    return;

  // Token expansion runs outside the LVGL lock, like the static text labels.
  const int slot = current_slot(w);
  text_widget_render(w->text[slot], (w->flags & TW_FLAG_SHOW_UNITS) != 0, text, sizeof(text));
  const uint32_t h = fnv1a(text);

  lvgl_port_lock(0);
  rt->slot = (int8_t)slot;
  if (h != rt->text_hash)
  {
    lv_label_set_text(rt->label, text);
    rt->text_hash = h;
    // Character index (not byte offset) of each animated colon, for partial redraws.
    rt->colon_count = 0;
    uint16_t chars = 0;
    for (const char *p = text; *p; p++)
    {
      if (((uint8_t)*p & 0xC0) == 0x80)
        continue;
      if (strncmp(p, TW_COLON_ANIM_UTF8, 3) == 0 && rt->colon_count < MAX_COLONS)
        rt->colon_idx[rt->colon_count++] = chars;
      chars++;
    }
  }
  int width = w->width;
  if (width == 0)
  {
    lv_point_t text_size;
    lv_text_get_size(&text_size, text, rt->font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    width = text_size.x > 0 ? text_size.x : 1;
  }
  lv_obj_set_size(rt->label, width, rt->font->line_height);
  if (visible)
    lv_obj_remove_flag(rt->label, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(rt->label, LV_OBJ_FLAG_HIDDEN);
  lvgl_port_unlock();
}

void text_widgets_refresh(uint8_t profile, bool visible)
{
  if (profile >= FRIXOS_SCREEN_LAYOUT_PROFILES)
    return;

  static bool scroll_was_active = false;
  bool scroll_found = false;
  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
    if (!(w->flags & TW_FLAG_SCROLL))
      continue;
    scroll_found = true;
    if (w->enabled && visible)
    {
      char text[SCROLL_MSG_LENGTH];
      text_widget_render(eeprom_screen_layout.profile[profile].scroll_text,
                         (w->flags & TW_FLAG_SHOW_UNITS) != 0, text, sizeof(text));
      set_scroll_widget_message(text, w, true);
      scroll_was_active = true;
    }
    else
    {
      set_scroll_widget_message(NULL, w, false);
      scroll_was_active = false;
    }
    break;
  }
  if (!scroll_found && scroll_was_active)
  {
    set_scroll_widget_message(NULL, NULL, false);
    scroll_was_active = false;
  }

  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    if (eeprom_text_widgets.widget[profile][i].flags & TW_FLAG_SCROLL)
      continue;
    refresh_one(i, profile, visible);
  }
}

void text_widgets_rotate_tick(uint8_t profile, bool visible)
{
  if (profile >= FRIXOS_SCREEN_LAYOUT_PROFILES)
    return;
  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
    if (s_rt[i].label && w->enabled && w->slot_count > 1 && current_slot(w) != s_rt[i].slot)
      refresh_one(i, profile, visible);
  }
}

void text_widgets_colon_tick(uint8_t top_dot_opacity)
{
  const bool visible = ((esp_timer_get_time() / 1000000) & 1) == 0;
  const bool blink_changed = visible != s_colon_visible;
  s_colon_visible = visible;
  const bool phase_changed = top_dot_opacity != sprite_font_colon_phase;
  sprite_font_colon_phase = top_dot_opacity;

  lvgl_port_lock(0);
  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    tw_runtime_t *rt = &s_rt[i];
    if (!rt->label || !rt->font || rt->colon_count == 0 || lv_obj_has_flag(rt->label, LV_OBJ_FLAG_HIDDEN))
      continue;
    if (!(rt->sprite ? phase_changed : blink_changed))
      continue;
    // Redraw only the colon cell: decoding the digits again every step would be wasteful.
    lv_obj_update_layout(rt->label);
    lv_area_t coords;
    lv_obj_get_coords(rt->label, &coords);
    const int32_t gw = lv_font_get_glyph_width(rt->font, SPRITE_FONT_COLON_ANIM, 0);
    for (int c = 0; c < rt->colon_count; c++)
    {
      lv_point_t pos;
      lv_label_get_letter_pos(rt->label, rt->colon_idx[c], &pos);
      lv_area_t a = {coords.x1 + pos.x, coords.y1 + pos.y,
                     coords.x1 + pos.x + gw - 1, coords.y1 + pos.y + rt->font->line_height - 1};
      lv_obj_invalidate_area(rt->label, &a);
    }
  }
  lvgl_port_unlock();
}

void text_widgets_raise_layer(uint8_t profile, uint8_t z)
{
  if (profile >= FRIXOS_SCREEN_LAYOUT_PROFILES)
    return;
  for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
  {
    const text_widget_cfg_t *w = &eeprom_text_widgets.widget[profile][i];
    if (s_rt[i].label && w->enabled && w->z == z)
      lv_obj_move_foreground(s_rt[i].label);
  }
}
