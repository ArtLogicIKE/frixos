#include "f-layout-migrate.h"
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "esp_log.h"
#include "nvs.h"

#define NVS_NAMESPACE "frixos"
#define MAX_SLOTS 8
#define SCHED_JSON_LEN 512
#define LEGACY_DIGIT_MIN_SIZE 9 // the legacy time widget stored its reduced digit height in .font

static const char *TAG = "f-layout-migrate";

// Legacy element indices (layout version 10).
enum
{
  L_GLUCOSE_LEVEL = 0,
  L_GLUCOSE_TREND = 1,
  L_WIFI_OFF = 2,
  L_WEATHER = 3,
  L_MOON = 4,
  L_TIME = 5,
  L_MESSAGE = 6,
  L_TEXT_1 = 7,
  L_AMPM = 15,
  L_TIME_AUX = 16,
  L_DIGIT_LABEL = 17,
  L_DIGIT_LABEL_AUX = 18,
  L_GRAPH = 19,
  L_ICON_1 = 20,
};

typedef struct
{
  int type; // 0 time, 1 CGM, 2 weather temperature, 3 Home Assistant
  uint16_t duration;
  char entity[64];
  char label[8];
  char name[26];
} sched_slot_t;

typedef struct
{
  int count;
  sched_slot_t slot[MAX_SLOTS];
} sched_t;

static void parse_schedule(sched_t *out, const char *json)
{
  out->count = 0;
  if (!json || !json[0])
    return;
  cJSON *arr = cJSON_Parse(json);
  if (!arr || !cJSON_IsArray(arr))
  {
    cJSON_Delete(arr);
    return;
  }
  const int n = cJSON_GetArraySize(arr);
  for (int i = 0; i < n && out->count < MAX_SLOTS; i++)
  {
    cJSON *s = cJSON_GetArrayItem(arr, i);
    cJSON *t = cJSON_GetObjectItem(s, "t");
    cJSON *d = cJSON_GetObjectItem(s, "d");
    if (!cJSON_IsNumber(t) || !cJSON_IsNumber(d) || t->valueint < 0 || t->valueint > 3 ||
        d->valueint <= 0 || d->valueint > 3600)
      continue;
    sched_slot_t *o = &out->slot[out->count];
    memset(o, 0, sizeof(*o));
    o->type = t->valueint;
    o->duration = (uint16_t)d->valueint;
    cJSON *e = cJSON_GetObjectItem(s, "e");
    if (cJSON_IsString(e))
      strlcpy(o->entity, e->valuestring, sizeof(o->entity));
    if (o->type == 3 && !o->entity[0])
      continue; // a Home Assistant slot without an entity showed nothing
    cJSON *l = cJSON_GetObjectItem(s, "l");
    if (cJSON_IsString(l))
      strlcpy(o->label, l->valuestring, sizeof(o->label));
    cJSON *nm = cJSON_GetObjectItem(s, "n");
    if (cJSON_IsString(nm))
      strlcpy(o->name, nm->valuestring, sizeof(o->name));
    out->count++;
  }
  cJSON_Delete(arr);
}

static void load_schedules(sched_t *primary, sched_t *aux)
{
  primary->count = 0;
  aux->count = 0;
  nvs_handle_t h;
  if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &h) != ESP_OK)
    return;

  char *json = malloc(SCHED_JSON_LEN);
  if (json)
  {
    size_t len = SCHED_JSON_LEN;
    if (nvs_get_str(h, "disp_sched", json, &len) == ESP_OK)
      parse_schedule(primary, json);
    len = SCHED_JSON_LEN;
    if (nvs_get_str(h, "disp_sched_aux", json, &len) == ESP_OK)
      parse_schedule(aux, json);
    free(json);
  }

  if (primary->count == 0)
  {
    // Before the JSON schedule existed the rotation came from three durations.
    uint8_t sec_time = 25, sec_cgm = 5, sec_weather = 0;
    nvs_get_u8(h, "sec_time", &sec_time);
    nvs_get_u8(h, "sec_cgm", &sec_cgm);
    nvs_get_u8(h, "sec_weather", &sec_weather);
    sched_slot_t *s = &primary->slot[primary->count++];
    memset(s, 0, sizeof(*s));
    s->type = 0;
    s->duration = sec_time > 0 ? sec_time : 30;
    if (sec_cgm > 0)
    {
      s = &primary->slot[primary->count++];
      memset(s, 0, sizeof(*s));
      s->type = 1;
      s->duration = sec_cgm;
    }
    if (sec_weather > 0)
    {
      s = &primary->slot[primary->count++];
      memset(s, 0, sizeof(*s));
      s->type = 2;
      s->duration = sec_weather;
    }
  }
  nvs_close(h);
}

