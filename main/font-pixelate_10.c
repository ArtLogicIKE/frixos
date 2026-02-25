/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --no-compress --font SF Pixelate Bold.ttf --range 0-255 --format lvgl -o pixelate_10.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef PIXELATE_10
#define PIXELATE_10 1
#endif

#if PIXELATE_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfa,

    /* U+0022 "\"" */
    0xe0,

    /* U+0023 "#" */
    0x57, 0xd5, 0xf5, 0x0,

    /* U+0024 "$" */
    0x23, 0xab, 0x47, 0x16, 0xae, 0x20,

    /* U+0025 "%" */
    0x44, 0xa4, 0x48, 0x10, 0x24, 0x4b, 0x44,

    /* U+0026 "&" */
    0x21, 0xea, 0xe8, 0x7a, 0x8a, 0xde, 0x20,

    /* U+0027 "'" */
    0x80,

    /* U+0028 "(" */
    0x34, 0x48, 0x88, 0x44, 0x30,

    /* U+0029 ")" */
    0x89, 0x12, 0x52, 0x80,

    /* U+002A "*" */
    0xb4, 0xb0,

    /* U+002B "+" */
    0x5d, 0x0,

    /* U+002C "," */
    0x80,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x8, 0x44, 0x22, 0x21, 0x10, 0x80,

    /* U+0030 "0" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0031 "1" */
    0xfe,

    /* U+0032 "2" */
    0x74, 0x42, 0xe8, 0x43, 0xe0,

    /* U+0033 "3" */
    0x74, 0x42, 0x60, 0xc5, 0xc0,

    /* U+0034 "4" */
    0x14, 0xa5, 0x2f, 0x88, 0x40,

    /* U+0035 "5" */
    0xfc, 0x20, 0xe0, 0xc5, 0xc0,

    /* U+0036 "6" */
    0x74, 0x61, 0xe8, 0xc5, 0xc0,

    /* U+0037 "7" */
    0xfc, 0x31, 0x8, 0x42, 0x8, 0x0,

    /* U+0038 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+0039 "9" */
    0x74, 0x62, 0xf0, 0xc5, 0xc0,

    /* U+003A ":" */
    0x88,

    /* U+003B ";" */
    0x88,

    /* U+003C "<" */
    0x34, 0x84, 0x30,

    /* U+003D "=" */
    0xe3, 0x80,

    /* U+003E ">" */
    0x84, 0x34, 0x80,

    /* U+003F "?" */
    0x74, 0x42, 0xe8, 0x2, 0x0,

    /* U+0040 "@" */
    0xff, 0xef, 0xbb, 0xb8,

    /* U+0041 "A" */
    0x74, 0x63, 0x1f, 0xc6, 0x20,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x72, 0x38, 0x20, 0x82, 0x37, 0x0,

    /* U+0044 "D" */
    0xf4, 0x63, 0x18, 0xc7, 0xc0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xf8, 0x43, 0xe0,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf8, 0x42, 0x0,

    /* U+0047 "G" */
    0x74, 0x61, 0x38, 0xc5, 0xe0,

    /* U+0048 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0049 "I" */
    0xfe,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0xc5, 0xc0,

    /* U+004B "K" */
    0x8e, 0x4a, 0x30, 0xa2, 0x48, 0xc0,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x43, 0xe0,

    /* U+004D "M" */
    0x83, 0x8e, 0xac, 0x99, 0x32, 0x64, 0x80,

    /* U+004E "N" */
    0x8e, 0x6b, 0x38, 0xc6, 0x20,

    /* U+004F "O" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0050 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+0051 "Q" */
    0x74, 0x63, 0x1a, 0xcd, 0xc0,

    /* U+0052 "R" */
    0xf4, 0x63, 0xea, 0x4a, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0056 "V" */
    0x8c, 0x63, 0x15, 0x28, 0x80,

    /* U+0057 "W" */
    0x88, 0xc4, 0x62, 0x31, 0x15, 0x52, 0xa8, 0x88,

    /* U+0058 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+0059 "Y" */
    0x8c, 0x62, 0xa2, 0x10, 0x80,

    /* U+005A "Z" */
    0xfc, 0x31, 0x8, 0x42, 0xf, 0xc0,

    /* U+005B "[" */
    0xea, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x84, 0x10, 0x82, 0x8, 0x41, 0x8,

    /* U+005D "]" */
    0xd5, 0x55, 0xc0,

    /* U+005E "^" */
    0x5c,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0xb0,

    /* U+0061 "a" */
    0xf0, 0x37, 0xe3, 0x70,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc7, 0xc0,

    /* U+0063 "c" */
    0x7e, 0x8, 0x20, 0x7c,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc5, 0xe0,

    /* U+0065 "e" */
    0x72, 0x3f, 0x20, 0x7c,

    /* U+0066 "f" */
    0x3a, 0x11, 0xc4, 0x21, 0x8,

    /* U+0067 "g" */
    0x7c, 0x63, 0x17, 0x85, 0xe0,

    /* U+0068 "h" */
    0x84, 0x3d, 0x18, 0xc6, 0x20,

    /* U+0069 "i" */
    0xbe,

    /* U+006A "j" */
    0x10, 0x11, 0x11, 0x19, 0x60,

    /* U+006B "k" */
    0x82, 0x8, 0xe4, 0xe2, 0x48, 0xc0,

    /* U+006C "l" */
    0xfe,

    /* U+006D "m" */
    0xed, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0x80,

    /* U+006F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+0070 "p" */
    0xf4, 0x63, 0x1f, 0x42, 0x0,

    /* U+0071 "q" */
    0x7c, 0x63, 0x17, 0x84, 0x20,

    /* U+0072 "r" */
    0xe4, 0xe1, 0x8, 0x0,

    /* U+0073 "s" */
    0x7e, 0x7, 0x3, 0xf0,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x90,

    /* U+0075 "u" */
    0x8c, 0x63, 0x17, 0x80,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa2, 0x0,

    /* U+0077 "w" */
    0x88, 0xc4, 0x55, 0x4a, 0xa2, 0x20,

    /* U+0078 "x" */
    0x8d, 0x42, 0x14, 0x8c,

    /* U+0079 "y" */
    0x8c, 0x63, 0x17, 0x85, 0xe0,

    /* U+007A "z" */
    0xfc, 0x37, 0x20, 0xfc,

    /* U+007B "{" */
    0x19, 0x8, 0x8c, 0x20, 0x84, 0x18,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0x84, 0x42, 0x32, 0x44, 0x80,

    /* U+007E "~" */
    0x5e,

    /* U+0082 "" */
    0x80,

    /* U+0084 "" */
    0xe0,

    /* U+0085 "" */
    0xa8,

    /* U+0088 "" */
    0x5c,

    /* U+0089 "" */
    0x44, 0xa, 0x40, 0x48, 0x1, 0x0, 0x24, 0x44,
    0xbb, 0x44, 0x40,

    /* U+008A "" */
    0x51, 0x1d, 0x18, 0x38, 0x31, 0x70,

    /* U+008B "" */
    0x34, 0x84, 0x30,

    /* U+008C "" */
    0x7f, 0x90, 0x90, 0x9f, 0x90, 0x90, 0x7f,

    /* U+0091 "" */
    0xc0,

    /* U+0092 "" */
    0x80,

    /* U+0093 "" */
    0xdc,

    /* U+0094 "" */
    0xe0,

    /* U+0095 "" */
    0x77, 0xff, 0xf7, 0x0,

    /* U+0096 "" */
    0xe0,

    /* U+0097 "" */
    0xf0,

    /* U+0098 "" */
    0x5e,

    /* U+009A "" */
    0x70, 0x87, 0xe0, 0x70, 0x3f, 0x0,

    /* U+009B "" */
    0x84, 0x34, 0x80,

    /* U+009C "" */
    0x6c, 0x93, 0x9c, 0x90, 0x6f,

    /* U+009F "" */
    0x60, 0x23, 0x18, 0xa8, 0x84, 0x20,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xbe,

    /* U+00A2 "¢" */
    0x11, 0xf9, 0x24, 0x91, 0xf1, 0x0,

    /* U+00A5 "¥" */
    0x8c, 0x62, 0xaf, 0xfc, 0x80,

    /* U+00A8 "¨" */
    0xc0,

    /* U+00AB "«" */
    0x2d, 0x4a, 0x14, 0x2c,

    /* U+00AD "­" */
    0xe0,

    /* U+00AF "¯" */
    0xe0,

    /* U+00B0 "°" */
    0xb0,

    /* U+00B1 "±" */
    0x5d, 0xe,

    /* U+00B4 "´" */
    0x60,

    /* U+00BB "»" */
    0xa1, 0x42, 0xd4, 0xa0,

    /* U+00BF "¿" */
    0x8, 0x2, 0xe8, 0x45, 0xc0,

    /* U+00C0 "À" */
    0x41, 0x1d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C1 "Á" */
    0x11, 0x1d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C2 "Â" */
    0x72, 0x9d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C3 "Ã" */
    0x74, 0x9d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C4 "Ä" */
    0x50, 0x1d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C5 "Å" */
    0x71, 0x1d, 0x18, 0xc7, 0xf1, 0x88,

    /* U+00C6 "Æ" */
    0x7f, 0x90, 0x90, 0xff, 0x90, 0x90, 0x9f,

    /* U+00C8 "È" */
    0x41, 0x3f, 0x8, 0x7e, 0x10, 0xf8,

    /* U+00C9 "É" */
    0x11, 0x3f, 0x8, 0x7e, 0x10, 0xf8,

    /* U+00CA "Ê" */
    0x62, 0xbf, 0x8, 0x7e, 0x10, 0xf8,

    /* U+00CB "Ë" */
    0x60, 0x3f, 0x8, 0x7e, 0x10, 0xf8,

    /* U+00CC "Ì" */
    0xff, 0x80,

    /* U+00CD "Í" */
    0xea, 0xaa, 0x80,

    /* U+00CE "Î" */
    0x55, 0x24, 0x92, 0x40,

    /* U+00CF "Ï" */
    0xbf, 0x80,

    /* U+00D1 "Ñ" */
    0x74, 0xa3, 0x9a, 0xce, 0x31, 0x88,

    /* U+00D2 "Ò" */
    0x41, 0x1d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D3 "Ó" */
    0x11, 0x1d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D4 "Ô" */
    0x72, 0x9d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D5 "Õ" */
    0x74, 0x9d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D6 "Ö" */
    0x50, 0x1d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D7 "×" */
    0xb4, 0xb0,

    /* U+00D8 "Ø" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+00D9 "Ù" */
    0x41, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DA "Ú" */
    0x11, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DB "Û" */
    0x72, 0xa3, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DC "Ü" */
    0x50, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DD "Ý" */
    0x11, 0x23, 0x18, 0xa8, 0x84, 0x20,

    /* U+00E0 "à" */
    0x40, 0x8f, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E1 "á" */
    0x10, 0x8f, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E2 "â" */
    0x31, 0x4f, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E3 "ã" */
    0x79, 0x4f, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E4 "ä" */
    0x30, 0xf, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E5 "å" */
    0x30, 0x8f, 0x3, 0x7e, 0x37, 0x0,

    /* U+00E6 "æ" */
    0xec, 0x25, 0xcc, 0x96, 0xc0,

    /* U+00E8 "è" */
    0x40, 0x87, 0x23, 0xf2, 0x7, 0xc0,

    /* U+00E9 "é" */
    0x10, 0x87, 0x23, 0xf2, 0x7, 0xc0,

    /* U+00EA "ê" */
    0x31, 0x47, 0x23, 0xf2, 0x7, 0xc0,

    /* U+00EB "ë" */
    0x30, 0x7, 0x23, 0xf2, 0x7, 0xc0,

    /* U+00EC "ì" */
    0xfe,

    /* U+00ED "í" */
    0xea, 0xa8,

    /* U+00EE "î" */
    0x55, 0x24, 0x90,

    /* U+00EF "ï" */
    0xbe,

    /* U+00F1 "ñ" */
    0x74, 0xbd, 0x18, 0xc6, 0x20,

    /* U+00F2 "ò" */
    0x41, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F3 "ó" */
    0x11, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F4 "ô" */
    0x72, 0x9d, 0x18, 0xc5, 0xc0,

    /* U+00F5 "õ" */
    0x74, 0x9d, 0x18, 0xc5, 0xc0,

    /* U+00F6 "ö" */
    0x50, 0x1d, 0x18, 0xc5, 0xc0,

    /* U+00F7 "÷" */
    0x5c, 0x20,

    /* U+00F8 "ø" */
    0x74, 0x63, 0x17, 0x0,

    /* U+00F9 "ù" */
    0x41, 0x23, 0x18, 0xc5, 0xe0,

    /* U+00FA "ú" */
    0x11, 0x23, 0x18, 0xc5, 0xe0,

    /* U+00FB "û" */
    0x72, 0xa3, 0x18, 0xc5, 0xe0,

    /* U+00FC "ü" */
    0x50, 0x23, 0x18, 0xc5, 0xe0,

    /* U+00FD "ý" */
    0x11, 0x23, 0x18, 0xc5, 0xe1, 0x78,

    /* U+00FF "ÿ" */
    0x50, 0x23, 0x18, 0xc5, 0xe1, 0x78
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 3, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 7, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 13, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 27, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 28, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 33, .adv_w = 80, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 37, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 39, .adv_w = 80, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 41, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 43, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 44, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 50, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 48, .box_w = 1, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 48, .box_w = 1, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 102, .adv_w = 80, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 104, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 107, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 144, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 176, .box_w = 9, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 64, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 253, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 259, .adv_w = 64, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 262, .adv_w = 72, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 263, .adv_w = 64, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 264, .adv_w = 56, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 265, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 283, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 297, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 96, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 308, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 144, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 333, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 338, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 80, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 176, .box_w = 9, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 372, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 382, .adv_w = 48, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 384, .adv_w = 96, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 389, .adv_w = 88, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 390, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 112, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 72, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 394, .adv_w = 208, .box_w = 12, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 414, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 48, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 422, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 423, .adv_w = 72, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 424, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 425, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 429, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 430, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 431, .adv_w = 88, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 432, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 441, .adv_w = 144, .box_w = 8, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 454, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 460, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 56, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 466, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 470, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 471, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 472, .adv_w = 56, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 473, .adv_w = 80, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 56, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 476, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 480, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 485, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 491, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 515, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 528, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 48, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 554, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 48, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 599, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 601, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 618, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 624, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 648, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 654, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 660, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 666, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 144, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 677, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 683, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 695, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 702, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 48, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 718, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 723, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 728, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 80, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 740, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 744, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 759, .adv_w = 112, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 770, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_1[] = {
    0, 0, 1, 2, 0, 0, 3, 4,
    5, 6, 7, 0, 0, 0, 0, 8,
    9, 10, 11, 12, 13, 14, 15, 0,
    16, 17, 18, 0, 0, 19, 20, 21,
    22, 0, 0, 23, 0, 0, 24, 0,
    0, 25, 0, 26, 0, 27, 28, 29,
    0, 0, 30, 0, 0, 0, 0, 0,
    0, 31, 0, 0, 0, 32, 33, 34,
    35, 36, 37, 38, 39, 0, 40, 41,
    42, 43, 44, 45, 46, 47, 0, 48,
    49, 50, 51, 52, 53, 54, 55, 56,
    57, 58, 59, 60, 0, 0, 61, 62,
    63, 64, 65, 66, 67, 0, 68, 69,
    70, 71, 72, 73, 74, 75, 0, 76,
    77, 78, 79, 80, 81, 82, 83, 84,
    85, 86, 87, 88, 0, 89
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 130, .range_length = 126, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_1, .list_length = 126, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t pixelate_10 = {
#else
lv_font_t pixelate_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if PIXELATE_10*/

