// Digit sprite sheets as an LVGL font (see f-sprite-font.h).
// Each glyph is an lv_image_dsc_t whose data points back at its sprite_font_t, so a
// dedicated image decoder can crop the glyph's cell out of the JPG sheet and reduce it
// on the fly. Only the band LVGL asks for is decoded, into a small RGB565 buffer:
// nothing is cached and no full-size bitmap is ever held in RAM.

#include "f-sprite-font.h"
#include "draw/lv_image_decoder_private.h"
#include "libs/tjpgd/tjpgd.h"
#include <stdio.h>
#include <string.h>

#define POOL_SIZE 4096 // TJPGD work area, same as LVGL's own JPG decoder
#define MAX_MCU_PX 16
#define MAX_FONTS 8
#define GLYPH_COLON 10
#define GLYPH_DOT 11
#define GLYPH_COLON_ANIM 12
// uint16 accumulators hold 6-bit channels times the cell area (weights sum to cw*sh).
#define MAX_CELL_AREA (65535 / 63)

static sprite_font_t *g_fonts[MAX_FONTS];
volatile uint8_t sprite_font_colon_phase = 255;

typedef struct
{
  bool from_dot; // source is the dot image, else the digit sheet
  uint8_t cell;  // digit index in the sheet
  int16_t x, y;  // destination offset inside the glyph
  uint8_t alpha; // 255 = opaque, scales the colour towards the black cell background
} blit_t;

typedef struct
{
  const sprite_font_t *sf;
  int idx;
  uint8_t *pool;
  uint16_t *out;
} glyph_ctx_t;

typedef struct
{
  uint16_t *acc; // 3 channels (B5, G6, R5) per destination pixel
  int32_t x1, y1, rw, rh;
} area_t;

static inline int scale_len(int px, int h)
{
  int v = (px * h + SPRITE_FONT_REF_H / 2) / SPRITE_FONT_REF_H;
  return v < 1 ? 1 : v;
}

static inline int scale_pos(int px, int h)
{
  return (px * h + SPRITE_FONT_REF_H / 2) / SPRITE_FONT_REF_H;
}

static bool is_registered(const void *p)
{
  for (int i = 0; i < MAX_FONTS; i++)
    if (g_fonts[i] && (const void *)g_fonts[i] == p)
      return true;
  return false;
}

// Size of a baseline JPEG from its SOF0/SOF1 marker; progressive files are rejected
// because TJPGD cannot decode them.
static bool jpeg_size(const char *path, uint16_t *w, uint16_t *h)
{
  FILE *f = fopen(path, "rb");
  if (!f)
    return false;
  bool ok = false;
  int c1 = fgetc(f), c2 = fgetc(f);
  if (c1 == 0xFF && c2 == 0xD8)
  {
    for (int guard = 0; guard < 64; guard++)
    {
      int c;
      while ((c = fgetc(f)) != EOF && c != 0xFF)
        ;
      while (c == 0xFF)
        c = fgetc(f);
      if (c == EOF || c == 0xD9 || c == 0xDA)
        break;
      if (c == 0x01 || (c >= 0xD0 && c <= 0xD7))
        continue; // markers without a length field
      uint8_t b[7];
      if (fread(b, 1, 2, f) != 2)
        break;
      const long len = ((long)b[0] << 8) | b[1];
      if (len < 2)
        break;
      if (c == 0xC0 || c == 0xC1)
      {
        if (fread(b, 1, 5, f) != 5)
          break;
        *h = (uint16_t)((b[1] << 8) | b[2]);
        *w = (uint16_t)((b[3] << 8) | b[4]);
        ok = *w > 0 && *h > 0;
        break;
      }
      if (c >= 0xC2 && c <= 0xCF && c != 0xC4 && c != 0xC8 && c != 0xCC)
        break;
      fseek(f, len - 2, SEEK_CUR);
    }
  }
  fclose(f);
  return ok;
}

