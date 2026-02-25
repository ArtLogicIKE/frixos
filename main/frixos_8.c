/*******************************************************************************
 * Size: 8 px
 * Bpp: 1
 * Opts: --bpp 1 --size 8 --no-compress --font Fiery_Turk.ttf --range 0-255 --format lvgl -o frixos_8.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef FRIXOS_8
#define FRIXOS_8 1
#endif

#if FRIXOS_8

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
    0xb4,

    /* U+0023 "#" */
    0x52, 0xbe, 0xaf, 0xa9, 0x40,

    /* U+0024 "$" */
    0x23, 0xab, 0x47, 0x16, 0xae, 0x20,

    /* U+0025 "%" */
    0x43, 0x49, 0x20, 0x82, 0x49, 0x61, 0x0,

    /* U+0026 "&" */
    0x72, 0x25, 0x19, 0x96, 0x27, 0x40,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x2a, 0x49, 0x22, 0x20,

    /* U+0029 ")" */
    0x88, 0x92, 0x4a, 0x80,

    /* U+002A "*" */
    0x25, 0x5d, 0x52, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0x58,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x11, 0x22, 0x44, 0x88,

    /* U+0030 "0" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0031 "1" */
    0x75, 0x54,

    /* U+0032 "2" */
    0x74, 0x42, 0x22, 0x23, 0xe0,

    /* U+0033 "3" */
    0x74, 0x42, 0x60, 0xc5, 0xc0,

    /* U+0034 "4" */
    0x11, 0x95, 0x2f, 0x88, 0x40,

    /* U+0035 "5" */
    0xfc, 0x3c, 0x10, 0xc5, 0xc0,

    /* U+0036 "6" */
    0x74, 0x61, 0xe8, 0xc5, 0xc0,

    /* U+0037 "7" */
    0xf8, 0x42, 0x21, 0x10, 0x80,

    /* U+0038 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+0039 "9" */
    0x74, 0x62, 0xf0, 0xc5, 0xc0,

    /* U+003A ":" */
    0x90,

    /* U+003B ";" */
    0x40, 0x58,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0x74, 0x42, 0x22, 0x0, 0x80,

    /* U+0040 "@" */
    0x38, 0x8a, 0x4d, 0x59, 0x48, 0xe, 0x0,

    /* U+0041 "A" */
    0x74, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x74, 0x61, 0x8, 0x45, 0xc0,

    /* U+0044 "D" */
    0xf4, 0x63, 0x18, 0xc7, 0xc0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+0047 "G" */
    0x74, 0x61, 0x38, 0xc5, 0xc0,

    /* U+0048 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0049 "I" */
    0xfe,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0xc5, 0xc0,

    /* U+004B "K" */
    0x8c, 0xa9, 0x8a, 0x4a, 0x20,

    /* U+004C "L" */
    0x88, 0x88, 0x88, 0xf0,

    /* U+004D "M" */
    0x83, 0x8e, 0xac, 0x98, 0x30, 0x60, 0x80,

    /* U+004E "N" */
    0x8c, 0x73, 0x59, 0xc6, 0x20,

    /* U+004F "O" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0050 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+0051 "Q" */
    0x74, 0x63, 0x18, 0xc5, 0xc1,

    /* U+0052 "R" */
    0xf4, 0x63, 0xea, 0x4a, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0056 "V" */
    0x8c, 0x62, 0xa5, 0x10, 0x80,

    /* U+0057 "W" */
    0x93, 0x26, 0x4c, 0x99, 0x32, 0x5b, 0x0,

    /* U+0058 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+0059 "Y" */
    0x8c, 0x54, 0x42, 0x10, 0x80,

    /* U+005A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+005B "[" */
    0xea, 0xaa, 0xc0,

    /* U+005C "\\" */
    0x88, 0x44, 0x22, 0x11,

    /* U+005D "]" */
    0xd5, 0x55, 0xc0,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x79, 0x99, 0x70,

    /* U+0062 "b" */
    0x88, 0xe9, 0x99, 0xe0,

    /* U+0063 "c" */
    0x69, 0x89, 0x60,

    /* U+0064 "d" */
    0x11, 0x79, 0x99, 0x70,

    /* U+0065 "e" */
    0x69, 0xf8, 0x60,

    /* U+0066 "f" */
    0x6b, 0xa8,

    /* U+0067 "g" */
    0x79, 0x99, 0x71, 0x60,

    /* U+0068 "h" */
    0x88, 0xe9, 0x99, 0x90,

    /* U+0069 "i" */
    0xbe,

    /* U+006A "j" */
    0x45, 0x55, 0x80,

    /* U+006B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+006C "l" */
    0xfe,

    /* U+006D "m" */
    0xed, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xe9, 0x99, 0x90,

    /* U+006F "o" */
    0x69, 0x99, 0x60,

    /* U+0070 "p" */
    0xe9, 0x99, 0xe8, 0x80,

    /* U+0071 "q" */
    0x79, 0x99, 0x71, 0x10,

    /* U+0072 "r" */
    0xf2, 0x48,

    /* U+0073 "s" */
    0x71, 0x1c,

    /* U+0074 "t" */
    0x4b, 0xa4, 0x90,

    /* U+0075 "u" */
    0x99, 0x99, 0x70,

    /* U+0076 "v" */
    0x99, 0x99, 0x60,

    /* U+0077 "w" */
    0x93, 0x26, 0x4c, 0x96, 0xc0,

    /* U+0078 "x" */
    0x99, 0x69, 0x90,

    /* U+0079 "y" */
    0x99, 0x99, 0x71, 0x60,

    /* U+007A "z" */
    0xf1, 0x68, 0xf0,

    /* U+007B "{" */
    0x29, 0x28, 0x92, 0x20,

    /* U+007C "|" */
    0xff,

    /* U+007D "}" */
    0x89, 0x22, 0x92, 0x80,

    /* U+007E "~" */
    0x5a,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xbe,

    /* U+00A2 "¢" */
    0x27, 0x88, 0x72,

    /* U+00A3 "£" */
    0x64, 0xa1, 0xc8, 0x47, 0xc0,

    /* U+00A4 "¤" */
    0x39, 0x1f, 0x10, 0xf1, 0x13, 0x80,

    /* U+00A5 "¥" */
    0x8c, 0x55, 0xf2, 0x7c, 0x80,

    /* U+00A6 "¦" */
    0xee,

    /* U+00A7 "§" */
    0x71, 0x54, 0x70,

    /* U+00A8 "¨" */
    0xa0,

    /* U+00A9 "©" */
    0x7d, 0x6, 0xed, 0x1b, 0xb0, 0x5f, 0x0,

    /* U+00AA "ª" */
    0x79, 0x99, 0x70, 0xf0,

    /* U+00AB "«" */
    0x4c, 0x92,

    /* U+00AC "¬" */
    0xf8, 0x42,

    /* U+00AD "­" */
    0xf0,

    /* U+00AE "®" */
    0x7d, 0x6, 0xed, 0x9a, 0xb0, 0x5f, 0x0,

    /* U+00AF "¯" */
    0xe0,

    /* U+00B0 "°" */
    0x55, 0x0,

    /* U+00B1 "±" */
    0x21, 0x3e, 0x42, 0x7c,

    /* U+00B2 "²" */
    0x69, 0x24, 0xf0,

    /* U+00B3 "³" */
    0xc7, 0x1c,

    /* U+00B4 "´" */
    0x60,

    /* U+00B5 "µ" */
    0x94, 0xa5, 0x2e, 0xc2, 0x0,

    /* U+00B6 "¶" */
    0x7d, 0x6a, 0xf2, 0x94, 0xa0,

    /* U+00B7 "·" */
    0x80,

    /* U+00B8 "¸" */
    0x47, 0x0,

    /* U+00B9 "¹" */
    0x75, 0x40,

    /* U+00BA "º" */
    0x69, 0x99, 0x60, 0xf0,

    /* U+00BB "»" */
    0x92, 0x64,

    /* U+00BC "¼" */
    0x4, 0x22, 0x32, 0x9, 0x25, 0x32, 0xa8, 0x9e,
    0x42,

    /* U+00BD "½" */
    0x4, 0x22, 0x32, 0x9, 0x65, 0x4a, 0x88, 0x88,
    0x4f,

    /* U+00BE "¾" */
    0x2, 0x30, 0x82, 0x43, 0x12, 0x29, 0xb2, 0xa1,
    0x3c, 0x42,

    /* U+00BF "¿" */
    0x20, 0x8, 0x88, 0x45, 0xc0,

    /* U+00C0 "À" */
    0x41, 0x0, 0xe8, 0xc7, 0xf1, 0x8c, 0x40,

    /* U+00C1 "Á" */
    0x11, 0x0, 0xe8, 0xc7, 0xf1, 0x8c, 0x40,

    /* U+00C2 "Â" */
    0x22, 0x80, 0xe8, 0xc7, 0xf1, 0x8c, 0x40,

    /* U+00C3 "Ã" */
    0x2a, 0x80, 0xe8, 0xc7, 0xf1, 0x8c, 0x40,

    /* U+00C4 "Ä" */
    0x50, 0x1d, 0x18, 0xfe, 0x31, 0x88,

    /* U+00C5 "Å" */
    0x22, 0x88, 0xe8, 0xc7, 0xf1, 0x8c, 0x40,

    /* U+00C6 "Æ" */
    0x7f, 0x22, 0x47, 0xe9, 0x12, 0x27, 0x80,

    /* U+00C7 "Ç" */
    0x74, 0x61, 0x8, 0x45, 0xc4, 0x40,

    /* U+00C8 "È" */
    0x41, 0x1, 0xf8, 0x43, 0xd0, 0x87, 0xc0,

    /* U+00C9 "É" */
    0x11, 0x1, 0xf8, 0x43, 0xd0, 0x87, 0xc0,

    /* U+00CA "Ê" */
    0x22, 0x81, 0xf8, 0x43, 0xd0, 0x87, 0xc0,

    /* U+00CB "Ë" */
    0x50, 0x3f, 0x8, 0x7a, 0x10, 0xf8,

    /* U+00CC "Ì" */
    0x91, 0x55, 0x50,

    /* U+00CD "Í" */
    0x62, 0xaa, 0xa0,

    /* U+00CE "Î" */
    0x54, 0x24, 0x92, 0x48,

    /* U+00CF "Ï" */
    0xa1, 0x24, 0x92, 0x40,

    /* U+00D0 "Ð" */
    0x79, 0x14, 0x79, 0x45, 0x17, 0x80,

    /* U+00D1 "Ñ" */
    0x2a, 0x81, 0x18, 0xe6, 0xb3, 0x8c, 0x40,

    /* U+00D2 "Ò" */
    0x41, 0x0, 0xe8, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00D3 "Ó" */
    0x11, 0x0, 0xe8, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00D4 "Ô" */
    0x22, 0x80, 0xe8, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00D5 "Õ" */
    0x5a, 0x6, 0x99, 0x96,

    /* U+00D6 "Ö" */
    0x50, 0x1d, 0x18, 0xc6, 0x31, 0x70,

    /* U+00D7 "×" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+00D8 "Ø" */
    0x74, 0x67, 0x5c, 0xc5, 0xc0,

    /* U+00D9 "Ù" */
    0x41, 0x1, 0x18, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00DA "Ú" */
    0x11, 0x1, 0x18, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00DB "Û" */
    0x22, 0x81, 0x18, 0xc6, 0x31, 0x8b, 0x80,

    /* U+00DC "Ü" */
    0x50, 0x23, 0x18, 0xc6, 0x31, 0x70,

    /* U+00DD "Ý" */
    0x11, 0x1, 0x18, 0xa8, 0x84, 0x21, 0x0,

    /* U+00DE "Þ" */
    0x88, 0xe9, 0xe8, 0x80,

    /* U+00DF "ß" */
    0x32, 0x63, 0x28, 0xc6, 0xd0,

    /* U+00E0 "à" */
    0x42, 0x7, 0x99, 0x97,

    /* U+00E1 "á" */
    0x24, 0x7, 0x99, 0x97,

    /* U+00E2 "â" */
    0x69, 0x7, 0x99, 0x97,

    /* U+00E3 "ã" */
    0x5a, 0x7, 0x99, 0x97,

    /* U+00E4 "ä" */
    0x90, 0x79, 0x99, 0x70,

    /* U+00E5 "å" */
    0x25, 0x27, 0x99, 0x97,

    /* U+00E6 "æ" */
    0x6c, 0x25, 0xfc, 0x86, 0xc0,

    /* U+00E7 "ç" */
    0x69, 0x89, 0x62, 0x40,

    /* U+00E8 "è" */
    0x42, 0x6, 0x9f, 0x86,

    /* U+00E9 "é" */
    0x24, 0x6, 0x9f, 0x86,

    /* U+00EA "ê" */
    0x69, 0x6, 0x9f, 0x86,

    /* U+00EB "ë" */
    0x90, 0x69, 0xf8, 0x60,

    /* U+00EC "ì" */
    0x91, 0x55,

    /* U+00ED "í" */
    0x62, 0xaa,

    /* U+00EE "î" */
    0x54, 0x24, 0x92,

    /* U+00EF "ï" */
    0xa1, 0x24, 0x90,

    /* U+00F0 "ð" */
    0x4f, 0x25, 0x99, 0x60,

    /* U+00F1 "ñ" */
    0x5a, 0xe, 0x99, 0x99,

    /* U+00F2 "ò" */
    0x42, 0x6, 0x99, 0x96,

    /* U+00F3 "ó" */
    0x24, 0x6, 0x99, 0x96,

    /* U+00F4 "ô" */
    0x69, 0x6, 0x99, 0x96,

    /* U+00F5 "õ" */
    0x5a, 0x6, 0x99, 0x96,

    /* U+00F6 "ö" */
    0x90, 0x69, 0x99, 0x60,

    /* U+00F7 "÷" */
    0x20, 0x3e, 0x2, 0x0,

    /* U+00F8 "ø" */
    0x69, 0xf9, 0x60,

    /* U+00F9 "ù" */
    0x42, 0x9, 0x99, 0x97,

    /* U+00FA "ú" */
    0x24, 0x9, 0x99, 0x97,

    /* U+00FB "û" */
    0x69, 0x9, 0x99, 0x97,

    /* U+00FC "ü" */
    0x90, 0x99, 0x99, 0x70,

    /* U+00FD "ý" */
    0x24, 0x9, 0x99, 0x97, 0x16,

    /* U+00FE "þ" */
    0x88, 0xe9, 0xe8, 0x80,

    /* U+00FF "ÿ" */
    0x90, 0x99, 0x99, 0x71, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 80, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 8, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 14, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 28, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 32, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 36, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 44, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 45, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 46, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 48, .box_w = 1, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 99, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 101, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 105, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 119, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 201, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 251, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 258, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 260, .adv_w = 112, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 262, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 285, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 293, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 313, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 317, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 342, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 349, .adv_w = 64, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 350, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 354, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 355, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 64, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 64, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 381, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 388, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 396, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 397, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 405, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 407, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 414, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 416, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 417, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 422, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 64, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 428, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 430, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 432, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 438, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 471, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 525, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 32, .box_w = 2, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 32, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 32, .box_w = 3, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 562, .adv_w = 32, .box_w = 3, .box_h = 9, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 566, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 579, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 586, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 600, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 610, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 614, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 619, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 633, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 640, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 657, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 662, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 666, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 670, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 674, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 678, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 686, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 695, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 699, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 703, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 32, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 715, .adv_w = 48, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 718, .adv_w = 48, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 721, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 725, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 737, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 753, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 756, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 760, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 768, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 772, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 777, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 781, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 96, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
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
const lv_font_t frixos_8 = {
#else
lv_font_t frixos_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
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



#endif /*#if FRIXOS_8*/

