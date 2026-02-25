/*******************************************************************************
 * Size: 11 px
 * Bpp: 1
 * Opts: --bpp 1 --size 11 --no-compress --stride 1 --align 1 --font F11-ari-w9500-condensed-display.ttf --range 0-255 --format lvgl -o frixos_11.c
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



#ifndef FRIXOS_11
#define FRIXOS_11 1
#endif

#if FRIXOS_11

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3, 0xc0,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xb0,

    /* U+0023 "#" */
    0x36, 0x36, 0xff, 0xff, 0x66, 0xff, 0xff, 0x6c,
    0x6c,

    /* U+0024 "$" */
    0x30, 0xc7, 0xbf, 0xcf, 0x87, 0x87, 0xcf, 0xf7,
    0x8c, 0x30,

    /* U+0025 "%" */
    0x73, 0x3e, 0xcd, 0xe3, 0xf8, 0x7f, 0x87, 0xf1,
    0xec, 0xdf, 0x33, 0x80,

    /* U+0026 "&" */
    0x38, 0xf9, 0xb1, 0xc7, 0x7f, 0xf7, 0x7f, 0x76,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x37, 0x6c, 0xcc, 0xcc, 0x67, 0x30,

    /* U+0029 ")" */
    0xce, 0x63, 0x33, 0x33, 0x6e, 0xc0,

    /* U+002A "*" */
    0x6f, 0x6f,

    /* U+002B "+" */
    0x30, 0xcf, 0xff, 0x30, 0xc0,

    /* U+002C "," */
    0x6f, 0xe0,

    /* U+002D "-" */
    0xff, 0xc0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x18, 0xcc, 0x66, 0x31, 0x98, 0xc0,

    /* U+0030 "0" */
    0x7b, 0xfc, 0xf3, 0xcf, 0x3c, 0xff, 0x78,

    /* U+0031 "1" */
    0x3f, 0xf3, 0x33, 0x33, 0x30,

    /* U+0032 "2" */
    0x7b, 0xfc, 0xc3, 0x1c, 0xe7, 0x3f, 0xfc,

    /* U+0033 "3" */
    0x7b, 0xf0, 0xcf, 0x38, 0x3c, 0xff, 0x78,

    /* U+0034 "4" */
    0xc, 0x38, 0xf3, 0xee, 0xdf, 0xff, 0x86, 0xc,

    /* U+0035 "5" */
    0xff, 0xfc, 0x3e, 0xfc, 0x3c, 0xff, 0x78,

    /* U+0036 "6" */
    0x7b, 0xfc, 0x3e, 0xff, 0x3c, 0xff, 0x78,

    /* U+0037 "7" */
    0xff, 0xf1, 0xc6, 0x38, 0xc7, 0x18, 0x60,

    /* U+0038 "8" */
    0x7b, 0xfc, 0xff, 0x7b, 0x3c, 0xff, 0x78,

    /* U+0039 "9" */
    0x7b, 0xfc, 0xf3, 0xfd, 0xf0, 0xff, 0x78,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0x6c, 0x0, 0xdf, 0xc0,

    /* U+003C "<" */
    0xc, 0xff, 0x3c, 0x3c, 0x30,

    /* U+003D "=" */
    0xff, 0xf0, 0x0, 0xff, 0xf0,

    /* U+003E ">" */
    0xc3, 0xc3, 0xcf, 0xf3, 0x0,

    /* U+003F "?" */
    0x7b, 0xfc, 0xc7, 0x38, 0xc0, 0xc, 0x30,

    /* U+0040 "@" */
    0x3f, 0x8f, 0xfb, 0x83, 0xe7, 0xbd, 0xf7, 0xb6,
    0xf7, 0xfe, 0x6e, 0xe0, 0xf, 0xfc, 0xff, 0x0,

    /* U+0041 "A" */
    0x10, 0x20, 0xe1, 0xc6, 0xcf, 0xbf, 0xe3, 0xc6,

    /* U+0042 "B" */
    0xfb, 0xfc, 0xff, 0xfb, 0x3c, 0xff, 0xf8,

    /* U+0043 "C" */
    0x3c, 0xff, 0x9e, 0xc, 0x18, 0x39, 0xbf, 0x3c,

    /* U+0044 "D" */
    0xf9, 0xfb, 0x3e, 0x3c, 0x78, 0xf3, 0xfe, 0xf8,

    /* U+0045 "E" */
    0xff, 0xfc, 0x3f, 0xff, 0xc, 0x3f, 0xfc,

    /* U+0046 "F" */
    0xff, 0xfc, 0x30, 0xff, 0xfc, 0x30, 0xc0,

    /* U+0047 "G" */
    0x3c, 0xff, 0x9e, 0xc, 0xf9, 0xf9, 0xbf, 0x3c,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0049 "I" */
    0xff, 0xff, 0xc0,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0xef, 0x7f, 0x70,

    /* U+004B "K" */
    0xc7, 0x9f, 0x77, 0xcf, 0x1f, 0x37, 0x67, 0xc6,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x3f, 0xfc,

    /* U+004D "M" */
    0xc3, 0xc3, 0xe7, 0xe7, 0xff, 0xff, 0xff, 0xdb,
    0xdb,

    /* U+004E "N" */
    0xe7, 0xcf, 0xdf, 0xbf, 0xfb, 0xf7, 0xe7, 0xce,

    /* U+004F "O" */
    0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e,
    0x3c,

    /* U+0050 "P" */
    0xfb, 0xfc, 0xf3, 0xff, 0xec, 0x30, 0xc0,

    /* U+0051 "Q" */
    0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0xdb, 0xfe, 0x7f,
    0x3b,

    /* U+0052 "R" */
    0xfb, 0xfc, 0xf3, 0xfb, 0xed, 0xf3, 0xcc,

    /* U+0053 "S" */
    0x7b, 0xfc, 0xf8, 0x78, 0x7c, 0xff, 0x78,

    /* U+0054 "T" */
    0xff, 0xf3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xff, 0x78,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xfd, 0xe7, 0x8c, 0x30,

    /* U+0057 "W" */
    0xc9, 0xe4, 0xf7, 0x7b, 0xbf, 0xfb, 0xb9, 0xdc,
    0xc6, 0x63, 0x0,

    /* U+0058 "X" */
    0xc7, 0x8f, 0xbb, 0xe3, 0x8f, 0xbb, 0xe3, 0xc6,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xff, 0x79, 0xe3, 0xc, 0x30,

    /* U+005A "Z" */
    0xff, 0xfc, 0x38, 0xe3, 0x8e, 0x38, 0x7f, 0xfe,

    /* U+005B "[" */
    0xff, 0xcc, 0xcc, 0xcc, 0xcf, 0xf0,

    /* U+005C "\\" */
    0xc6, 0x18, 0xc3, 0x18, 0xc3, 0x18,

    /* U+005D "]" */
    0xff, 0x33, 0x33, 0x33, 0x3f, 0xf0,

    /* U+005E "^" */
    0x23, 0xbf, 0xb8, 0x80,

    /* U+005F "_" */
    0xff, 0xfc,

    /* U+0060 "`" */
    0xe6, 0x30,

    /* U+0061 "a" */
    0x79, 0xf0, 0xdf, 0xcf, 0xf7, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xd, 0xbf, 0xcf, 0x3c, 0xff, 0xf8,

    /* U+0063 "c" */
    0x7b, 0xec, 0x30, 0xcf, 0xf7, 0x80,

    /* U+0064 "d" */
    0xc, 0x36, 0xff, 0xcf, 0x3c, 0xff, 0x7c,

    /* U+0065 "e" */
    0x7b, 0xfc, 0xff, 0xc3, 0xe7, 0x80,

    /* U+0066 "f" */
    0x3b, 0xd9, 0xff, 0xb1, 0x8c, 0x60,

    /* U+0067 "g" */
    0x6f, 0xfc, 0xf3, 0xfd, 0xf0, 0xff, 0xf8,

    /* U+0068 "h" */
    0xc3, 0xd, 0xbf, 0xef, 0x3c, 0xf3, 0xcc,

    /* U+0069 "i" */
    0xf3, 0xff, 0xf0,

    /* U+006A "j" */
    0x6c, 0x36, 0xdb, 0x6f, 0xe0,

    /* U+006B "k" */
    0xc3, 0xc, 0xf7, 0xfb, 0xcf, 0xb7, 0xcc,

    /* U+006C "l" */
    0xff, 0xff, 0xc0,

    /* U+006D "m" */
    0xb6, 0xff, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb,

    /* U+006E "n" */
    0xdb, 0xfe, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0xfc, 0xf3, 0xcf, 0xf7, 0x80,

    /* U+0070 "p" */
    0xfb, 0xfc, 0xf3, 0xcf, 0xfd, 0xb0, 0xc0,

    /* U+0071 "q" */
    0x7f, 0xfc, 0xf3, 0xcf, 0xf6, 0xc3, 0xc,

    /* U+0072 "r" */
    0xdf, 0xf9, 0x8c, 0x63, 0x0,

    /* U+0073 "s" */
    0x7f, 0xf0, 0xe1, 0xff, 0xc0,

    /* U+0074 "t" */
    0x63, 0x3f, 0xf6, 0x31, 0x8f, 0x38,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xdf, 0xf6, 0xc0,

    /* U+0076 "v" */
    0xcf, 0x3f, 0xde, 0x78, 0xc3, 0x0,

    /* U+0077 "w" */
    0xc7, 0xaf, 0x5f, 0xf7, 0xcd, 0x9b, 0x0,

    /* U+0078 "x" */
    0xcf, 0xf7, 0x8c, 0x7b, 0xfc, 0xc0,

    /* U+0079 "y" */
    0xcf, 0x3f, 0xde, 0x78, 0xc7, 0x38, 0xc0,

    /* U+007A "z" */
    0xff, 0xf3, 0x9c, 0xe3, 0xff, 0xc0,

    /* U+007B "{" */
    0x3b, 0xd8, 0xce, 0x63, 0x8c, 0x63, 0xce,

    /* U+007C "|" */
    0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xe7, 0x8c, 0x63, 0x8c, 0xe6, 0x37, 0xb8,

    /* U+007E "~" */
    0x67, 0xef, 0x7e, 0x60,

    /* U+00A1 "¡" */
    0xf3, 0xff, 0xc0,

    /* U+00A2 "¢" */
    0xc, 0x27, 0xbe, 0xd3, 0xce, 0xff, 0x79, 0xc,
    0x0,

    /* U+00A3 "£" */
    0x39, 0xf6, 0x3e, 0xf9, 0x86, 0x7f, 0xdc,

    /* U+00A4 "¤" */
    0xd6, 0xf9, 0xb3, 0xed, 0x60,

    /* U+00A5 "¥" */
    0xcf, 0x3c, 0xde, 0xfc, 0xcf, 0xcc, 0x30,

    /* U+00A6 "¦" */
    0xff, 0x3f, 0xc0,

    /* U+00A7 "§" */
    0x7b, 0xfc, 0x1e, 0xff, 0x3f, 0xde, 0xf, 0xf7,
    0x80,

    /* U+00A8 "¨" */
    0xcf, 0x30,

    /* U+00A9 "©" */
    0x3e, 0x31, 0xb7, 0x76, 0xdb, 0xd, 0xb7, 0x76,
    0xc6, 0x3e, 0x0,

    /* U+00AA "ª" */
    0x7e, 0xfe, 0xd0,

    /* U+00AB "«" */
    0x3b, 0xbb, 0xbb, 0x8e, 0xe3, 0xb8,

    /* U+00AC "¬" */
    0xff, 0xf0, 0xc3,

    /* U+00AD "­" */
    0xff, 0xc0,

    /* U+00AE "®" */
    0x7d, 0x8e, 0xed, 0x9a, 0xb8, 0xdf, 0x0,

    /* U+00AF "¯" */
    0xf0,

    /* U+00B0 "°" */
    0x77, 0xf7, 0xf7, 0x0,

    /* U+00B1 "±" */
    0x30, 0xcf, 0xff, 0x30, 0xcf, 0xff,

    /* U+00B2 "²" */
    0xf7, 0xce, 0xee, 0x7f, 0xe0,

    /* U+00B3 "³" */
    0xf7, 0xce, 0xe3, 0xff, 0xc0,

    /* U+00B4 "´" */
    0x76, 0xc0,

    /* U+00B5 "µ" */
    0xcf, 0x3c, 0xf3, 0xdf, 0xfe, 0xf0, 0xc0,

    /* U+00B6 "¶" */
    0x7f, 0xff, 0xdf, 0xbf, 0x6e, 0xc5, 0x8b, 0x16,
    0x2c, 0x58,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x7e,

    /* U+00B9 "¹" */
    0x3f, 0xf3, 0x33, 0x30,

    /* U+00BA "º" */
    0x76, 0xf6, 0xe0,

    /* U+00BB "»" */
    0xee, 0x3b, 0x8e, 0xee, 0xee, 0xe0,

    /* U+00BC "¼" */
    0x63, 0x71, 0x19, 0x8d, 0x80, 0x80, 0xcc, 0xca,
    0x4f, 0x61, 0x80,

    /* U+00BD "½" */
    0x63, 0x71, 0x19, 0x8d, 0x80, 0x80, 0xf8, 0xc6,
    0x46, 0x67, 0x80,

    /* U+00BE "¾" */
    0xe3, 0x39, 0xd, 0x9d, 0x80, 0x80, 0xcc, 0xca,
    0x4f, 0x61, 0x80,

    /* U+00BF "¿" */
    0x30, 0xc0, 0xc, 0x73, 0x8c, 0xff, 0x78,

    /* U+00C0 "À" */
    0x70, 0x70, 0x0, 0x81, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C1 "Á" */
    0x1c, 0x70, 0x0, 0x81, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C2 "Â" */
    0x38, 0xd8, 0x0, 0x81, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C3 "Ã" */
    0x77, 0xb8, 0x0, 0x81, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C4 "Ä" */
    0x6c, 0xd8, 0x0, 0x81, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C5 "Å" */
    0x38, 0xd9, 0xb1, 0xc1, 0x7, 0xe, 0x36, 0x7d,
    0xff, 0x1e, 0x30,

    /* U+00C6 "Æ" */
    0x1f, 0x8f, 0xcf, 0x7, 0xf6, 0xfb, 0xe3, 0xf1,
    0x9f, 0xcf, 0x80,

    /* U+00C7 "Ç" */
    0x3c, 0xff, 0x9e, 0xc, 0x18, 0x39, 0xbf, 0x3c,
    0x71, 0xc0,

    /* U+00C8 "È" */
    0x70, 0xe0, 0x3f, 0xff, 0xf, 0xff, 0xc3, 0xf,
    0xff,

    /* U+00C9 "É" */
    0x39, 0xc0, 0x3f, 0xff, 0xf, 0xff, 0xc3, 0xf,
    0xff,

    /* U+00CA "Ê" */
    0x7b, 0xb0, 0x3f, 0xff, 0xf, 0xff, 0xc3, 0xf,
    0xff,

    /* U+00CB "Ë" */
    0xcf, 0x30, 0x3f, 0xff, 0xf, 0xff, 0xc3, 0xf,
    0xff,

    /* U+00CC "Ì" */
    0xe7, 0x6, 0x66, 0x66, 0x66, 0x66,

    /* U+00CD "Í" */
    0x7e, 0x6, 0x66, 0x66, 0x66, 0x66,

    /* U+00CE "Î" */
    0x7b, 0xb0, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc,

    /* U+00CF "Ï" */
    0xcf, 0x30, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc,

    /* U+00D0 "Ð" */
    0x7c, 0x7e, 0x67, 0xf3, 0xf3, 0x63, 0x67, 0x7e,
    0x7c,

    /* U+00D1 "Ñ" */
    0x77, 0xb8, 0x7, 0x3e, 0x7e, 0xfd, 0xff, 0xdf,
    0xbf, 0x3e, 0x70,

    /* U+00D2 "Ò" */
    0x38, 0x1c, 0x0, 0x3c, 0x7e, 0xe7, 0xc3, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+00D3 "Ó" */
    0x1c, 0x38, 0x0, 0x3c, 0x7e, 0xe7, 0xc3, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+00D4 "Ô" */
    0x3c, 0x76, 0x0, 0x3c, 0x7e, 0xe7, 0xc3, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+00D5 "Õ" */
    0x7b, 0xde, 0x0, 0x3c, 0x7e, 0xe7, 0xc3, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+00D6 "Ö" */
    0x66, 0x66, 0x0, 0x3c, 0x7e, 0xe7, 0xc3, 0xc3,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+00D7 "×" */
    0xc7, 0xdd, 0xf1, 0xc7, 0xdd, 0xf1, 0x80,

    /* U+00D8 "Ø" */
    0x3b, 0x7e, 0xe7, 0xcf, 0xdb, 0xf3, 0xe7, 0x7e,
    0xdc,

    /* U+00D9 "Ù" */
    0x70, 0xe0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00DA "Ú" */
    0x39, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00DB "Û" */
    0x7b, 0xb0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00DC "Ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0xde,

    /* U+00DD "Ý" */
    0x39, 0xc0, 0x33, 0xcf, 0x3f, 0xde, 0x78, 0xc3,
    0xc,

    /* U+00DE "Þ" */
    0xc3, 0xef, 0xf3, 0xcf, 0x3f, 0xfe, 0xc0,

    /* U+00DF "ß" */
    0x7b, 0xfc, 0xf7, 0xdb, 0x3c, 0xf7, 0xd8,

    /* U+00E0 "à" */
    0x70, 0xc1, 0x80, 0x79, 0xf0, 0xdf, 0xcf, 0xf7,
    0xc0,

    /* U+00E1 "á" */
    0x1c, 0x63, 0x0, 0x79, 0xf0, 0xdf, 0xcf, 0xf7,
    0xc0,

    /* U+00E2 "â" */
    0x31, 0xec, 0xc0, 0x79, 0xf0, 0xdf, 0xcf, 0xf7,
    0xc0,

    /* U+00E3 "ã" */
    0x77, 0x60, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E4 "ä" */
    0xcf, 0x30, 0x1e, 0x7c, 0x37, 0xf3, 0xfd, 0xf0,

    /* U+00E5 "å" */
    0x31, 0xa3, 0x0, 0x79, 0xf0, 0xdf, 0xcf, 0xf7,
    0xc0,

    /* U+00E6 "æ" */
    0x76, 0x7f, 0x1b, 0x7f, 0xd8, 0xfe, 0x6e,

    /* U+00E7 "ç" */
    0x7b, 0xec, 0x30, 0xcf, 0xf7, 0x8e, 0x70,

    /* U+00E8 "è" */
    0x70, 0xc1, 0x80, 0x7b, 0xfc, 0xff, 0xc3, 0xe7,
    0x80,

    /* U+00E9 "é" */
    0x38, 0xc6, 0x0, 0x7b, 0xfc, 0xff, 0xc3, 0xe7,
    0x80,

    /* U+00EA "ê" */
    0x31, 0xec, 0xc0, 0x7b, 0xfc, 0xff, 0xc3, 0xe7,
    0x80,

    /* U+00EB "ë" */
    0xcf, 0x30, 0x1e, 0xff, 0x3f, 0xf0, 0xf9, 0xe0,

    /* U+00EC "ì" */
    0xe6, 0x30, 0x66, 0x66, 0x66, 0x60,

    /* U+00ED "í" */
    0x76, 0xc0, 0x66, 0x66, 0x66, 0x60,

    /* U+00EE "î" */
    0x31, 0xec, 0xc0, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x0,

    /* U+00EF "ï" */
    0xcf, 0x30, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+00F0 "ð" */
    0x7c, 0xe7, 0x83, 0x6f, 0xfc, 0xf3, 0xfd, 0xe0,

    /* U+00F1 "ñ" */
    0x77, 0x60, 0x36, 0xff, 0xbc, 0xf3, 0xcf, 0x30,

    /* U+00F2 "ò" */
    0xe1, 0x83, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+00F3 "ó" */
    0x1c, 0x63, 0x0, 0x7b, 0xfc, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+00F4 "ô" */
    0x31, 0xec, 0xc0, 0x7b, 0xfc, 0xf3, 0xcf, 0xf7,
    0x80,

    /* U+00F5 "õ" */
    0x77, 0x60, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F6 "ö" */
    0xcf, 0x30, 0x1e, 0xff, 0x3c, 0xf3, 0xfd, 0xe0,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x3f, 0xfc, 0x3, 0xc,

    /* U+00F8 "ø" */
    0x7f, 0xfd, 0xff, 0xef, 0xff, 0x80,

    /* U+00F9 "ù" */
    0xe1, 0x83, 0x0, 0xcf, 0x3c, 0xf3, 0xdf, 0xf6,
    0xc0,

    /* U+00FA "ú" */
    0x1c, 0x63, 0x0, 0xcf, 0x3c, 0xf3, 0xdf, 0xf6,
    0xc0,

    /* U+00FB "û" */
    0x31, 0xec, 0xc0, 0xcf, 0x3c, 0xf3, 0xdf, 0xf6,
    0xc0,

    /* U+00FC "ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf7, 0xfd, 0xb0,

    /* U+00FD "ý" */
    0x1c, 0x63, 0x0, 0xcf, 0x3f, 0xde, 0x78, 0xc7,
    0x38, 0xc0,

    /* U+00FE "þ" */
    0xc3, 0xd, 0xbf, 0xcf, 0x3c, 0xff, 0xdb, 0xc,
    0x0,

    /* U+00FF "ÿ" */
    0xcf, 0x30, 0x33, 0xcf, 0xf7, 0x9e, 0x31, 0xce,
    0x30
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 7, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 26, .adv_w = 176, .box_w = 10, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 47, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 53, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 59, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 66, .adv_w = 64, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 68, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 70, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 112, .box_w = 4, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 112, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 152, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 157, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 162, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 167, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 190, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 274, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 283, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 337, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 391, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 80, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 403, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 407, .adv_w = 112, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 409, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 411, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 456, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 48, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 48, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 471, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 488, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 507, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 514, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 562, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 575, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 578, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 585, .adv_w = 128, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 589, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 592, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 601, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 613, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 620, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 632, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 634, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 648, .adv_w = 160, .box_w = 9, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 654, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 657, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 659, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 666, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 667, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 671, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 677, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 682, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 687, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 689, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 696, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 706, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 707, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 708, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 712, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 715, .adv_w = 160, .box_w = 9, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 721, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 743, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 761, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 772, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 783, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 794, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 805, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 827, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 848, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 857, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 866, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 875, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 884, .adv_w = 48, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 890, .adv_w = 48, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 896, .adv_w = 48, .box_w = 6, .box_h = 12, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 48, .box_w = 6, .box_h = 12, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 914, .adv_w = 128, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 923, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 934, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 946, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 958, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 970, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 982, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 994, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 1001, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1010, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1019, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1028, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1037, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1046, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1055, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1062, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1069, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1078, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1087, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1096, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1104, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1112, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1121, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1128, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1135, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1144, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1153, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1162, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1170, .adv_w = 48, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1176, .adv_w = 48, .box_w = 4, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1182, .adv_w = 48, .box_w = 6, .box_h = 11, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1191, .adv_w = 48, .box_w = 6, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1199, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1207, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1215, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1224, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1233, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1242, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1250, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1258, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1264, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1270, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1279, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1288, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1297, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1305, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1315, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1324, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -2}
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
const lv_font_t frixos_11 = {
#else
lv_font_t frixos_11 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if FRIXOS_11*/
