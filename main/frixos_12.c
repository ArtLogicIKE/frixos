/*******************************************************************************
 * Size: 11 px
 * Bpp: 1
 * Opts: --bpp 1 --size 11 --no-compress --stride 1 --align 1 --font NeuePixelSans-F12.ttf --range 0-255 --format lvgl -o frixos_12.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef FRIXOS_12
#define FRIXOS_12 1
#endif

#if FRIXOS_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfc, 0x80,

    /* U+0022 "\"" */
    0xf0,

    /* U+0023 "#" */
    0x12, 0x7f, 0x24, 0xfe, 0x48, 0x48,

    /* U+0024 "$" */
    0x20, 0xf2, 0x28, 0xa0, 0xe2, 0x49, 0x25, 0xe2,
    0x8,

    /* U+0025 "%" */
    0x7, 0x90, 0x12, 0x10, 0x84, 0xa5, 0x88,

    /* U+0026 "&" */
    0x70, 0x8, 0xa0, 0x62, 0x38, 0xa2, 0x70,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x2a, 0x49, 0x24, 0x44,

    /* U+0029 ")" */
    0x88, 0x92, 0x49, 0x50,

    /* U+002A "*" */
    0x5c, 0x50,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xc0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x4, 0x10, 0x2, 0x10, 0x84, 0x20, 0x80,

    /* U+0030 "0" */
    0x70, 0x23, 0x3a, 0xe6, 0x31, 0x70,

    /* U+0031 "1" */
    0x3c, 0x92, 0x49, 0x20,

    /* U+0032 "2" */
    0xf0, 0x2, 0x11, 0x11, 0x10, 0xf8,

    /* U+0033 "3" */
    0xf0, 0x2, 0x17, 0x4, 0x21, 0xf0,

    /* U+0034 "4" */
    0x11, 0x11, 0x1f, 0x84, 0x21, 0x8,

    /* U+0035 "5" */
    0xfc, 0x21, 0xf, 0x4, 0x21, 0xf0,

    /* U+0036 "6" */
    0x70, 0x21, 0xf, 0x46, 0x31, 0x70,

    /* U+0037 "7" */
    0xfe, 0x10, 0x84, 0x0, 0x82, 0x8, 0x20,

    /* U+0038 "8" */
    0x70, 0x23, 0x17, 0x46, 0x31, 0x70,

    /* U+0039 "9" */
    0x70, 0x23, 0x17, 0x84, 0x21, 0x70,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x86,

    /* U+003C "<" */
    0x8, 0x88, 0x88, 0x20, 0x82, 0x8,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x82, 0x8, 0x20, 0x88, 0x88, 0x80,

    /* U+003F "?" */
    0xe0, 0x11, 0x24, 0x0, 0x40,

    /* U+0040 "@" */
    0x70, 0x2b, 0xbc, 0xee, 0x90, 0x78,

    /* U+0041 "A" */
    0x70, 0x23, 0x1f, 0xc6, 0x31, 0x88,

    /* U+0042 "B" */
    0xf4, 0x23, 0x1f, 0x46, 0x31, 0xf0,

    /* U+0043 "C" */
    0x70, 0x88, 0x88, 0x88, 0x70,

    /* U+0044 "D" */
    0xf2, 0x28, 0x21, 0x86, 0x18, 0x62, 0xf0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xf, 0x42, 0x10, 0xf8,

    /* U+0046 "F" */
    0xfc, 0x21, 0xf, 0x42, 0x10, 0x80,

    /* U+0047 "G" */
    0x78, 0x8, 0x20, 0xbe, 0x18, 0x61, 0x70,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x84,

    /* U+0049 "I" */
    0xff, 0x80,

    /* U+004A "J" */
    0x71, 0x11, 0x11, 0x11, 0xe0,

    /* U+004B "K" */
    0x8c, 0x61, 0x2e, 0x4a, 0x11, 0x88,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x10, 0xf8,

    /* U+004D "M" */
    0x87, 0x1a, 0xe5, 0x86, 0x18, 0x61, 0x84,

    /* U+004E "N" */
    0x86, 0x1c, 0x69, 0x96, 0x38, 0x61, 0x84,

    /* U+004F "O" */
    0x70, 0x8, 0x61, 0x86, 0x18, 0x61, 0x70,

    /* U+0050 "P" */
    0xfa, 0x8, 0x61, 0xfa, 0x8, 0x20, 0x80,

    /* U+0051 "Q" */
    0x70, 0x8, 0x61, 0x86, 0x1a, 0x69, 0x70, 0x82,
    0x0,

    /* U+0052 "R" */
    0xfa, 0x8, 0x61, 0xfa, 0x89, 0x22, 0x84,

    /* U+0053 "S" */
    0x3c, 0x8, 0x20, 0x38, 0x10, 0x41, 0x78,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x84, 0x20,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x40, 0x70,

    /* U+0056 "V" */
    0x86, 0x18, 0x61, 0x86, 0x10, 0x12, 0x30,

    /* U+0057 "W" */
    0x86, 0x18, 0x65, 0x96, 0x5b, 0xc0, 0x44,

    /* U+0058 "X" */
    0x8c, 0x40, 0xa2, 0x2a, 0x31, 0x88,

    /* U+0059 "Y" */
    0x86, 0x10, 0x12, 0x30, 0x82, 0x8, 0x20,

    /* U+005A "Z" */
    0xfc, 0x10, 0x84, 0x21, 0x8, 0x20, 0xfc,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0xe0,

    /* U+005C "\\" */
    0x82, 0x0, 0x10, 0x20, 0x40, 0x81, 0x4,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0xe0,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x88, 0x80,

    /* U+0061 "a" */
    0x70, 0x5e, 0x19, 0xb4,

    /* U+0062 "b" */
    0x84, 0x21, 0x6c, 0xc6, 0x39, 0xb0,

    /* U+0063 "c" */
    0x78, 0x88, 0x87,

    /* U+0064 "d" */
    0x8, 0x42, 0xd9, 0xc6, 0x33, 0x68,

    /* U+0065 "e" */
    0x74, 0x7d, 0x8, 0x38,

    /* U+0066 "f" */
    0x30, 0x4e, 0x44, 0x44, 0x40,

    /* U+0067 "g" */
    0x6c, 0xe3, 0x19, 0xb4, 0x21, 0x70,

    /* U+0068 "h" */
    0x84, 0x21, 0x6c, 0xc6, 0x31, 0x88,

    /* U+0069 "i" */
    0x9f, 0x80,

    /* U+006A "j" */
    0x41, 0x55, 0x56,

    /* U+006B "k" */
    0x84, 0x21, 0x19, 0x72, 0x51, 0x88,

    /* U+006C "l" */
    0xaa, 0xa8, 0x40,

    /* U+006D "m" */
    0xf5, 0x6b, 0x5a, 0xd4,

    /* U+006E "n" */
    0xb6, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xb6, 0x63, 0x1c, 0xda, 0x10, 0x80,

    /* U+0071 "q" */
    0x6c, 0xe3, 0x19, 0xb4, 0x21, 0x8,

    /* U+0072 "r" */
    0xbc, 0x88, 0x88,

    /* U+0073 "s" */
    0x74, 0x1c, 0x0, 0xb8,

    /* U+0074 "t" */
    0x88, 0x8f, 0x89, 0x90, 0x60,

    /* U+0075 "u" */
    0x8c, 0x63, 0x19, 0xb4,

    /* U+0076 "v" */
    0x8c, 0x63, 0x15, 0x10,

    /* U+0077 "w" */
    0x86, 0x19, 0x65, 0xad, 0x10,

    /* U+0078 "x" */
    0x8a, 0x88, 0xa8, 0xc4,

    /* U+0079 "y" */
    0x8c, 0x63, 0x14, 0x9c, 0x21, 0x70,

    /* U+007A "z" */
    0xf8, 0x88, 0x88, 0x7c,

    /* U+007B "{" */
    0x21, 0x28, 0x92, 0x20,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0x81, 0x22, 0x92, 0x80,

    /* U+007E "~" */
    0x4d, 0x64,

    /* U+00A1 "¡" */
    0x9f, 0x80,

    /* U+00A2 "¢" */
    0x21, 0x1d, 0x5a, 0x52, 0xae, 0x20,

    /* U+00A7 "§" */
    0x34, 0x8e, 0x97, 0x12, 0xe0,

    /* U+00A9 "©" */
    0x31, 0x2b, 0x69, 0x58, 0xc0,

    /* U+00AA "ª" */
    0x3f, 0x80,

    /* U+00AB "«" */
    0x8, 0x52, 0x94, 0xa1, 0x42, 0x85, 0x8,

    /* U+00AE "®" */
    0x23, 0xb2, 0x47, 0x0,

    /* U+00B2 "²" */
    0xe2, 0x26,

    /* U+00B3 "³" */
    0x6c, 0x20,

    /* U+00B5 "µ" */
    0x86, 0x18, 0xf5, 0x86, 0x98, 0x20,

    /* U+00B6 "¶" */
    0x7b, 0xff, 0xf1, 0x8c, 0x63, 0x18,

    /* U+00B9 "¹" */
    0x74, 0x90,

    /* U+00BA "º" */
    0xf6, 0xe0,

    /* U+00BB "»" */
    0x42, 0x85, 0xa, 0x14, 0xa5, 0x28, 0x40,

    /* U+00BF "¿" */
    0x20, 0x2, 0x40, 0x88, 0x70,

    /* U+00C0 "À" */
    0x41, 0x1c, 0x8, 0xc7, 0xf1, 0x8c, 0x62,

    /* U+00C1 "Á" */
    0x19, 0x1c, 0x8, 0xc7, 0xf1, 0x8c, 0x62,

    /* U+00C2 "Â" */
    0x22, 0x9c, 0x8, 0xc7, 0xf1, 0x8c, 0x62,

    /* U+00C3 "Ã" */
    0x55, 0x1c, 0x8, 0xc7, 0xf1, 0x8c, 0x62,

    /* U+00C4 "Ä" */
    0x53, 0x81, 0x18, 0xfe, 0x31, 0x8c, 0x40,

    /* U+00C5 "Å" */
    0x22, 0x88, 0xe0, 0x46, 0x3f, 0x8c, 0x63, 0x10,

    /* U+00C6 "Æ" */
    0x3f, 0x28, 0x28, 0x48, 0x7e, 0x48, 0x88, 0x88,
    0x8f,

    /* U+00C7 "Ç" */
    0x70, 0x88, 0x88, 0x88, 0x72, 0x60,

    /* U+00C8 "È" */
    0xc1, 0x3f, 0x8, 0x43, 0xd0, 0x84, 0x3e,

    /* U+00C9 "É" */
    0x34, 0xf8, 0x88, 0xf8, 0x88, 0xf0,

    /* U+00CA "Ê" */
    0x22, 0xbf, 0x8, 0x43, 0xd0, 0x84, 0x3e,

    /* U+00CB "Ë" */
    0x57, 0xe1, 0x8, 0x7a, 0x10, 0x87, 0xc0,

    /* U+00CC "Ì" */
    0xe3, 0x22, 0x22, 0x22, 0x22, 0x20,

    /* U+00CD "Í" */
    0x7c, 0x44, 0x44, 0x44, 0x44, 0x40,

    /* U+00CE "Î" */
    0x55, 0x24, 0x92, 0x49, 0x0,

    /* U+00CF "Ï" */
    0xa9, 0x24, 0x92, 0x48,

    /* U+00D0 "Ð" */
    0x79, 0x14, 0x11, 0xe5, 0x14, 0x51, 0x78,

    /* U+00D1 "Ñ" */
    0x29, 0x48, 0x61, 0xc6, 0x99, 0x63, 0x86, 0x18,
    0x40,

    /* U+00D2 "Ò" */
    0x60, 0x87, 0x0, 0x86, 0x18, 0x61, 0x86, 0x17,
    0x0,

    /* U+00D3 "Ó" */
    0x30, 0x87, 0x0, 0x86, 0x18, 0x61, 0x86, 0x17,
    0x0,

    /* U+00D4 "Ô" */
    0x21, 0x47, 0x0, 0x86, 0x18, 0x61, 0x86, 0x17,
    0x0,

    /* U+00D5 "Õ" */
    0x29, 0x47, 0x80, 0x86, 0x18, 0x61, 0x86, 0x17,
    0x80,

    /* U+00D6 "Ö" */
    0x51, 0xc0, 0x21, 0x86, 0x18, 0x61, 0x85, 0xc0,

    /* U+00D8 "Ø" */
    0x3a, 0x1, 0x12, 0x24, 0xca, 0x99, 0x22, 0xb8,

    /* U+00D9 "Ù" */
    0x60, 0x88, 0x61, 0x86, 0x18, 0x61, 0x84, 0x7,
    0x0,

    /* U+00DA "Ú" */
    0x30, 0x88, 0x61, 0x86, 0x18, 0x61, 0x84, 0x7,
    0x0,

    /* U+00DB "Û" */
    0x21, 0x48, 0x61, 0x86, 0x18, 0x61, 0x84, 0x7,
    0x0,

    /* U+00DC "Ü" */
    0x52, 0x18, 0x61, 0x86, 0x18, 0x61, 0x1, 0xc0,

    /* U+00DD "Ý" */
    0x38, 0x88, 0x61, 0x1, 0x23, 0x8, 0x20, 0x82,
    0x0,

    /* U+00DE "Þ" */
    0x84, 0x3d, 0x8, 0xc7, 0xd0, 0x80,

    /* U+00DF "ß" */
    0x72, 0x29, 0x24, 0x8a, 0x8, 0x6a, 0x90,

    /* U+00E0 "à" */
    0x6c, 0xe3, 0x19, 0xb4,

    /* U+00E1 "á" */
    0x13, 0x80, 0xe0, 0xbc, 0x33, 0x68,

    /* U+00E2 "â" */
    0x22, 0x80, 0xe0, 0xbc, 0x33, 0x68,

    /* U+00E3 "ã" */
    0x55, 0x1, 0xc0, 0xbc, 0x73, 0x78,

    /* U+00E4 "ä" */
    0x50, 0x1c, 0x17, 0x86, 0x6d,

    /* U+00E5 "å" */
    0x22, 0x88, 0x7, 0x5, 0xe1, 0x9b, 0x40,

    /* U+00E6 "æ" */
    0x76, 0x9, 0x7e, 0x8, 0x98, 0x67,

    /* U+00E7 "ç" */
    0x78, 0x88, 0x87, 0x22,

    /* U+00E8 "è" */
    0x43, 0x80, 0xe8, 0xfa, 0x10, 0x70,

    /* U+00E9 "é" */
    0x13, 0x0, 0xe8, 0xfa, 0x10, 0x70,

    /* U+00EA "ê" */
    0x22, 0x80, 0xe8, 0xfa, 0x10, 0x70,

    /* U+00EB "ë" */
    0x50, 0x1d, 0x1f, 0x42, 0xe,

    /* U+00EC "ì" */
    0xc7, 0x2, 0x22, 0x22, 0x20,

    /* U+00ED "í" */
    0x3e, 0x4, 0x44, 0x44, 0x40,

    /* U+00EE "î" */
    0x54, 0x24, 0x92, 0x40,

    /* U+00EF "ï" */
    0xa1, 0x24, 0x92,

    /* U+00F0 "ð" */
    0xe0, 0xc8, 0xf8, 0xc6, 0x31, 0x70,

    /* U+00F1 "ñ" */
    0x55, 0x1, 0x6c, 0xc6, 0x31, 0x88,

    /* U+00F2 "ò" */
    0x43, 0x80, 0xe8, 0xc6, 0x31, 0x70,

    /* U+00F3 "ó" */
    0x13, 0x80, 0xe8, 0xc6, 0x31, 0x70,

    /* U+00F4 "ô" */
    0x22, 0x80, 0xe8, 0xc6, 0x31, 0x70,

    /* U+00F5 "õ" */
    0x55, 0x0, 0xe8, 0xc6, 0x31, 0x70,

    /* U+00F6 "ö" */
    0x50, 0x1d, 0x18, 0xc6, 0x2e,

    /* U+00F7 "÷" */
    0x27, 0xc0, 0x40,

    /* U+00F8 "ø" */
    0x3d, 0x35, 0x59, 0x46, 0xe0,

    /* U+00F9 "ù" */
    0x43, 0x81, 0x18, 0xc6, 0x33, 0x68,

    /* U+00FA "ú" */
    0x13, 0x81, 0x18, 0xc6, 0x33, 0x68,

    /* U+00FB "û" */
    0x22, 0x81, 0x18, 0xc6, 0x33, 0x68,

    /* U+00FC "ü" */
    0x50, 0x23, 0x18, 0xc6, 0x6d,

    /* U+00FD "ý" */
    0x9, 0x81, 0x18, 0xc6, 0x29, 0x38, 0x42, 0xe0,

    /* U+00FE "þ" */
    0x84, 0x21, 0x6c, 0xc6, 0x39, 0xb4, 0x21, 0x0,

    /* U+00FF "ÿ" */
    0x50, 0x23, 0x18, 0xc5, 0x27, 0x8, 0x5c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 77, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 59, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 51, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 4, .adv_w = 139, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 10, .adv_w = 103, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 19, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 29, .box_w = 1, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 34, .adv_w = 59, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 38, .adv_w = 59, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 42, .adv_w = 59, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 44, .adv_w = 88, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 48, .adv_w = 29, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 49, .adv_w = 88, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 50, .adv_w = 29, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 66, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 29, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 29, .box_w = 1, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 88, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 127, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 88, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 95, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 88, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 29, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 73, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 117, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 250, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 117, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 59, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 59, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 88, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 328, .adv_w = 95, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 59, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 331, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 81, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 73, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 365, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 29, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 29, .box_w = 2, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 376, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 51, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 103, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 403, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 409, .adv_w = 73, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 73, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 425, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 117, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 444, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 66, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 29, .box_w = 1, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 454, .adv_w = 66, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 95, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 460, .adv_w = 29, .box_w = 1, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 468, .adv_w = 88, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 103, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 478, .adv_w = 51, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 480, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 487, .adv_w = 95, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 491, .adv_w = 88, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 493, .adv_w = 73, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 495, .adv_w = 103, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 501, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 73, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 509, .adv_w = 81, .box_w = 3, .box_h = 4, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 511, .adv_w = 110, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 81, .box_w = 4, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 95, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 95, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 537, .adv_w = 95, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 544, .adv_w = 95, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 95, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 95, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 566, .adv_w = 147, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 581, .adv_w = 88, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 88, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 88, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 88, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 59, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 614, .adv_w = 59, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 620, .adv_w = 59, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 59, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 629, .adv_w = 117, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 654, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 681, .adv_w = 103, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 132, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 697, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 706, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 715, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 724, .adv_w = 103, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 103, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 747, .adv_w = 103, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 88, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 758, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 770, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 776, .adv_w = 88, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 781, .adv_w = 88, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 788, .adv_w = 147, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 794, .adv_w = 81, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 798, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 804, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 810, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 88, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 59, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 826, .adv_w = 59, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 831, .adv_w = 59, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 835, .adv_w = 59, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 844, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 856, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 862, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 868, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 874, .adv_w = 88, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 88, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 882, .adv_w = 103, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 893, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 899, .adv_w = 88, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 88, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 910, .adv_w = 88, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 918, .adv_w = 88, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 926, .adv_w = 88, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_1[] = {
    0, 1, 0, 0, 0, 0, 2, 0,
    3, 4, 5, 0, 0, 6, 0, 0,
    0, 7, 8, 0, 9, 10, 0, 0,
    11, 12, 13
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 161, .range_length = 27, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_1, .list_length = 27, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 191, .range_length = 24, .glyph_id_start = 110,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 216, .range_length = 40, .glyph_id_start = 134,
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
    .cmap_num = 4,
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
const lv_font_t frixos_12 = {
#else
lv_font_t frixos_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FRIXOS_12*/