// Copy at most TEXT_WIDGET_TEXT_LEN-1 bytes without cutting a UTF-8 sequence.
static void copy_text(char *dst, const char *src)
{
  size_t n = strnlen(src, TEXT_WIDGET_TEXT_LEN - 1);
  while (n > 0 && src[n] != '\0' && ((uint8_t)src[n] & 0xC0) == 0x80)
    n--;
  memcpy(dst, src, n);
  dst[n] = '\0';
}

static void slot_text(char *dst, const sched_slot_t *s)
{
  char buf[2 * TEXT_WIDGET_TEXT_LEN];
  switch (s->type)
  {
  case 1:
    strcpy(buf, "[CGM:reading]");
    break;
  case 2:
    strcpy(buf, "[temp]\xC2\xB0"); // degree sign instead of the unit
    break;
  case 3:
    snprintf(buf, sizeof(buf), "[HA:%s:state]%s", s->entity, s->label);
    break;
  default:
    strcpy(buf, TW_TOKEN_CLOCK);
    break;
  }
  copy_text(dst, buf);
}

static void set_geometry(text_widget_cfg_t *t, const screen_widget_t *w)
{
  t->enabled = 1;
  t->x = w->x;
  t->y = w->y;
  t->z = w->z;
}

// Digit display: the schedule becomes the rotating contents (the legacy rule: a schedule of one
// entry always showed the time).
static void make_digit_widget(text_widget_cfg_t *t, const screen_widget_t *w, uint8_t family, const sched_t *sc)
{
  set_geometry(t, w);
  t->family = family;
  t->size = (w->font >= LEGACY_DIGIT_MIN_SIZE && w->font < TEXT_WIDGET_SIZE_MAX) ? w->font : TEXT_WIDGET_SIZE_MAX;
  t->flags = 0; // digits show no units: weather/HA slots carry their own symbol
  const int n = sc->count > 1 ? (sc->count < TEXT_WIDGET_SLOTS ? sc->count : TEXT_WIDGET_SLOTS) : 1;
  t->slot_count = (uint8_t)n;
  for (int i = 0; i < n; i++)
  {
    if (sc->count > 1)
    {
      slot_text(t->text[i], &sc->slot[i]);
      t->duration[i] = sc->slot[i].duration;
    }
    else
    {
      strcpy(t->text[i], TW_TOKEN_CLOCK);
    }
  }
  t->color_r = t->color_g = t->color_b = 255;
}

static const uint8_t k_font_px[] = {8, 10, 12, 14, 16, 8};

// Text label / digit label: same position, colours, width and alignment, drawn in the sans family.
static void make_label_widget(text_widget_cfg_t *t, const screen_widget_t *w, const char *text)
{
  set_geometry(t, w);
  t->family = TW_FAMILY_SANS;
  t->size = k_font_px[w->font <= 5 ? w->font : 0];
  t->width = w->width;
  t->align = w->align;
  t->flags = TW_FLAG_SHOW_UNITS;
  t->slot_count = 1;
  t->color_r = w->color_r;
  t->color_g = w->color_g;
  t->color_b = w->color_b;
  t->bg_r = w->bg_r;
  t->bg_g = w->bg_g;
  t->bg_b = w->bg_b;
  copy_text(t->text[0], text);
}

// A digit label showed the schedule slot's name while that slot was up: same rotation here.
static void make_digit_label(text_widget_cfg_t *t, const screen_widget_t *w, const char *text, const sched_t *sc)
{
  make_label_widget(t, w, text);
  if (sc->count < 2)
    return;
  bool named = false;
  for (int i = 0; i < sc->count; i++)
    named = named || sc->slot[i].name[0];
  if (!named)
    return;
  const int n = sc->count < TEXT_WIDGET_SLOTS ? sc->count : TEXT_WIDGET_SLOTS;
  t->slot_count = (uint8_t)n;
  for (int i = 0; i < n; i++)
  {
    copy_text(t->text[i], sc->slot[i].name[0] ? sc->slot[i].name : text);
    t->duration[i] = sc->slot[i].duration;
  }
}

