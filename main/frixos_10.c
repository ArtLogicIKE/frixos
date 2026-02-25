/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: --bpp 1 --size 10 --no-compress --stride 1 --align 1 --font F10-digits.ttf --range 0-255 --format lvgl -o frixos_10.c
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



#ifndef FRIXOS_10
#define FRIXOS_10 1
#endif

#if FRIXOS_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x52, 0xbe, 0xa5, 0x7d, 0x4a,

    /* U+0024 "$" */
    0x31, 0xec, 0xf0, 0x78, 0x3c, 0xf3, 0x78, 0xc0,

    /* U+0025 "%" */
    0x46, 0x9a, 0x94, 0x29, 0x59, 0x62,

    /* U+0026 "&" */
    0x31, 0xec, 0xf0, 0x33, 0xc, 0xf3, 0x78, 0xc0,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x7b, 0x6d, 0xb3,

    /* U+0029 ")" */
    0xcd, 0xb6, 0xde,

    /* U+002A "*" */
    0xaa, 0x80,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xd0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x18, 0xcc, 0x66, 0x33, 0x18,

    /* U+0030 "0" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0031 "1" */
    0x7d, 0xb6, 0xdb,

    /* U+0032 "2" */
    0x7b, 0x3c, 0xc6, 0x31, 0x8c, 0xff,

    /* U+0033 "3" */
    0x7b, 0x30, 0xce, 0xf, 0x3c, 0xde,

    /* U+0034 "4" */
    0x3d, 0xbc, 0xf3, 0xfc, 0x30, 0xc3,

    /* U+0035 "5" */
    0xff, 0xc, 0x3e, 0xc, 0x3c, 0xfe,

    /* U+0036 "6" */
    0x7b, 0x3c, 0x3e, 0xcf, 0x3c, 0xde,

    /* U+0037 "7" */
    0xfc, 0x30, 0xc6, 0x30, 0xc3, 0xc,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xde, 0xcf, 0x3c, 0xde,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xdf, 0xf, 0x3c, 0xde,

    /* U+003A ":" */
    0xcc,

    /* U+003B ";" */
    0xcd,

    /* U+003C "<" */
    0x2a, 0x22,

    /* U+003D "=" */
    0xf0, 0xf0,

    /* U+003E ">" */
    0x88, 0xa8,

    /* U+003F "?" */
    0x7b, 0x3c, 0xf3, 0x1c, 0xc0, 0xc,

    /* U+0040 "@" */
    0x7d, 0x6, 0xed, 0x5b, 0xb1, 0xe0, 0x3e,

    /* U+0041 "A" */
    0x31, 0x2c, 0xf3, 0xff, 0x3c, 0xf3,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3c, 0xfe,

    /* U+0043 "C" */
    0x7b, 0x3c, 0xf0, 0xc3, 0x3c, 0xde,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xfe,

    /* U+0045 "E" */
    0xff, 0xc, 0x3e, 0xc3, 0xc, 0x3f,

    /* U+0046 "F" */
    0xff, 0xc, 0x3e, 0xc3, 0xc, 0x30,

    /* U+0047 "G" */
    0x7b, 0x3c, 0xf0, 0xc3, 0x7c, 0xdf,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xf3,

    /* U+0049 "I" */
    0xff, 0xff,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xf, 0x3c, 0xde,

    /* U+004B "K" */
    0xcf, 0x3d, 0xbc, 0xdb, 0x3c, 0xf3,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x3f,

    /* U+004D "M" */
    0xc7, 0xdf, 0x5e, 0x3c, 0x78, 0xf1, 0xe3,

    /* U+004E "N" */
    0xcf, 0x3e, 0xf7, 0xcf, 0x3c, 0xf3,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xfe, 0xc3, 0xc, 0x30,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3d, 0xdf,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3c, 0xf3,

    /* U+0053 "S" */
    0x7b, 0x3c, 0x1e, 0xf, 0x3c, 0xde,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcd, 0x27, 0x8c,

    /* U+0057 "W" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x7a, 0xfb, 0xe3,

    /* U+0058 "X" */
    0xcf, 0x3c, 0xde, 0xcf, 0x3c, 0xf3,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xde, 0x30, 0xc3, 0xc,

    /* U+005A "Z" */
    0xfc, 0x30, 0xc4, 0x23, 0xc, 0x3f,

    /* U+005B "[" */
    0xea, 0xab,

    /* U+005C "\\" */
    0xc6, 0x18, 0xc3, 0x18, 0x63,

    /* U+005D "]" */
    0xd5, 0x57,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x7f, 0x3c, 0xf3, 0x7c,

    /* U+0062 "b" */
    0xc3, 0xc, 0x3e, 0xcf, 0x3c, 0xfe,

    /* U+0063 "c" */
    0x7b, 0x3c, 0x33, 0x78,

    /* U+0064 "d" */
    0xc, 0x30, 0xdf, 0xcf, 0x3c, 0xdf,

    /* U+0065 "e" */
    0x7b, 0x3f, 0xf0, 0x7c,

    /* U+0066 "f" */
    0x7b, 0x3c, 0xf0, 0xff, 0xc, 0x30,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0x7c, 0x3f, 0x80,

    /* U+0068 "h" */
    0xc3, 0xc, 0x3e, 0xcf, 0x3c, 0xf3,

    /* U+0069 "i" */
    0xcf, 0xfc,

    /* U+006A "j" */
    0xcf, 0xff, 0xc0,

    /* U+006B "k" */
    0xc3, 0xc, 0x33, 0xcf, 0xcc, 0xf3,

    /* U+006C "l" */
    0xff, 0xff,

    /* U+006D "m" */
    0xfd, 0xaf, 0x5e, 0xbd, 0x60,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcc,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0x78,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0x7c, 0x30, 0xc0,

    /* U+0072 "r" */
    0xfb, 0x3c, 0xf0, 0xc0,

    /* U+0073 "s" */
    0xff, 0xf, 0xc3, 0xfc,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x60,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xde, 0x30,

    /* U+0077 "w" */
    0xd7, 0xaf, 0x5e, 0xb7, 0xe0,

    /* U+0078 "x" */
    0xcf, 0x37, 0xb3, 0xcc,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xfc, 0x3f, 0x80,

    /* U+007A "z" */
    0xfc, 0x33, 0x30, 0xfc,

    /* U+007B "{" */
    0x7b, 0x3d, 0xb3,

    /* U+007C "|" */
    0xff, 0xff,

    /* U+007D "}" */
    0xcd, 0xe6, 0xde,

    /* U+007E "~" */
    0x5a,

    /* U+00A1 "¡" */
    0xcf, 0xff,

    /* U+00A2 "¢" */
    0x31, 0xec, 0xf0, 0xcd, 0xe3, 0x0,

    /* U+00A3 "£" */
    0x39, 0x86, 0x3c, 0x61, 0x86, 0x3f,

    /* U+00A4 "¤" */
    0x85, 0xe4, 0x92, 0x7a, 0x10,

    /* U+00A5 "¥" */
    0xcf, 0x3c, 0xde, 0xfc, 0xcf, 0xcc,

    /* U+00A6 "¦" */
    0xfc, 0x3f,

    /* U+00A7 "§" */
    0xfa, 0x17, 0x22, 0x44, 0xe8, 0x5f,

    /* U+00A8 "¨" */
    0xa0,

    /* U+00A9 "©" */
    0x7d, 0x6, 0xed, 0x1b, 0xb0, 0x5f, 0x0,

    /* U+00AA "ª" */
    0x7b, 0xb7,

    /* U+00AB "«" */
    0x2a, 0xa8, 0xa2, 0x80,

    /* U+00AC "¬" */
    0xfc, 0x10,

    /* U+00AE "®" */
    0xfa, 0x80,

    /* U+00AF "¯" */
    0xf0,

    /* U+00B0 "°" */
    0x69, 0x60,

    /* U+00B1 "±" */
    0x23, 0x88, 0xf, 0x80,

    /* U+00B2 "²" */
    0xe7, 0xce,

    /* U+00B3 "³" */
    0xe7, 0x9e,

    /* U+00B4 "´" */
    0x60,

    /* U+00B5 "µ" */
    0xcf, 0x3c, 0xf3, 0xff, 0xc, 0x0,

    /* U+00B6 "¶" */
    0x7f, 0xdf, 0x5d, 0x14, 0x51, 0x45,

    /* U+00B7 "·" */
    0x80,

    /* U+00B8 "¸" */
    0x98, 0xe0,

    /* U+00B9 "¹" */
    0xd5, 0x40,

    /* U+00BA "º" */
    0xf6, 0xde,

    /* U+00BB "»" */
    0xa2, 0x8a, 0xaa, 0x0,

    /* U+00BC "¼" */
    0xc1, 0x21, 0x11, 0x9, 0x55, 0x29, 0x1d, 0x3,
    0x1,

    /* U+00BD "½" */
    0xc1, 0x21, 0x11, 0x9, 0x75, 0x9, 0x1d, 0x9,
    0x7,

    /* U+00BE "¾" */
    0xe0, 0x88, 0x4e, 0x20, 0x95, 0xe9, 0x44, 0x72,
    0x5, 0x1,

    /* U+00BF "¿" */
    0x30, 0x3, 0x38, 0xcf, 0x3c, 0xde,

    /* U+00C0 "À" */
    0x20, 0x43, 0x12, 0xcf, 0x3f, 0xf3, 0xcf, 0x30,

    /* U+00C1 "Á" */
    0x10, 0x83, 0x12, 0xcf, 0x3f, 0xf3, 0xcf, 0x30,

    /* U+00C2 "Â" */
    0x21, 0x43, 0x12, 0xcf, 0x3f, 0xf3, 0xcf, 0x30,

    /* U+00C3 "Ã" */
    0x29, 0x43, 0x12, 0xcf, 0x3f, 0xf3, 0xcf, 0x30,

    /* U+00C4 "Ä" */
    0x48, 0x3, 0x12, 0xcf, 0x3f, 0xf3, 0xcf, 0x30,

    /* U+00C5 "Å" */
    0x31, 0x23, 0xc, 0x4b, 0x3c, 0xff, 0xcf, 0x3c,
    0xc0,

    /* U+00C6 "Æ" */
    0x1f, 0xf1, 0xb0, 0x33, 0x3, 0x3e, 0x63, 0x7,
    0xf0, 0xc3, 0xc, 0x3f,

    /* U+00C7 "Ç" */
    0x7b, 0x3c, 0xf0, 0xc3, 0x3c, 0xde, 0x10, 0x80,

    /* U+00C8 "È" */
    0x20, 0x4f, 0xf0, 0xc3, 0xec, 0x30, 0xc3, 0xf0,

    /* U+00C9 "É" */
    0x10, 0x8f, 0xf0, 0xc3, 0xec, 0x30, 0xc3, 0xf0,

    /* U+00CA "Ê" */
    0x21, 0x4f, 0xf0, 0xc3, 0xec, 0x30, 0xc3, 0xf0,

    /* U+00CB "Ë" */
    0x48, 0xf, 0xf0, 0xc3, 0xec, 0x30, 0xc3, 0xf0,

    /* U+00CC "Ì" */
    0x9f, 0xff, 0xf0,

    /* U+00CD "Í" */
    0x6f, 0xff, 0xf0,

    /* U+00CE "Î" */
    0x55, 0xb6, 0xdb, 0x6c,

    /* U+00CF "Ï" */
    0x90, 0x66, 0x66, 0x66, 0x66,

    /* U+00D0 "Ð" */
    0x7c, 0xcd, 0x9f, 0xb6, 0x6c, 0xd9, 0xbe,

    /* U+00D1 "Ñ" */
    0x29, 0x4c, 0xf3, 0xef, 0x7c, 0xf3, 0xcf, 0x30,

    /* U+00D2 "Ò" */
    0x20, 0x47, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00D3 "Ó" */
    0x10, 0x87, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00D4 "Ô" */
    0x21, 0x47, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00D5 "Õ" */
    0x29, 0x47, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00D6 "Ö" */
    0x48, 0x7, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00D7 "×" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+00D8 "Ø" */
    0x7f, 0x3c, 0xf7, 0xef, 0x3c, 0xfe,

    /* U+00D9 "Ù" */
    0x20, 0x4c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DA "Ú" */
    0x10, 0x8c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DB "Û" */
    0x21, 0x4c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DC "Ü" */
    0x48, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00DD "Ý" */
    0x10, 0x8c, 0xf3, 0xcd, 0xe3, 0xc, 0x30, 0xc0,

    /* U+00DE "Þ" */
    0xc3, 0xec, 0xf3, 0xfb, 0x0,

    /* U+00DF "ß" */
    0x73, 0x3d, 0xf4, 0xdf, 0x3c, 0xf6,

    /* U+00E0 "à" */
    0x20, 0x47, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00E1 "á" */
    0x10, 0x87, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00E2 "â" */
    0x21, 0x47, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00E3 "ã" */
    0x29, 0x47, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00E4 "ä" */
    0x48, 0x7, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00E5 "å" */
    0x31, 0x23, 0x1f, 0xcf, 0x3c, 0xdf,

    /* U+00E6 "æ" */
    0xff, 0x83, 0x3f, 0xff, 0x30, 0xff, 0xc0,

    /* U+00E7 "ç" */
    0x7b, 0x3c, 0x33, 0x78, 0x43, 0x0,

    /* U+00E8 "è" */
    0x20, 0x47, 0xb3, 0xff, 0x7, 0xc0,

    /* U+00E9 "é" */
    0x10, 0x87, 0xb3, 0xff, 0x7, 0xc0,

    /* U+00EA "ê" */
    0x31, 0x27, 0xb3, 0xff, 0x7, 0xc0,

    /* U+00EB "ë" */
    0x48, 0x7, 0xb3, 0xff, 0x7, 0xc0,

    /* U+00EC "ì" */
    0x93, 0xff,

    /* U+00ED "í" */
    0x63, 0xff,

    /* U+00EE "î" */
    0x54, 0x36, 0xdb,

    /* U+00EF "ï" */
    0x90, 0x66, 0x66, 0x60,

    /* U+00F0 "ð" */
    0x23, 0xe2, 0xdf, 0xcf, 0x3c, 0xde,

    /* U+00F1 "ñ" */
    0x29, 0x4f, 0xb3, 0xcf, 0x3c, 0xc0,

    /* U+00F2 "ò" */
    0x20, 0x47, 0xb3, 0xcf, 0x37, 0x80,

    /* U+00F3 "ó" */
    0x10, 0x87, 0xb3, 0xcf, 0x37, 0x80,

    /* U+00F4 "ô" */
    0x21, 0x47, 0xb3, 0xcf, 0x37, 0x80,

    /* U+00F5 "õ" */
    0x29, 0x47, 0xb3, 0xcf, 0x37, 0x80,

    /* U+00F6 "ö" */
    0x48, 0x7, 0xb3, 0xcf, 0x37, 0x80,

    /* U+00F7 "÷" */
    0x20, 0x3e, 0x2, 0x0,

    /* U+00F8 "ø" */
    0x7f, 0x7e, 0xf3, 0xf8,

    /* U+00F9 "ù" */
    0x20, 0x4c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00FA "ú" */
    0x10, 0x8c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00FB "û" */
    0x31, 0x2c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00FC "ü" */
    0x48, 0xc, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+00FD "ý" */
    0x10, 0x8c, 0xf3, 0xcf, 0x3f, 0xc3, 0xf8,

    /* U+00FE "þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0xec, 0x30,

    /* U+00FF "ÿ" */
    0x48, 0xc, 0xf3, 0xcf, 0x3f, 0xc3, 0xf8
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 4, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 17, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 31, .adv_w = 32, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 32, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 44, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 46, .adv_w = 48, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 111, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 115, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 295, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 297, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 333, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 344, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 371, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 377, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 393, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 412, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 425, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -5},
    {.bitmap_index = 427, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 433, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 64, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 459, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 468, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 474, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 476, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 477, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 479, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 485, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 487, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 488, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 494, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 32, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 501, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 503, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 505, .adv_w = 64, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 507, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 160, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 529, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 539, .adv_w = 80, .box_w = 6, .box_h = 8, .ofs_x = -2, .ofs_y = -5},
    {.bitmap_index = 545, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 585, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 594, .adv_w = 208, .box_w = 12, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 614, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 622, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 638, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 646, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 649, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 48, .box_w = 3, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 656, .adv_w = 64, .box_w = 4, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 112, .box_w = 7, .box_h = 8, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 676, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 692, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 700, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 720, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 742, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 750, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 758, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 766, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 777, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 795, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 807, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 813, .adv_w = 176, .box_w = 10, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 820, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 826, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 844, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 854, .adv_w = 48, .box_w = 3, .box_h = 8, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 857, .adv_w = 64, .box_w = 4, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 861, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 867, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 885, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 891, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 897, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 903, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 907, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 911, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 917, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 923, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 929, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 935, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 942, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 948, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2}
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
        .range_start = 161, .range_length = 12, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 174, .range_length = 82, .glyph_id_start = 108,
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
    .cmap_num = 3,
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
const lv_font_t frixos_10 = {
#else
lv_font_t frixos_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 5,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 1,
    .underline_thickness = 0,
#endif    
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FRIXOS_10*/