static int glyph_blits(const sprite_font_t *sf, int idx, blit_t out[2])
{
  if (idx < 10)
  {
    out[0] = (blit_t){.from_dot = false, .cell = (uint8_t)idx, .alpha = 255};
    return 1;
  }
  if (!sf->has_dot)
    return 0;
  // Dot placement on the 36 px reference grid, matching the legacy time widget.
  const int16_t x = (int16_t)scale_pos(1, sf->h);
  if (idx == GLYPH_COLON || idx == GLYPH_COLON_ANIM)
  {
    const uint8_t top = idx == GLYPH_COLON_ANIM ? sprite_font_colon_phase : 255;
    out[0] = (blit_t){.from_dot = true, .x = x, .y = (int16_t)scale_pos(10, sf->h), .alpha = top};
    out[1] = (blit_t){.from_dot = true, .x = x, .y = (int16_t)scale_pos(26, sf->h),
                      .alpha = idx == GLYPH_COLON_ANIM ? (uint8_t)(255 - top) : 255};
    return 2;
  }
  out[0] = (blit_t){.from_dot = true, .x = x, .y = (int16_t)scale_pos(30, sf->h), .alpha = 255};
  return 1;
}

// Source pixel l spans [l*dst_n, (l+1)*dst_n) and destination pixel i spans
// [i*src_n, (i+1)*src_n) in common units; fills the (at most 2) overlapped dst pixels.
static int axis_map(uint32_t l, uint32_t src_n, uint32_t dst_n, uint32_t idx[2], uint32_t w[2])
{
  const uint32_t a = l * dst_n, b = a + dst_n;
  const uint32_t i0 = a / src_n, i1 = (b - 1) / src_n;
  int n = 0;
  for (uint32_t i = i0; i <= i1 && n < 2; i++)
  {
    const uint32_t lo = a > i * src_n ? a : i * src_n;
    const uint32_t hi = b < (i + 1) * src_n ? b : (i + 1) * src_n;
    idx[n] = i;
    w[n] = hi - lo;
    n++;
  }
  return n;
}

static size_t jd_input(JDEC *jd, uint8_t *buf, size_t n)
{
  FILE *f = jd->device;
  if (buf)
    return fread(buf, 1, n, f);
  return fseek(f, (long)n, SEEK_CUR) == 0 ? n : 0;
}