static void convert_profile(const legacy_profile_v10_t *o, screen_layout_profile_t *n,
                            text_widget_cfg_t tw[TEXT_WIDGET_COUNT],
                            const sched_t *primary, const sched_t *aux)
{
  memset(n, 0, sizeof(*n));
  n->widget[SCREEN_ELEM_GLUCOSE_LEVEL] = o->widget[L_GLUCOSE_LEVEL];
  n->widget[SCREEN_ELEM_GLUCOSE_TREND] = o->widget[L_GLUCOSE_TREND];
  n->widget[SCREEN_ELEM_WIFI_OFF] = o->widget[L_WIFI_OFF];
  n->widget[SCREEN_ELEM_WEATHER] = o->widget[L_WEATHER];
  n->widget[SCREEN_ELEM_MOON] = o->widget[L_MOON];
  n->widget[SCREEN_ELEM_MESSAGE] = o->widget[L_MESSAGE];
  n->widget[SCREEN_ELEM_GRAPH] = o->widget[L_GRAPH];
  for (int i = 0; i < SCREEN_ICON_COUNT; i++)
    n->widget[SCREEN_ELEM_ICON_1 + i] = o->widget[L_ICON_1 + i];
  memcpy(n->scroll_text, o->scroll_text, sizeof(n->scroll_text));
  n->graph = o->graph;

  int next = 0;
  int dropped = 0;
#define NEXT_WIDGET() (next < TEXT_WIDGET_COUNT ? &tw[next++] : (dropped++, NULL))
  text_widget_cfg_t *t;
  if (o->widget[L_TIME].enabled && (t = NEXT_WIDGET()))
    make_digit_widget(t, &o->widget[L_TIME], TW_FAMILY_SPRITE_THEME, primary);
  if (o->widget[L_TIME_AUX].enabled && (t = NEXT_WIDGET()))
    make_digit_widget(t, &o->widget[L_TIME_AUX], TW_FAMILY_SPRITE_AUX, aux);
  if (o->widget[L_TIME].enabled && o->widget[L_AMPM].enabled && (t = NEXT_WIDGET()))
  {
    // The AM/PM sprite becomes a small text mark that is empty in 24 h mode.
    set_geometry(t, &o->widget[L_AMPM]);
    t->family = TW_FAMILY_SANS;
    t->size = 12;
    t->flags = TW_FLAG_SHOW_UNITS | TW_FLAG_TRANSPARENT_BG;
    t->slot_count = 1;
    t->color_r = t->color_g = t->color_b = 255;
    strcpy(t->text[0], TW_TOKEN_AMPM12);
  }
  if (o->widget[L_DIGIT_LABEL].enabled && (t = NEXT_WIDGET()))
    make_digit_label(t, &o->widget[L_DIGIT_LABEL], o->digit_label_text, primary);
  if (o->widget[L_DIGIT_LABEL_AUX].enabled && (t = NEXT_WIDGET()))
    make_digit_label(t, &o->widget[L_DIGIT_LABEL_AUX], o->digit_label_aux_text, aux);
  for (int i = 0; i < LEGACY_STATIC_TEXT_COUNT; i++)
  {
    if (o->widget[L_TEXT_1 + i].enabled && (t = NEXT_WIDGET()))
      make_label_widget(t, &o->widget[L_TEXT_1 + i], o->static_text[i]);
  }
#undef NEXT_WIDGET
  if (dropped)
    ESP_LOGW(TAG, "%d legacy element(s) did not fit the %d text widgets", dropped, TEXT_WIDGET_COUNT);
}

void screen_layout_migrate_v10(const legacy_profile_v10_t legacy[FRIXOS_SCREEN_LAYOUT_PROFILES],
                               screen_layout_t *layout, text_widgets_layout_t *tw)
{
  sched_t *primary = malloc(sizeof(sched_t));
  sched_t *aux = malloc(sizeof(sched_t));
  if (!primary || !aux)
  {
    free(primary);
    free(aux);
    ESP_LOGE(TAG, "no memory for the schedule, using defaults");
    screen_layout_apply_factory_defaults(layout);
    return;
  }
  load_schedules(primary, aux);

  text_widgets_defaults(tw);
  for (int p = 0; p < FRIXOS_SCREEN_LAYOUT_PROFILES; p++)
  {
    // Start from empty widgets: only what the old layout showed is carried over.
    tw->meta.instanced[p] = 0;
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      tw->widget[p][i].enabled = 0;
      memset(tw->widget[p][i].text, 0, sizeof(tw->widget[p][i].text));
      tw->widget[p][i].family = TW_FAMILY_SANS;
      tw->widget[p][i].size = 16;
      tw->widget[p][i].flags = TW_FLAG_SHOW_UNITS | TW_FLAG_TABULAR;
      tw->widget[p][i].slot_count = 1;
      tw->widget[p][i].z = 2;
    }
    convert_profile(&legacy[p], &layout->profile[p], tw->widget[p], primary, aux);
  }
  free(primary);
  free(aux);
  text_widgets_sanitize(tw);
}

