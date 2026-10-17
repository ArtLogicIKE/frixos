#ifndef F_SPRITE_FONT_H
#define F_SPRITE_FONT_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

// Digit sprite themes rendered as an LVGL font: '0'-'9' come from themes/<theme>/font.jpg
// (10 cells of 18x36), ':' and '.' are composed from themes/<theme>/dot.jpg. Sheets are
// authored at 36 px and only reduced. Other characters go to the fallback font.
#define SPRITE_FONT_REF_H 36
#define SPRITE_FONT_GLYPHS 13
#define SPRITE_FONT_PATH_LEN 32

// Private-use code point for the animated colon: its top dot fades in while the bottom
// dot fades out, driven by sprite_font_colon_phase (opacity of the top dot, 0..255).
#define SPRITE_FONT_COLON_ANIM 0xE000
extern volatile uint8_t sprite_font_colon_phase;

typedef struct
{
  lv_font_t font;
  lv_image_dsc_t glyph[SPRITE_FONT_GLYPHS];
  char sheet[SPRITE_FONT_PATH_LEN];
  char dot[SPRITE_FONT_PATH_LEN];
  uint16_t sheet_w, sheet_h;
  uint8_t dot_w, dot_h;
  uint8_t h;      // glyph height
  uint8_t dw;     // digit cell width
  uint8_t sep_w;  // ':' / '.' width
  bool preserve_color;
  uint8_t color_r, color_g, color_b;
  bool has_dot;
} sprite_font_t;

// Register the glyph decoder with LVGL (once, after lv_init, under the LVGL lock).
void sprite_font_decoder_init(void);

// Build a sprite font for `theme` reduced to height size_px (<= 36). Returns false when
// the theme has no usable baseline JPG sheet; sf is then left unusable.
bool sprite_font_setup(sprite_font_t *sf, const char *theme, uint8_t size_px, const lv_font_t *fallback,
                       bool preserve_color, uint8_t color_r, uint8_t color_g, uint8_t color_b);

// Forget sf (call before freeing it).
void sprite_font_release(sprite_font_t *sf);

#endif