// Decode one source cell and box-average it into the destination area at (b->x, b->y).
static bool blit_area(const sprite_font_t *sf, const blit_t *b, uint8_t *pool, area_t *r)
{
  const uint32_t sw = b->from_dot ? sf->dot_w : sf->sheet_w;
  const uint32_t sh = b->from_dot ? sf->dot_h : sf->sheet_h;
  const uint32_t cw = b->from_dot ? sw : sw / 10;
  const uint32_t dcw = (uint32_t)scale_len((int)cw, sf->h);
  const uint32_t dh = (uint32_t)scale_len((int)sh, sf->h);

  // Destination rectangle of this blit clipped to the requested area (cell-local coords).
  const int32_t lx1 = LV_MAX(r->x1 - b->x, 0);
  const int32_t ly1 = LV_MAX(r->y1 - b->y, 0);
  const int32_t lx2 = LV_MIN(r->x1 + r->rw - 1 - b->x, (int32_t)dcw - 1);
  const int32_t ly2 = LV_MIN(r->y1 + r->rh - 1 - b->y, (int32_t)dh - 1);
  if (lx2 < lx1 || ly2 < ly1)
    return true;

  const uint32_t cx0 = b->cell * cw;
  const uint32_t sx_lo = cx0 + ((uint32_t)lx1 * cw) / dcw;
  const uint32_t sx_hi = cx0 + (((uint32_t)lx2 + 1) * cw + dcw - 1) / dcw - 1;
  const uint32_t sy_lo = ((uint32_t)ly1 * sh) / dh;
  const uint32_t sy_hi = (((uint32_t)ly2 + 1) * sh + dh - 1) / dh - 1;

  FILE *f = fopen(b->from_dot ? sf->dot : sf->sheet, "rb");
  if (!f)
    return false;
  JDEC jd;
  JRESULT rc = jd_prepare(&jd, jd_input, pool, POOL_SIZE, f);
  if (rc != JDR_OK || jd.width != sw || jd.height != sh)
  {
    fclose(f);
    return false;
  }

  const uint32_t mx = jd.msx * 8, my = jd.msy * 8;
  uint16_t rst = 0, rsc = 0;
  jd.dcv[0] = jd.dcv[1] = jd.dcv[2] = 0;
  jd.scale = 0;

  for (uint32_t y = 0; y < jd.height && rc == JDR_OK && y <= sy_hi; y += my)
  {
    for (uint32_t x = 0; x < jd.width; x += mx)
    {
      if (jd.nrst && rst++ == jd.nrst)
      {
        rc = jd_restart(&jd, rsc++);
        if (rc != JDR_OK)
          break;
        rst = 1;
      }
      rc = jd_mcu_load(&jd);
      if (rc != JDR_OK)
        break;
      if (x > sx_hi || x + mx <= sx_lo || y + my <= sy_lo)
        continue; // MCU does not feed the area: skip colour conversion
      rc = jd_mcu_output(&jd, NULL, x, y);
      if (rc != JDR_OK)
        break;

      const uint32_t rx = (x + mx <= jd.width) ? mx : jd.width - x;
      const uint32_t ry = (y + my <= jd.height) ? my : jd.height - y;
      const uint8_t *px = (const uint8_t *)jd.workbuf; // B, G, R per pixel
      uint32_t xi[MAX_MCU_PX][2], xw[MAX_MCU_PX][2];
      int xn[MAX_MCU_PX];
      for (uint32_t i = 0; i < rx; i++)
      {
        const uint32_t sx = x + i;
        xn[i] = (sx < sx_lo || sx > sx_hi) ? 0 : axis_map(sx - cx0, cw, dcw, xi[i], xw[i]);
      }
      for (uint32_t j = 0; j < ry; j++)
      {
        const uint32_t sy = y + j;
        if (sy < sy_lo || sy > sy_hi)
          continue;
        uint32_t yi[2], yw[2];
        const int yn = axis_map(sy, sh, dh, yi, yw);
        for (uint32_t i = 0; i < rx; i++)
        {
          const uint8_t *p = px + (j * rx + i) * 3;
          for (int bx = 0; bx < xn[i]; bx++)
          {
            const int32_t dx = (int32_t)xi[i][bx] + b->x - r->x1;
            if (dx < 0 || dx >= r->rw)
              continue;
            for (int by = 0; by < yn; by++)
            {
              const int32_t dy = (int32_t)yi[by] + b->y - r->y1;
              if (dy < 0 || dy >= r->rh)
                continue;
              const uint32_t wgt = xw[i][bx] * yw[by];
              uint16_t *q = r->acc + ((uint32_t)dy * (uint32_t)r->rw + (uint32_t)dx) * 3;
              q[0] += (uint16_t)(wgt * (p[0] >> 3));
              q[1] += (uint16_t)(wgt * (p[1] >> 2));
              q[2] += (uint16_t)(wgt * (p[2] >> 3));
            }
          }
        }
      }
    }
  }
  fclose(f);
  return rc == JDR_OK;
}

static lv_result_t decoder_info(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc, lv_image_header_t *header)
{
  LV_UNUSED(decoder);
  if (dsc->src_type != LV_IMAGE_SRC_VARIABLE)
    return LV_RESULT_INVALID;
  const lv_image_dsc_t *img = dsc->src;
  if (!is_registered(img->data))
    return LV_RESULT_INVALID;
  *header = img->header;
  return LV_RESULT_OK;
}

