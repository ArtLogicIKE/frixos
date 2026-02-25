/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: --bpp 1 --size 14 --no-compress --stride 1 --align 1 --font F14-mania.ttf --range 0-255 --format lvgl -o frixos_14.c
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



#ifndef FRIXOS_14
#define FRIXOS_14 1
#endif

#if FRIXOS_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xfc, 0xf0,

    /* U+0022 "\"" */
    0xde, 0xe4,

    /* U+0023 "#" */
    0x6c, 0xdb, 0xff, 0xf6, 0xdf, 0xff, 0xb6, 0x6c,

    /* U+0024 "$" */
    0x30, 0xc7, 0xbf, 0xcd, 0xc3, 0xb3, 0xfd, 0xe3,
    0xc,

    /* U+0025 "%" */
    0x4e, 0xb5, 0x86, 0x30, 0xc6, 0x1a, 0xd7, 0x20,

    /* U+0026 "&" */
    0x38, 0xf9, 0xb3, 0xe3, 0x8f, 0xff, 0xee, 0xfe,
    0xec,

    /* U+0027 "'" */
    0xf8,

    /* U+0028 "(" */
    0x7f, 0x6d, 0xb6, 0xec,

    /* U+0029 ")" */
    0xdd, 0xb6, 0xdb, 0xf8,

    /* U+002A "*" */
    0xab, 0x9d, 0x50,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0xf8,

    /* U+002D "-" */
    0xff, 0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0xc, 0x31, 0x86, 0x30, 0xc6, 0x18, 0xc3, 0x0,

    /* U+0030 "0" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+0031 "1" */
    0x7d, 0xb6, 0xdb, 0x6c,

    /* U+0032 "2" */
    0x7b, 0xfc, 0xc7, 0x39, 0xc6, 0x38, 0xff, 0xf0,

    /* U+0033 "3" */
    0x7b, 0xfc, 0xc3, 0x38, 0xe0, 0xf3, 0xfd, 0xe0,

    /* U+0034 "4" */
    0x18, 0xe3, 0x9e, 0x5b, 0x6f, 0xff, 0x18, 0x60,

    /* U+0035 "5" */
    0xff, 0xfc, 0x30, 0xfb, 0xf0, 0xf3, 0xfd, 0xe0,

    /* U+0036 "6" */
    0x7b, 0xfc, 0xf0, 0xfb, 0xfc, 0xf3, 0xfd, 0xe0,

    /* U+0037 "7" */
    0xff, 0xf0, 0xc6, 0x18, 0x63, 0xc, 0x30, 0xc0,

    /* U+0038 "8" */
    0x7b, 0xfc, 0xf3, 0x79, 0xec, 0xf3, 0xfd, 0xe0,

    /* U+0039 "9" */
    0x7b, 0xfc, 0xf3, 0xfd, 0xf0, 0xf3, 0xfd, 0xe0,

    /* U+003A ":" */
    0xf0, 0x0, 0xf0,

    /* U+003B ";" */
    0xf0, 0x3, 0xe0,

    /* U+003C "<" */
    0xc, 0xff, 0x30, 0xf0, 0xf0, 0xc0,

    /* U+003D "=" */
    0xff, 0xf0, 0x3f, 0xfc,

    /* U+003E ">" */
    0xc3, 0xc3, 0xc3, 0x3f, 0xcc, 0x0,

    /* U+003F "?" */
    0x7b, 0xfc, 0xc7, 0x38, 0xc3, 0x0, 0x30, 0xc0,

    /* U+0040 "@" */
    0x7d, 0xff, 0x1e, 0xfd, 0x7b, 0xf6, 0x63, 0xfe,
    0xf8,

    /* U+0041 "A" */
    0x7b, 0xfc, 0xf3, 0xcf, 0xff, 0xf3, 0xcf, 0x30,

    /* U+0042 "B" */
    0xfb, 0xfc, 0xf3, 0xfb, 0xec, 0xf3, 0xff, 0xe0,

    /* U+0043 "C" */
    0x7b, 0xfc, 0xf3, 0xc3, 0xc, 0xf3, 0xfd, 0xe0,

    /* U+0044 "D" */
    0xfb, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xff, 0xe0,

    /* U+0045 "E" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xff, 0xf0,

    /* U+0046 "F" */
    0xff, 0xfc, 0x30, 0xfb, 0xec, 0x30, 0xc3, 0x0,

    /* U+0047 "G" */
    0x7b, 0xfc, 0xf3, 0xc3, 0x7d, 0xf3, 0xfd, 0xe0,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0xfc, 0xf3, 0xcf, 0x30,

    /* U+0049 "I" */
    0xff, 0xff, 0xf0,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xc, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+004B "K" */
    0xc7, 0x9f, 0x77, 0xcf, 0x1e, 0x3e, 0x6e, 0xcf,
    0x8c,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x18, 0xff, 0xc0,

    /* U+004D "M" */
    0xc7, 0x8f, 0xbf, 0xff, 0xfa, 0xf1, 0xe3, 0xc7,
    0x8c,

    /* U+004E "N" */
    0xc7, 0x8f, 0x9f, 0xbf, 0xfb, 0xf3, 0xe3, 0xc7,
    0x8c,

    /* U+004F "O" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+0050 "P" */
    0xfb, 0xfc, 0xf3, 0xff, 0xec, 0x30, 0xc3, 0x0,

    /* U+0051 "Q" */
    0x79, 0xfb, 0x36, 0x6c, 0xd9, 0xb7, 0x66, 0xfc,
    0xf4,

    /* U+0052 "R" */
    0xfb, 0xfc, 0xf3, 0xfb, 0xec, 0xf3, 0xcf, 0x30,

    /* U+0053 "S" */
    0x7b, 0xfc, 0xf8, 0x70, 0xe1, 0xf3, 0xfd, 0xe0,

    /* U+0054 "T" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xff, 0x78, 0xc0,

    /* U+0057 "W" */
    0xc7, 0x8f, 0x1e, 0x3d, 0x7f, 0xff, 0xe7, 0xc7,
    0x8c,

    /* U+0058 "X" */
    0xc7, 0x8f, 0x1f, 0x77, 0xcf, 0xbb, 0xe3, 0xc7,
    0x8c,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0xfd, 0xe3, 0xc, 0x30, 0xc0,

    /* U+005A "Z" */
    0xff, 0xf0, 0xc7, 0x39, 0xce, 0x30, 0xff, 0xf0,

    /* U+005B "[" */
    0xff, 0x6d, 0xb6, 0xfc,

    /* U+005C "\\" */
    0xc3, 0x6, 0x18, 0x30, 0xc1, 0x86, 0xc, 0x30,

    /* U+005D "]" */
    0xfd, 0xb6, 0xdb, 0xfc,

    /* U+005E "^" */
    0x23, 0xbf, 0xb0,

    /* U+005F "_" */
    0xff, 0xfc,

    /* U+0060 "`" */
    0xdd, 0x80,

    /* U+0061 "a" */
    0x79, 0xf0, 0xdf, 0xcf, 0xf7, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xc, 0x3e, 0xff, 0x3c, 0xf3, 0xff, 0xe0,

    /* U+0063 "c" */
    0x7f, 0xfc, 0x30, 0xc3, 0xf7, 0xc0,

    /* U+0064 "d" */
    0xc, 0x30, 0xdf, 0xff, 0x3c, 0xf3, 0xfd, 0xf0,

    /* U+0065 "e" */
    0x7b, 0xfc, 0xff, 0xc3, 0xf7, 0xc0,

    /* U+0066 "f" */
    0x3d, 0xf6, 0x18, 0xf3, 0xc6, 0x18, 0x61, 0x80,

    /* U+0067 "g" */
    0x7f, 0xfc, 0xf3, 0xcf, 0xf7, 0xc3, 0x7d, 0xe0,

    /* U+0068 "h" */
    0xc3, 0xc, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+0069 "i" */
    0xf3, 0xff, 0xc0,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0xfe,

    /* U+006B "k" */
    0xc3, 0xc, 0x33, 0xdf, 0xef, 0x3e, 0xdf, 0x30,

    /* U+006C "l" */
    0xff, 0xff, 0xf0,

    /* U+006D "m" */
    0xfd, 0xff, 0x5e, 0xbc, 0x78, 0xf1, 0x80,

    /* U+006E "n" */
    0xfb, 0xfc, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0xfc, 0xf3, 0xcf, 0xf7, 0x80,

    /* U+0070 "p" */
    0xfb, 0xfc, 0xf3, 0xcf, 0xff, 0xb0, 0xc3, 0x0,

    /* U+0071 "q" */
    0x7d, 0xfb, 0x36, 0x6c, 0xdf, 0x9f, 0x6, 0xe,
    0x18,

    /* U+0072 "r" */
    0xfb, 0xfc, 0xf0, 0xc3, 0xc, 0x0,

    /* U+0073 "s" */
    0x7f, 0xfc, 0x1e, 0xf, 0xff, 0x80,

    /* U+0074 "t" */
    0xc6, 0x3d, 0xec, 0x63, 0x1f, 0x78,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xf7, 0xc0,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xf3, 0xfd, 0xe3, 0x0,

    /* U+0077 "w" */
    0xc7, 0x8f, 0x1e, 0xbf, 0xff, 0xf1, 0x80,

    /* U+0078 "x" */
    0xc7, 0xdd, 0xf1, 0xc7, 0xdd, 0xf1, 0x80,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xf7, 0xc3, 0x7d, 0xe0,

    /* U+007A "z" */
    0xff, 0xfc, 0x71, 0xc7, 0x1f, 0xff, 0x80,

    /* U+007B "{" */
    0x37, 0x66, 0xee, 0x66, 0x73,

    /* U+007C "|" */
    0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xce, 0x66, 0x77, 0x66, 0xec,

    /* U+007E "~" */
    0x77, 0xff, 0x70,

    /* U+00A1 "¡" */
    0xf3, 0xff, 0xf0,

    /* U+00A2 "¢" */
    0x9, 0xff, 0xf4, 0xd3, 0x4f, 0xdf, 0x20,

    /* U+00A3 "£" */
    0x3c, 0xfd, 0x9b, 0xf, 0x9f, 0x18, 0x30, 0x7e,
    0xfc,

    /* U+00A4 "¤" */
    0xba, 0xf9, 0xb3, 0x67, 0xd7, 0x40,

    /* U+00A5 "¥" */
    0xcf, 0x3c, 0xf3, 0xfd, 0xef, 0xcc, 0xfc, 0xc0,

    /* U+00A6 "¦" */
    0xff, 0xf, 0xf0,

    /* U+00A7 "§" */
    0x7b, 0xfc, 0xdc, 0xcf, 0x33, 0xb3, 0xfd, 0xe0,

    /* U+00A8 "¨" */
    0xcf, 0x30,

    /* U+00A9 "©" */
    0x38, 0x8a, 0x6d, 0x1a, 0x33, 0x51, 0x1c,

    /* U+00AA "ª" */
    0x61, 0x79, 0x70,

    /* U+00AB "«" */
    0x36, 0xdb, 0x63, 0x63, 0x60,

    /* U+00AC "¬" */
    0xff, 0xf0, 0xc3,

    /* U+00AD "­" */
    0xff, 0xf0,

    /* U+00AE "®" */
    0x38, 0x8a, 0xed, 0x5b, 0x35, 0x51, 0x1c,

    /* U+00AF "¯" */
    0xff, 0xf0,

    /* U+00B0 "°" */
    0x77, 0xf7, 0xf7, 0x0,

    /* U+00B1 "±" */
    0x30, 0xcf, 0xff, 0x30, 0xc0, 0x3f, 0xfc,

    /* U+00B2 "²" */
    0xc5, 0x70,

    /* U+00B3 "³" */
    0xec, 0xf0,

    /* U+00B4 "´" */
    0x78,

    /* U+00B5 "µ" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xff, 0xb0, 0xc3, 0x0,

    /* U+00B6 "¶" */
    0x7f, 0xff, 0xd7, 0xaf, 0x4e, 0x85, 0xa, 0x14,
    0x28,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0xc7, 0x0,

    /* U+00B9 "¹" */
    0xf0,

    /* U+00BA "º" */
    0x77, 0xf7, 0xf7, 0x0,

    /* U+00BB "»" */
    0xd8, 0xd8, 0xdb, 0x6d, 0x80,

    /* U+00BC "¼" */
    0x8d, 0x1a, 0x64, 0xc3, 0x6, 0x1a, 0xb5, 0xcf,
    0x84,

    /* U+00BD "½" */
    0x8e, 0x39, 0xa6, 0x30, 0xc6, 0xd9, 0xcb, 0x30,

    /* U+00BE "¾" */
    0xcc, 0x9a, 0x62, 0xcb, 0x6, 0x1a, 0xb5, 0xcf,
    0x84,

    /* U+00BF "¿" */
    0x30, 0xc0, 0xc, 0x31, 0xce, 0x33, 0xfd, 0xe0,

    /* U+00C0 "À" */
    0x60, 0xc7, 0xbf, 0xcf, 0x3c, 0xff, 0xff, 0x3c,
    0xf3,

    /* U+00C1 "Á" */
    0x18, 0xc7, 0xbf, 0xcf, 0x3c, 0xff, 0xff, 0x3c,
    0xf3,

    /* U+00C2 "Â" */
    0x31, 0x20, 0x1e, 0xff, 0x3c, 0xf3, 0xff, 0xfc,
    0xf3, 0xcc,

    /* U+00C3 "Ã" */
    0x29, 0x40, 0x1e, 0xff, 0x3c, 0xf3, 0xff, 0xfc,
    0xf3, 0xcc,

    /* U+00C4 "Ä" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xff, 0xfc,
    0xf3, 0xcc,

    /* U+00C5 "Å" */
    0x31, 0x23, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xff,
    0xf3, 0xcf, 0x30,

    /* U+00C6 "Æ" */
    0x1f, 0xe7, 0xfd, 0xd8, 0x33, 0x6, 0x7c, 0xff,
    0xbf, 0x87, 0x30, 0xc7, 0xff, 0xfc,

    /* U+00C7 "Ç" */
    0x7b, 0xfc, 0xf3, 0xc3, 0xc, 0xf3, 0xfd, 0xe3,
    0x2, 0x30,

    /* U+00C8 "È" */
    0x60, 0xc0, 0x3f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x3f, 0xfc,

    /* U+00C9 "É" */
    0x18, 0xc0, 0x3f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x3f, 0xfc,

    /* U+00CA "Ê" */
    0x31, 0x20, 0x3f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x3f, 0xfc,

    /* U+00CB "Ë" */
    0xcf, 0x30, 0x3f, 0xff, 0xc, 0x3e, 0xfb, 0xc,
    0x3f, 0xfc,

    /* U+00CC "Ì" */
    0x93, 0xff, 0xff, 0xc0,

    /* U+00CD "Í" */
    0x63, 0xff, 0xff, 0xc0,

    /* U+00CE "Î" */
    0x69, 0x6, 0x66, 0x66, 0x66, 0x66, 0x60,

    /* U+00CF "Ï" */
    0xcf, 0x30, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x30,

    /* U+00D0 "Ð" */
    0x7c, 0xfd, 0x9b, 0x3f, 0x7e, 0xd9, 0xb3, 0x7e,
    0xf8,

    /* U+00D1 "Ñ" */
    0x14, 0x50, 0x6, 0x3c, 0x7c, 0xfd, 0xff, 0xdf,
    0x9f, 0x1e, 0x3c, 0x60,

    /* U+00D2 "Ò" */
    0x60, 0xc0, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00D3 "Ó" */
    0x18, 0xc0, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00D4 "Ô" */
    0x31, 0x20, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00D5 "Õ" */
    0x29, 0x40, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00D6 "Ö" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00D7 "×" */
    0xcf, 0x37, 0x9e, 0xff, 0x30,

    /* U+00D8 "Ø" */
    0x3d, 0x7f, 0x66, 0x66, 0x6e, 0x7e, 0x76, 0x66,
    0xfe, 0xbc,

    /* U+00D9 "Ù" */
    0x60, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00DA "Ú" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00DB "Û" */
    0x31, 0x20, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00DC "Ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xff, 0x78,

    /* U+00DD "Ý" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xff, 0x78, 0xc3,
    0xc, 0x30,

    /* U+00DE "Þ" */
    0xc3, 0xd, 0xbf, 0xef, 0xbf, 0xf6, 0xc3, 0x0,

    /* U+00DF "ß" */
    0x73, 0xed, 0xb6, 0xd3, 0x6c, 0xf3, 0xdf, 0x60,

    /* U+00E0 "à" */
    0x20, 0x40, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E1 "á" */
    0x10, 0x80, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E2 "â" */
    0x31, 0x20, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E3 "ã" */
    0x29, 0x40, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E4 "ä" */
    0xcf, 0x30, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E5 "å" */
    0x31, 0x23, 0x0, 0x79, 0xf0, 0xdf, 0xcf, 0xf7,
    0xc0,

    /* U+00E6 "æ" */
    0x7b, 0xcf, 0xfc, 0x39, 0xbf, 0xfc, 0xe1, 0xff,
    0xde, 0xf8,

    /* U+00E7 "ç" */
    0x7f, 0xfc, 0x30, 0xc3, 0xf7, 0xcc, 0x8, 0xc0,

    /* U+00E8 "è" */
    0x20, 0x40, 0x1e, 0xff, 0x3f, 0xf0, 0xfd, 0xf0,

    /* U+00E9 "é" */
    0x10, 0x80, 0x1e, 0xff, 0x3f, 0xf0, 0xfd, 0xf0,

    /* U+00EA "ê" */
    0x31, 0x20, 0x1e, 0xff, 0x3f, 0xf0, 0xfd, 0xf0,

    /* U+00EB "ë" */
    0xcf, 0x30, 0x1e, 0xff, 0x3f, 0xf0, 0xfd, 0xf0,

    /* U+00EC "ì" */
    0x93, 0xff, 0xc0,

    /* U+00ED "í" */
    0x63, 0xff, 0xc0,

    /* U+00EE "î" */
    0x69, 0x6, 0x66, 0x66, 0x60,

    /* U+00EF "ï" */
    0xcf, 0x30, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+00F0 "ð" */
    0x68, 0xc5, 0x9f, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F1 "ñ" */
    0x29, 0x40, 0x3e, 0xff, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+00F2 "ò" */
    0x20, 0x40, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F3 "ó" */
    0x10, 0x80, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F4 "ô" */
    0x31, 0x20, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F5 "õ" */
    0x29, 0x40, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F6 "ö" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x3f, 0xfc, 0x3, 0xc,

    /* U+00F8 "ø" */
    0x3d, 0x7f, 0x4e, 0x5a, 0x72, 0xfe, 0xbc,

    /* U+00F9 "ù" */
    0x20, 0x40, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,

    /* U+00FA "ú" */
    0x10, 0x80, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,

    /* U+00FB "û" */
    0x31, 0x20, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,

    /* U+00FC "ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,

    /* U+00FD "ý" */
    0x10, 0x80, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,
    0xdf, 0x78,

    /* U+00FE "þ" */
    0xc3, 0xef, 0xf3, 0xff, 0xec, 0x0,

    /* U+00FF "ÿ" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xfd, 0xf0,
    0xdf, 0x78
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 90, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 97, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 6, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 23, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 41, .adv_w = 64, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 64, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 52, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 57, .adv_w = 48, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 58, .adv_w = 135, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 60, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 64, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 151, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 157, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 161, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 167, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 224, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 64, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 396, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 64, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 411, .adv_w = 128, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 415, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 465, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 481, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 129, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 519, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 528, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 580, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 82, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 592, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 595, .adv_w = 82, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 600, .adv_w = 128, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 603, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 613, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 622, .adv_w = 128, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 628, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 647, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 11},
    {.bitmap_index = 649, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 656, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 659, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 664, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 667, .adv_w = 135, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 669, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 676, .adv_w = 135, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 678, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 682, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 691, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 693, .adv_w = 112, .box_w = 3, .box_h = 2, .ofs_x = 2, .ofs_y = 11},
    {.bitmap_index = 694, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 702, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 712, .adv_w = 112, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 714, .adv_w = 32, .box_w = 1, .box_h = 4, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 715, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 719, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 724, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 750, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 758, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 767, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 776, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 786, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 796, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 806, .adv_w = 107, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 817, .adv_w = 195, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 831, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 841, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 861, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 871, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 881, .adv_w = 48, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 885, .adv_w = 48, .box_w = 2, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 96, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 896, .adv_w = 113, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 906, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 128, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 927, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 937, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 947, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 957, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 967, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 977, .adv_w = 105, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 982, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 992, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1002, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1012, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1022, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1032, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1042, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1050, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1058, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1066, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1074, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1082, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1090, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1098, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1107, .adv_w = 179, .box_w = 11, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1117, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1125, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1133, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1141, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1149, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1157, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1160, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1163, .adv_w = 74, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1168, .adv_w = 115, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1175, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1183, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1191, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1199, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1207, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1215, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1223, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1231, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 1237, .adv_w = 141, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1244, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1252, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1260, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1268, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1276, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1286, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1292, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3}
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
        .range_start = 161, .range_length = 95, .glyph_id_start = 96,
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
const lv_font_t frixos_14 = {
#else
lv_font_t frixos_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif    
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FRIXOS_14*/