void screen_layout_migrate_v10_done(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &h) != ESP_OK)
    return;
  static const char *const keys[] = {
      "disp_sched", "disp_sched_aux", "sec_time", "sec_cgm", "sec_weather",
      "msg_red", "msg_green", "msg_blue", "night_msg_red", "night_msg_green", "night_msg_blue",
      "msg_font", "scroll_speed", "scroll_eng"};
  for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    nvs_erase_key(h, keys[i]);
  nvs_commit(h);
  nvs_close(h);
}

static bool widget_has_token(const text_widget_cfg_t *widget, const char *token)
{
  for (int slot = 0; slot < TEXT_WIDGET_SLOTS; slot++)
    if (strstr(widget->text[slot], token) != NULL)
      return true;
  return false;
}

void screen_layout_migrate_v11(screen_layout_t *layout, text_widgets_layout_t *tw)
{
  static const uint8_t font_sizes[] = {8, 10, 12, 14, 16, 8};
  if (!layout || !tw)
    return;

  for (int p = 0; p < FRIXOS_SCREEN_LAYOUT_PROFILES; p++)
  {
    screen_layout_profile_t *profile = &layout->profile[p];
    screen_widget_t *legacy = &profile->widget[SCREEN_ELEM_MESSAGE];
    int target = -1;
    for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
    {
      const bool exists = (tw->meta.instanced[p] & (1u << i)) != 0;
      if (!exists && !tw->widget[p][i].enabled)
      {
        target = i;
        break;
      }
    }

    if (target < 0)
    {
      int clock = -1;
      int ampm = -1;
      for (int i = 0; i < TEXT_WIDGET_COUNT; i++)
      {
        if (widget_has_token(&tw->widget[p][i], TW_TOKEN_CLOCK))
          clock = i;
        if (widget_has_token(&tw->widget[p][i], TW_TOKEN_AMPM12))
          ampm = i;
      }
      if (clock >= 0 && ampm >= 0 && clock != ampm)
      {
        text_widget_cfg_t *clock_widget = &tw->widget[p][clock];
        int clock_slot = -1;
        for (int s = 0; s < TEXT_WIDGET_SLOTS; s++)
          if (strstr(clock_widget->text[s], TW_TOKEN_CLOCK) != NULL)
          {
            clock_slot = s;
            break;
          }
        if (clock_slot >= 0 && strnlen(clock_widget->text[clock_slot], TEXT_WIDGET_TEXT_LEN) +
                                   sizeof(TW_TOKEN_AMPM12) - 1 < TEXT_WIDGET_TEXT_LEN)
        {
          strlcat(clock_widget->text[clock_slot], TW_TOKEN_AMPM12, sizeof(clock_widget->text[clock_slot]));
          tw->widget[p][ampm].enabled = 0;
          target = ampm;
        }
      }
    }

    if (target >= 0)
    {
      text_widget_cfg_t *scroll = &tw->widget[p][target];
      memset(scroll, 0, sizeof(*scroll));
      scroll->enabled = legacy->enabled ? 1 : 0;
      scroll->x = legacy->x;
      scroll->y = legacy->y;
      scroll->z = legacy->z;
      scroll->width = legacy->width;
      scroll->align = legacy->align;
      scroll->family = TW_FAMILY_SANS;
      scroll->size = font_sizes[legacy->font <= 5 ? legacy->font : 0];
      scroll->flags = TW_FLAG_SCROLL | TW_FLAG_SHOW_UNITS | TW_FLAG_COLOR_OVERRIDE;
      scroll->slot_count = 1;
      scroll->duration[0] = TEXT_WIDGET_DURATION_DEFAULT;
      scroll->color_r = legacy->color_r;
      scroll->color_g = legacy->color_g;
      scroll->color_b = legacy->color_b;
      tw->meta.instanced[p] |= (uint16_t)(1u << target);
      strlcpy(tw->meta.name[p][target], "Scroll", WIDGET_NAME_LEN);
    }
    else
      ESP_LOGW(TAG, "profile %d: no free text widget for legacy scrolling message", p);

    legacy->enabled = 0;
  }
}