static lv_result_t decoder_open(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
  LV_UNUSED(decoder);
  const lv_image_dsc_t *img = dsc->src;
  if (dsc->src_type != LV_IMAGE_SRC_VARIABLE || !is_registered(img->data))
    return LV_RESULT_INVALID;

  glyph_ctx_t *ctx = lv_malloc_zeroed(sizeof(*ctx));
  if (!ctx)
    return LV_RESULT_INVALID;
  ctx->sf = (const sprite_font_t *)img->data;
  ctx->idx = (int)img->data_size;
  ctx->pool = lv_malloc(POOL_SIZE);
  if (!ctx->pool)
  {
    LV_LOG_WARN("sprite_font: no memory for decoder pool");
    lv_free(ctx);
    return LV_RESULT_INVALID;
  }
  dsc->user_data = ctx;
  dsc->header = img->header;
  return LV_RESULT_OK;
}

static lv_result_t decoder_get_area(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc,
                                    const lv_area_t *full_area, lv_area_t *decoded_area)
{
  LV_UNUSED(decoder);
  // The requested area is produced in one go; the second call ends LVGL's draw loop.
  if (decoded_area->y1 != LV_COORD_MIN)
    return LV_RESULT_INVALID;

  glyph_ctx_t *ctx = dsc->user_data;
  const int32_t gw = dsc->header.w, gh = dsc->header.h;
  area_t r;
  r.x1 = LV_MAX(full_area->x1, 0);
  r.y1 = LV_MAX(full_area->y1, 0);
  const int32_t x2 = LV_MIN(full_area->x2, gw - 1);
  const int32_t y2 = LV_MIN(full_area->y2, gh - 1);
  if (x2 < r.x1 || y2 < r.y1)
    return LV_RESULT_INVALID;
  r.rw = x2 - r.x1 + 1;
  r.rh = y2 - r.y1 + 1;

  const size_t npx = (size_t)r.rw * (size_t)r.rh;
  r.acc = lv_malloc_zeroed(npx * 3 * sizeof(uint16_t));
  lv_free(ctx->out);
  ctx->out = lv_malloc(npx * sizeof(uint16_t));
  lv_draw_buf_t *db = (lv_draw_buf_t *)dsc->decoded;
  if (!db)
    db = lv_malloc_zeroed(sizeof(lv_draw_buf_t));
  if (!r.acc || !ctx->out || !db)
  {
    LV_LOG_WARN("sprite_font: no memory for %dx%d glyph band", (int)r.rw, (int)r.rh);
    lv_free(r.acc);
    if (db != dsc->decoded)
      lv_free(db);
    return LV_RESULT_INVALID;
  }
  dsc->decoded = db;

  blit_t blits[2];
  const int nb = glyph_blits(ctx->sf, ctx->idx, blits);
  for (int i = 0; i < nb; i++)
  {
    if (!blit_area(ctx->sf, &blits[i], ctx->pool, &r))
    {
      LV_LOG_WARN("sprite_font: cannot decode %s", blits[i].from_dot ? ctx->sf->dot : ctx->sf->sheet);
      break;
    }
  }

  // Divide by the box area of each source; uncovered pixels stay black like the sheets.
  memset(ctx->out, 0, npx * sizeof(uint16_t));
  for (int i = 0; i < nb; i++)
  {
    const blit_t *b = &blits[i];
    const uint32_t sw = b->from_dot ? ctx->sf->dot_w : ctx->sf->sheet_w;
    const uint32_t sh = b->from_dot ? ctx->sf->dot_h : ctx->sf->sheet_h;
    const uint32_t cw = b->from_dot ? sw : sw / 10;
    const uint32_t den = cw * sh;
    const int32_t bx2 = b->x + scale_len((int)cw, ctx->sf->h) - 1;
    const int32_t by2 = b->y + scale_len((int)sh, ctx->sf->h) - 1;
    for (int32_t y = LV_MAX(b->y, r.y1); y <= LV_MIN(by2, y2); y++)
    {
      for (int32_t x = LV_MAX(b->x, r.x1); x <= LV_MIN(bx2, x2); x++)
      {
        const size_t o = (size_t)(y - r.y1) * (size_t)r.rw + (size_t)(x - r.x1);
        const uint16_t *q = r.acc + o * 3;
        const uint32_t source_b5 = ((q[0] + den / 2) / den) * b->alpha / 255;
        const uint32_t source_g6 = ((q[1] + den / 2) / den) * b->alpha / 255;
        const uint32_t source_r5 = ((q[2] + den / 2) / den) * b->alpha / 255;
        uint32_t r5 = source_r5, g6 = source_g6, b5 = source_b5;
        if (!ctx->sf->preserve_color)
        {
          const uint32_t source_g5 = (source_g6 * 31 + 31) / 63;
          const uint32_t coverage = (source_r5 * 77 + source_g5 * 150 + source_b5 * 29 + 128) >> 8;
          const uint32_t color_r5 = (ctx->sf->color_r * 31 + 127) / 255;
          const uint32_t color_g6 = (ctx->sf->color_g * 63 + 127) / 255;
          const uint32_t color_b5 = (ctx->sf->color_b * 31 + 127) / 255;
          r5 = (color_r5 * coverage + 15) / 31;
          g6 = (color_g6 * coverage + 15) / 31;
          b5 = (color_b5 * coverage + 15) / 31;
        }
        ctx->out[o] = (uint16_t)((r5 << 11) | (g6 << 5) | b5);
      }
    }
  }
  lv_free(r.acc);

  db->header = dsc->header;
  db->header.w = r.rw;
  db->header.h = r.rh;
  db->header.stride = r.rw * 2;
  db->data = (uint8_t *)ctx->out;
  db->unaligned_data = ctx->out;
  db->data_size = npx * sizeof(uint16_t);

  decoded_area->x1 = r.x1;
  decoded_area->y1 = r.y1;
  decoded_area->x2 = x2;
  decoded_area->y2 = y2;
  return LV_RESULT_OK;
}

