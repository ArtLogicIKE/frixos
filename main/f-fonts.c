#include "f-fonts.h"
#include <stdlib.h>

#define FAMILY_FONTS(f)                                                                     \
  LV_FONT_DECLARE(tw_##f##_8) LV_FONT_DECLARE(tw_##f##_10) LV_FONT_DECLARE(tw_##f##_12)     \
  LV_FONT_DECLARE(tw_##f##_14) LV_FONT_DECLARE(tw_##f##_16) LV_FONT_DECLARE(tw_##f##_20)    \
  LV_FONT_DECLARE(tw_##f##_24) LV_FONT_DECLARE(tw_##f##_28) LV_FONT_DECLARE(tw_##f##_32)    \
  LV_FONT_DECLARE(tw_##f##_36)

FAMILY_FONTS(sans)
FAMILY_FONTS(bold)
LV_FONT_DECLARE(tw_sans_40) LV_FONT_DECLARE(tw_sans_44) LV_FONT_DECLARE(tw_sans_48) LV_FONT_DECLARE(tw_sans_52)

#define FAMILY_ROW(f)                                                                  \
  &tw_##f##_8, &tw_##f##_10, &tw_##f##_12, &tw_##f##_14, &tw_##f##_16, &tw_##f##_20,  \
      &tw_##f##_24, &tw_##f##_28, &tw_##f##_32, &tw_##f##_36

// Must match FAMILY_SIZES in tools/gen_text_widget_fonts.sh.
static const uint8_t k_sizes[] = {8, 10, 12, 14, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52};
#define SIZE_COUNT (sizeof(k_sizes) / sizeof(k_sizes[0]))
#define STD_SIZE_COUNT 10 // sizes up to FONT_SIZE_MAX, available to every family

static const lv_font_t *const k_fonts[FONT_FAMILY_COUNT][SIZE_COUNT] = {
    [FONT_FAMILY_SANS] = {FAMILY_ROW(sans), &tw_sans_40, &tw_sans_44, &tw_sans_48, &tw_sans_52},
    [FONT_FAMILY_BOLD] = {FAMILY_ROW(bold)},
};

const lv_font_t *font_registry_get(uint8_t family, uint8_t size_px)
{
  if (family >= FONT_FAMILY_COUNT)
    family = FONT_FAMILY_SANS;
  size_t i = 0;
  while (i + 1 < STD_SIZE_COUNT && k_sizes[i + 1] <= size_px)
    i++;
  return k_fonts[family][i];
}

const lv_font_t *font_registry_match_cap(uint8_t cap_px)
{
  const lv_font_t *best = k_fonts[FONT_FAMILY_SANS][0];
  int best_diff = 1000;
  for (size_t i = 0; i < SIZE_COUNT; i++)
  {
    const lv_font_t *f = k_fonts[FONT_FAMILY_SANS][i];
    lv_font_glyph_dsc_t g;
    if (!f->get_glyph_dsc(f, &g, 'H', 0))
      continue;
    const int diff = abs((int)g.box_h - (int)cap_px);
    if (diff < best_diff)
    {
      best_diff = diff;
      best = f;
    }
  }
  return best;
}