static void decoder_close(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
  LV_UNUSED(decoder);
  glyph_ctx_t *ctx = dsc->user_data;
  if (ctx)
  {
    lv_free(ctx->pool);
    lv_free(ctx->out);
    lv_free(ctx);
  }
  lv_free((void *)dsc->decoded);
  dsc->decoded = NULL;
  dsc->user_data = NULL;
}

static int glyph_index(uint32_t letter)
{
  if (letter >= '0' && letter <= '9')
    return (int)(letter - '0');
  if (letter == ':')
    return GLYPH_COLON;
  if (letter == '.')
    return GLYPH_DOT;
  if (letter == SPRITE_FONT_COLON_ANIM)
    return GLYPH_COLON_ANIM;
  return -1;
}

static bool font_get_glyph_dsc(const lv_font_t *font, lv_font_glyph_dsc_t *out, uint32_t letter, uint32_t next)
{
  LV_UNUSED(next);
  const sprite_font_t *sf = font->dsc;
  const int idx = glyph_index(letter);
  if (idx < 0 || (idx >= 10 && !sf->has_dot))
    return false;
  const lv_image_dsc_t *g = &sf->glyph[idx];
  out->adv_w = (uint16_t)g->header.w;
  out->box_w = (uint16_t)g->header.w;
  out->box_h = (uint16_t)g->header.h;
  out->ofs_x = 0;
  out->ofs_y = 0;
  out->format = LV_FONT_GLYPH_FORMAT_IMAGE;
  out->is_placeholder = 0;
  out->gid.src = g;
  return true;
}

static const void *font_get_glyph_bitmap(lv_font_glyph_dsc_t *g, lv_draw_buf_t *draw_buf)
{
  LV_UNUSED(draw_buf);
  return g->gid.src;
}

void sprite_font_decoder_init(void)
{
  lv_image_decoder_t *dec = lv_image_decoder_create();
  if (!dec)
    return;
  lv_image_decoder_set_info_cb(dec, decoder_info);
  lv_image_decoder_set_open_cb(dec, decoder_open);
  lv_image_decoder_set_get_area_cb(dec, decoder_get_area);
  lv_image_decoder_set_close_cb(dec, decoder_close);
  dec->name = "SPRITE_FONT";
}

static bool themed_path(char *out, const char *theme, const char *item)
{
  snprintf(out, SPRITE_FONT_PATH_LEN, "/spiffs/themes/%s/%s.jpg", theme, item);
  FILE *f = fopen(out, "rb");
  if (!f)
  {
    snprintf(out, SPRITE_FONT_PATH_LEN, "/spiffs/%s-%s.jpg", theme, item);
    f = fopen(out, "rb");
  }
  if (!f)
  {
    snprintf(out, SPRITE_FONT_PATH_LEN, "/spiffs/themes/default/%s.jpg", item);
    f = fopen(out, "rb");
  }
  if (!f)
  {
    snprintf(out, SPRITE_FONT_PATH_LEN, "/spiffs/default-%s.jpg", item);
    f = fopen(out, "rb");
  }
  if (!f)
    return false;
  fclose(f);
  return true;
}

void sprite_font_release(sprite_font_t *sf)
{
  for (int i = 0; i < MAX_FONTS; i++)
    if (g_fonts[i] == sf)
      g_fonts[i] = NULL;
}

bool sprite_font_setup(sprite_font_t *sf, const char *theme, uint8_t size_px, const lv_font_t *fallback,
                       bool preserve_color, uint8_t color_r, uint8_t color_g, uint8_t color_b)
{
  sprite_font_release(sf);
  memset(sf, 0, sizeof(*sf));

  if (!themed_path(sf->sheet, theme, "font") || !jpeg_size(sf->sheet, &sf->sheet_w, &sf->sheet_h) ||
      sf->sheet_w < 10 || (uint32_t)(sf->sheet_w / 10) * sf->sheet_h > MAX_CELL_AREA)
    return false;
  uint16_t dw = 0, dh = 0;
  sf->has_dot = themed_path(sf->dot, theme, "dot") && jpeg_size(sf->dot, &dw, &dh) &&
                (uint32_t)dw * dh <= MAX_CELL_AREA && dw < 64 && dh < 64;
  sf->dot_w = (uint8_t)dw;
  sf->dot_h = (uint8_t)dh;
  sf->preserve_color = preserve_color;
  sf->color_r = color_r;
  sf->color_g = color_g;
  sf->color_b = color_b;

  if (size_px > SPRITE_FONT_REF_H)
    size_px = SPRITE_FONT_REF_H;
  sf->h = size_px;
  sf->dw = (uint8_t)scale_len(sf->sheet_w / 10, size_px);
  sf->sep_w = (uint8_t)(scale_pos(1, size_px) * 2 + scale_len(sf->dot_w, size_px));

  for (int i = 0; i < SPRITE_FONT_GLYPHS; i++)
  {
    lv_image_dsc_t *g = &sf->glyph[i];
    g->header.magic = LV_IMAGE_HEADER_MAGIC;
    g->header.cf = LV_COLOR_FORMAT_RGB565;
    g->header.w = i < 10 ? sf->dw : sf->sep_w;
    g->header.h = sf->h;
    g->header.stride = g->header.w * 2;
    g->data = (const uint8_t *)sf;
    g->data_size = (uint32_t)i;
  }

  sf->font.get_glyph_dsc = font_get_glyph_dsc;
  sf->font.get_glyph_bitmap = font_get_glyph_bitmap;
  sf->font.subpx = LV_FONT_SUBPX_NONE;
  sf->font.dsc = sf;
  sf->font.fallback = fallback;
  // Digits sit on the fallback baseline so mixed text such as "5.6 mmol/L" lines up.
  sf->font.base_line = fallback ? fallback->base_line : 0;
  sf->font.line_height = sf->h + sf->font.base_line;

  for (int i = 0; i < MAX_FONTS; i++)
  {
    if (!g_fonts[i])
    {
      g_fonts[i] = sf;
      return true;
    }
  }
  return false;
}
