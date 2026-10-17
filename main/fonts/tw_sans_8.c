/*******************************************************************************
 * Size: 8 px
 * Bpp: 2
 * Opts: --font managed_components/lvgl__lvgl/scripts/built_in_font/Montserrat-Medium.ttf --range 0x20-0x7E --range 0xA0-0xFF --range 0x2190-0x2193 --size 8 --bpp 2 --no-compress --format lvgl --lv-include lvgl.h -o main/fonts/tw_sans_8.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef TW_SANS_8
#define TW_SANS_8 1
#endif

#if TW_SANS_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0x65, 0x50, 0x50,

    /* U+0022 "\"" */
    0x49, 0x20, 0x0,

    /* U+0023 "#" */
    0x14, 0x46, 0xa8, 0x21, 0x7, 0xb8, 0x22, 0x0,

    /* U+0024 "$" */
    0x8, 0xb, 0x85, 0x80, 0xb8, 0x9, 0x9b, 0x80,
    0x80,

    /* U+0025 "%" */
    0x64, 0x42, 0x21, 0x1, 0x65, 0x2, 0x11, 0x10,
    0xa0,

    /* U+0026 "&" */
    0x2a, 0x2, 0x20, 0x2c, 0x8, 0x28, 0x2a, 0x80,
    0x0,

    /* U+0027 "'" */
    0x44, 0x0,

    /* U+0028 "(" */
    0x20, 0x82, 0x8, 0x20, 0x82, 0x0,

    /* U+0029 ")" */
    0x42, 0x23, 0x22, 0x40,

    /* U+002A "*" */
    0x64, 0x78, 0x10,

    /* U+002B "+" */
    0x0, 0x2, 0x6, 0xe0, 0x20,

    /* U+002C "," */
    0x5, 0x40,

    /* U+002D "-" */
    0x64,

    /* U+002E "." */
    0x5,

    /* U+002F "/" */
    0x2, 0x0, 0x80, 0x80, 0x20, 0x14, 0x8, 0x2,
    0x0,

    /* U+0030 "0" */
    0x2a, 0x14, 0x28, 0xd, 0x42, 0x2a, 0x0,

    /* U+0031 "1" */
    0xb0, 0x82, 0x8, 0x20,

    /* U+0032 "2" */
    0x6a, 0x0, 0xc0, 0x60, 0x90, 0xba, 0x0,

    /* U+0033 "3" */
    0x6b, 0x2, 0x1, 0xa0, 0x8, 0xaa, 0x0,

    /* U+0034 "4" */
    0x6, 0x0, 0x80, 0x22, 0xa, 0xb8, 0x2, 0x0,

    /* U+0035 "5" */
    0x3a, 0x14, 0x6, 0xa0, 0x8, 0x6a, 0x0,

    /* U+0036 "6" */
    0x2a, 0x14, 0xa, 0xa1, 0x82, 0x2a, 0x40,

    /* U+0037 "7" */
    0xeb, 0x50, 0xc0, 0x50, 0x30, 0x14, 0x0,

    /* U+0038 "8" */
    0x2a, 0x14, 0x63, 0xb2, 0x42, 0x2a, 0x40,

    /* U+0039 "9" */
    0x6a, 0x20, 0x92, 0xa8, 0x9, 0x2a, 0x0,

    /* U+003A ":" */
    0x50, 0x5,

    /* U+003B ";" */
    0x50, 0x5, 0x40,

    /* U+003C "<" */
    0x0, 0x6, 0x87, 0x0, 0x18, 0x0, 0x0,

    /* U+003D "=" */
    0x6a, 0x0, 0x6, 0xa0,

    /* U+003E ">" */
    0x0, 0xa, 0x0, 0x71, 0x90, 0x0, 0x0,

    /* U+003F "?" */
    0x6a, 0x0, 0xc0, 0x90, 0x0, 0x8, 0x0,

    /* U+0040 "@" */
    0x9, 0x64, 0x26, 0xb5, 0x88, 0x22, 0x88, 0x22,
    0x26, 0xa9, 0xa, 0x50,

    /* U+0041 "A" */
    0x2, 0x80, 0xa, 0x0, 0x82, 0x7, 0xac, 0x20,
    0x8,

    /* U+0042 "B" */
    0x3a, 0x82, 0x9, 0x3a, 0xc2, 0x2, 0x3a, 0x90,

    /* U+0043 "C" */
    0x1a, 0x86, 0x0, 0x80, 0x6, 0x0, 0x1a, 0x80,

    /* U+0044 "D" */
    0x3a, 0x80, 0x80, 0xc2, 0x2, 0x8, 0xc, 0x3a,
    0x80,

    /* U+0045 "E" */
    0x3a, 0x88, 0x3, 0xa4, 0x80, 0x3a, 0x80,

    /* U+0046 "F" */
    0x3a, 0x88, 0x3, 0xa4, 0x80, 0x20, 0x0,

    /* U+0047 "G" */
    0x1a, 0x86, 0x0, 0x80, 0x6, 0x5, 0x1a, 0x80,

    /* U+0048 "H" */
    0x20, 0x22, 0x2, 0x3a, 0xa2, 0x2, 0x20, 0x20,

    /* U+0049 "I" */
    0x22, 0x22, 0x20,

    /* U+004A "J" */
    0x1b, 0x40, 0x50, 0x14, 0x9, 0x2a, 0x0,

    /* U+004B "K" */
    0x20, 0x82, 0x20, 0x3d, 0x3, 0x24, 0x20, 0x90,

    /* U+004C "L" */
    0x20, 0x8, 0x2, 0x0, 0x80, 0x3a, 0x40,

    /* U+004D "M" */
    0x30, 0xc, 0xd0, 0xb2, 0x89, 0xc8, 0xa3, 0x20,
    0xc,

    /* U+004E "N" */
    0x30, 0x23, 0x82, 0x26, 0x22, 0x1e, 0x20, 0x60,

    /* U+004F "O" */
    0x1a, 0x81, 0x80, 0xc8, 0x2, 0x58, 0xc, 0x1a,
    0x80,

    /* U+0050 "P" */
    0x3a, 0x82, 0x9, 0x20, 0x83, 0xa4, 0x20, 0x0,

    /* U+0051 "Q" */
    0x1a, 0x81, 0x80, 0xc8, 0x2, 0x58, 0xc, 0x1a,
    0x80, 0x2, 0x80,

    /* U+0052 "R" */
    0x3a, 0x82, 0x9, 0x20, 0x83, 0xb8, 0x20, 0x80,

    /* U+0053 "S" */
    0x2a, 0x14, 0x2, 0xa0, 0x6, 0x6a, 0x0,

    /* U+0054 "T" */
    0xae, 0x43, 0x0, 0xc0, 0x30, 0xc, 0x0,

    /* U+0055 "U" */
    0x20, 0x62, 0x6, 0x20, 0x63, 0x5, 0x1a, 0x80,

    /* U+0056 "V" */
    0x20, 0x14, 0x60, 0x80, 0xc5, 0x1, 0xb0, 0x3,
    0x40,

    /* U+0057 "W" */
    0x90, 0xc0, 0x88, 0x64, 0x83, 0x22, 0x20, 0x68,
    0xa4, 0xc, 0xc, 0x0,

    /* U+0058 "X" */
    0x60, 0x82, 0x60, 0xd, 0x2, 0x60, 0x50, 0x80,

    /* U+0059 "Y" */
    0x20, 0x20, 0x22, 0x0, 0x68, 0x0, 0xc0, 0x3,
    0x0,

    /* U+005A "Z" */
    0x6b, 0xc0, 0x20, 0x8, 0x2, 0x0, 0xba, 0x80,

    /* U+005B "[" */
    0x34, 0x82, 0x8, 0x20, 0x83, 0x40,

    /* U+005C "\\" */
    0x20, 0x8, 0x1, 0x40, 0x20, 0x8, 0x0, 0x40,
    0x20,

    /* U+005D "]" */
    0xb3, 0x33, 0x33, 0xb0,

    /* U+005E "^" */
    0xc, 0x9, 0x41, 0x20,

    /* U+005F "_" */
    0x55,

    /* U+0060 "`" */
    0x14,

    /* U+0061 "a" */
    0x2a, 0xa, 0xc9, 0x30, 0xac,

    /* U+0062 "b" */
    0x60, 0x6, 0x0, 0x7a, 0x46, 0xc, 0x60, 0xc7,
    0xa4,

    /* U+0063 "c" */
    0x2a, 0x24, 0x9, 0x0, 0xa8,

    /* U+0064 "d" */
    0x0, 0x80, 0x22, 0xaa, 0x42, 0x90, 0x8a, 0xa0,

    /* U+0065 "e" */
    0x2a, 0x2a, 0x99, 0x0, 0xa8,

    /* U+0066 "f" */
    0x28, 0x20, 0xb4, 0x20, 0x20, 0x20,

    /* U+0067 "g" */
    0x2a, 0xa4, 0x39, 0xc, 0xaa, 0x2a, 0x0,

    /* U+0068 "h" */
    0x60, 0x18, 0x7, 0xa5, 0x82, 0x60, 0xd8, 0x30,

    /* U+0069 "i" */
    0x10, 0x66, 0x66,

    /* U+006A "j" */
    0x4, 0x0, 0x82, 0x8, 0x26, 0x40,

    /* U+006B "k" */
    0x60, 0x18, 0x6, 0x25, 0xb0, 0x76, 0x18, 0x50,

    /* U+006C "l" */
    0x66, 0x66, 0x66,

    /* U+006D "m" */
    0x7a, 0xa9, 0x60, 0x82, 0x60, 0x83, 0x60, 0x83,

    /* U+006E "n" */
    0x7a, 0x58, 0x26, 0xd, 0x83,

    /* U+006F "o" */
    0x2a, 0x24, 0x29, 0x8, 0xa8,

    /* U+0070 "p" */
    0x7a, 0x46, 0xc, 0x60, 0xc7, 0xa4, 0x60, 0x0,

    /* U+0071 "q" */
    0x2a, 0xa4, 0x29, 0x8, 0xaa, 0x0, 0x80,

    /* U+0072 "r" */
    0x68, 0x60, 0x60, 0x60,

    /* U+0073 "s" */
    0x69, 0x90, 0x6, 0x69,

    /* U+0074 "t" */
    0x20, 0xb4, 0x20, 0x20, 0x28,

    /* U+0075 "u" */
    0x50, 0x94, 0x26, 0x8, 0xaa,

    /* U+0076 "v" */
    0x20, 0x91, 0x48, 0x9, 0x40, 0x70,

    /* U+0077 "w" */
    0x83, 0x8, 0x56, 0x88, 0x28, 0xa0, 0x28, 0x60,

    /* U+0078 "x" */
    0x52, 0xa, 0x2, 0x81, 0x48,

    /* U+0079 "y" */
    0x20, 0x80, 0x88, 0xa, 0x0, 0x60, 0x28, 0x0,

    /* U+007A "z" */
    0x6a, 0x8, 0x20, 0xba,

    /* U+007B "{" */
    0x24, 0xc3, 0x18, 0x30, 0xc2, 0x40,

    /* U+007C "|" */
    0x22, 0x22, 0x22, 0x20,

    /* U+007D "}" */
    0x90, 0x82, 0xc, 0x20, 0x89, 0x0,

    /* U+007E "~" */
    0x21, 0x11, 0x80,

    /* U+00A0 " " */

    /* U+00A1 "¡" */
    0x50, 0x55, 0x60,

    /* U+00A2 "¢" */
    0x4, 0xb, 0x85, 0x82, 0x20, 0x2e, 0x2, 0x0,

    /* U+00A3 "£" */
    0xa, 0x83, 0x0, 0x75, 0x3, 0x0, 0xba, 0x80,

    /* U+00A4 "¤" */
    0x0, 0x6, 0xa8, 0x20, 0x82, 0x8, 0x6a, 0x80,
    0x0,

    /* U+00A5 "¥" */
    0x24, 0x20, 0x26, 0x40, 0x79, 0x1, 0xa4, 0x2,
    0x0,

    /* U+00A6 "¦" */
    0x22, 0x10, 0x12, 0x20,

    /* U+00A7 "§" */
    0x29, 0x60, 0x95, 0x69, 0x5, 0x68,

    /* U+00A8 "¨" */
    0x44,

    /* U+00A9 "©" */
    0x5, 0x40, 0x59, 0x45, 0x41, 0x15, 0x4, 0x16,
    0x50, 0x15, 0x0,

    /* U+00AA "ª" */
    0x55, 0x65, 0x40,

    /* U+00AB "«" */
    0x25, 0x58, 0x25,

    /* U+00AC "¬" */
    0x6a, 0x0, 0x80, 0x0,

    /* U+00AD "­" */
    0x64,

    /* U+00AE "®" */
    0x5, 0x40, 0x65, 0x44, 0x89, 0x12, 0x84, 0x14,
    0x50, 0x15, 0x0,

    /* U+00AF "¯" */
    0x54,

    /* U+00B0 "°" */
    0x15, 0x11, 0x40,

    /* U+00B1 "±" */
    0x0, 0x2, 0x6, 0xe0, 0x20, 0x6a, 0x0,

    /* U+00B2 "²" */
    0x58, 0x4, 0x64,

    /* U+00B3 "³" */
    0x68, 0x14, 0x58,

    /* U+00B4 "´" */
    0x24,

    /* U+00B5 "µ" */
    0x60, 0x98, 0x26, 0x9, 0xea, 0x60, 0x0,

    /* U+00B6 "¶" */
    0x7e, 0x7d, 0x52, 0x54, 0x15, 0x5, 0x41, 0x50,
    0x0,

    /* U+00B7 "·" */
    0x50,

    /* U+00B8 "¸" */
    0x20, 0x40,

    /* U+00B9 "¹" */
    0x20, 0x42, 0x80,

    /* U+00BA "º" */
    0x58, 0x88, 0x24,

    /* U+00BB "»" */
    0x58, 0x25, 0x58,

    /* U+00BC "¼" */
    0x20, 0x10, 0x4, 0x4, 0x1, 0x8, 0x0, 0x98,
    0x10, 0x4, 0x2a, 0x1, 0x0, 0x0,

    /* U+00BD "½" */
    0x20, 0x10, 0x10, 0x10, 0x10, 0x95, 0x25, 0x2,
    0x1, 0x8, 0x4, 0x15,

    /* U+00BE "¾" */
    0x58, 0x10, 0x5, 0x4, 0x0, 0x88, 0x1, 0x58,
    0x10, 0x4, 0x2a, 0x1, 0x0, 0x0,

    /* U+00BF "¿" */
    0x8, 0x1, 0x2, 0x41, 0x40, 0x2a, 0x0,

    /* U+00C0 "À" */
    0x1, 0x0, 0x4, 0x0, 0x28, 0x0, 0xa0, 0x8,
    0x20, 0x7a, 0xc2, 0x0, 0x80,

    /* U+00C1 "Á" */
    0x0, 0x40, 0x0, 0x0, 0x28, 0x0, 0xa0, 0x8,
    0x20, 0x7a, 0xc2, 0x0, 0x80,

    /* U+00C2 "Â" */
    0x1, 0x40, 0x1, 0x0, 0x28, 0x0, 0xa0, 0x8,
    0x20, 0x7a, 0xc2, 0x0, 0x80,

    /* U+00C3 "Ã" */
    0x1, 0x0, 0x11, 0x0, 0x28, 0x0, 0xa0, 0x8,
    0x20, 0x7a, 0xc2, 0x0, 0x80,

    /* U+00C4 "Ä" */
    0x1, 0x40, 0x0, 0x0, 0x28, 0x0, 0xa0, 0x8,
    0x20, 0x7a, 0xc2, 0x0, 0x80,

    /* U+00C5 "Å" */
    0x1, 0x40, 0x5, 0x0, 0x10, 0x0, 0xa0, 0x2,
    0x80, 0x20, 0x81, 0xeb, 0x8, 0x2,

    /* U+00C6 "Æ" */
    0x0, 0xfa, 0x80, 0x98, 0x0, 0x53, 0xa4, 0x3a,
    0x80, 0x20, 0x3a, 0x80,

    /* U+00C7 "Ç" */
    0x1a, 0x86, 0x0, 0x80, 0x6, 0x0, 0x1a, 0x80,
    0x60,

    /* U+00C8 "È" */
    0x4, 0x1, 0x3, 0xa8, 0x80, 0x3a, 0x48, 0x3,
    0xa8,

    /* U+00C9 "É" */
    0x1, 0x1, 0x3, 0xa8, 0x80, 0x3a, 0x48, 0x3,
    0xa8,

    /* U+00CA "Ê" */
    0x4, 0x0, 0x43, 0xa8, 0x80, 0x3a, 0x48, 0x3,
    0xa8,

    /* U+00CB "Ë" */
    0x5, 0x0, 0x3, 0xa8, 0x80, 0x3a, 0x48, 0x3,
    0xa8,

    /* U+00CC "Ì" */
    0x10, 0x10, 0x82, 0x8, 0x20, 0x80,

    /* U+00CD "Í" */
    0x10, 0x42, 0x8, 0x20, 0x82, 0x0,

    /* U+00CE "Î" */
    0x4, 0x10, 0x8, 0x8, 0x8, 0x8, 0x8,

    /* U+00CF "Ï" */
    0x40, 0x2, 0x8, 0x20, 0x82, 0x0,

    /* U+00D0 "Ð" */
    0x3a, 0x80, 0x80, 0xcb, 0x82, 0x8, 0xc, 0x3a,
    0x80,

    /* U+00D1 "Ñ" */
    0x4, 0x40, 0x50, 0x30, 0x23, 0x82, 0x26, 0x22,
    0x1e, 0x20, 0x60,

    /* U+00D2 "Ò" */
    0x4, 0x0, 0x4, 0x1, 0xa8, 0x18, 0xc, 0x80,
    0x25, 0x80, 0xc1, 0xa8, 0x0,

    /* U+00D3 "Ó" */
    0x0, 0x0, 0x4, 0x1, 0xa8, 0x18, 0xc, 0x80,
    0x25, 0x80, 0xc1, 0xa8, 0x0,

    /* U+00D4 "Ô" */
    0x1, 0x0, 0x11, 0x1, 0xa8, 0x18, 0xc, 0x80,
    0x25, 0x80, 0xc1, 0xa8, 0x0,

    /* U+00D5 "Õ" */
    0x4, 0x40, 0x15, 0x1, 0xa8, 0x18, 0xc, 0x80,
    0x25, 0x80, 0xc1, 0xa8, 0x0,

    /* U+00D6 "Ö" */
    0x4, 0x40, 0x0, 0x1, 0xa8, 0x18, 0xc, 0x80,
    0x25, 0x80, 0xc1, 0xa8, 0x0,

    /* U+00D7 "×" */
    0x0, 0x25, 0x1c, 0x11,

    /* U+00D8 "Ø" */
    0x0, 0x20, 0x6b, 0x46, 0x17, 0x20, 0x49, 0x68,
    0x30, 0xfa, 0x0, 0x0, 0x0,

    /* U+00D9 "Ù" */
    0x4, 0x0, 0x0, 0x20, 0x62, 0x6, 0x20, 0x63,
    0x5, 0x1a, 0x80,

    /* U+00DA "Ú" */
    0x1, 0x0, 0x10, 0x20, 0x62, 0x6, 0x20, 0x63,
    0x5, 0x1a, 0x80,

    /* U+00DB "Û" */
    0x1, 0x0, 0x40, 0x20, 0x62, 0x6, 0x20, 0x63,
    0x5, 0x1a, 0x80,

    /* U+00DC "Ü" */
    0x5, 0x0, 0x0, 0x20, 0x62, 0x6, 0x20, 0x63,
    0x5, 0x1a, 0x80,

    /* U+00DD "Ý" */
    0x0, 0x40, 0x4, 0x2, 0x2, 0x2, 0x20, 0x6,
    0x80, 0xc, 0x0, 0x30, 0x0,

    /* U+00DE "Þ" */
    0x3a, 0x42, 0x8, 0x20, 0x83, 0xa4, 0x20, 0x0,

    /* U+00DF "ß" */
    0x2a, 0x2, 0x4, 0x61, 0x46, 0x74, 0x60, 0xc6,
    0x68,

    /* U+00E0 "à" */
    0x24, 0x0, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E1 "á" */
    0x9, 0x0, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E2 "â" */
    0x18, 0x0, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E3 "ã" */
    0x21, 0x5, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E4 "ä" */
    0x11, 0x0, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E5 "å" */
    0x4, 0x1, 0x2, 0xa0, 0xac, 0x93, 0xa, 0xc0,

    /* U+00E6 "æ" */
    0x2a, 0xa8, 0x2b, 0xa9, 0x93, 0x40, 0x29, 0xa8,

    /* U+00E7 "ç" */
    0x2a, 0x24, 0x9, 0x0, 0xa8, 0xc, 0x0,

    /* U+00E8 "è" */
    0x14, 0x0, 0x2, 0xa2, 0xa9, 0x90, 0xa, 0x80,

    /* U+00E9 "é" */
    0x5, 0x0, 0x2, 0xa2, 0xa9, 0x90, 0xa, 0x80,

    /* U+00EA "ê" */
    0x1d, 0x0, 0x2, 0xa2, 0xa9, 0x90, 0xa, 0x80,

    /* U+00EB "ë" */
    0x11, 0x0, 0x2, 0xa2, 0xa9, 0x90, 0xa, 0x80,

    /* U+00EC "ì" */
    0x20, 0x1, 0x86, 0x18, 0x60,

    /* U+00ED "í" */
    0x20, 0x6, 0x18, 0x61, 0x80,

    /* U+00EE "î" */
    0x28, 0x0, 0x18, 0x18, 0x18, 0x18,

    /* U+00EF "ï" */
    0x50, 0x6, 0x18, 0x61, 0x80,

    /* U+00F0 "ð" */
    0x2e, 0x45, 0x52, 0x99, 0x46, 0x81, 0x8a, 0x80,

    /* U+00F1 "ñ" */
    0x15, 0x4, 0x47, 0xa5, 0x82, 0x60, 0xd8, 0x30,

    /* U+00F2 "ò" */
    0x14, 0x0, 0x2, 0xa2, 0x42, 0x90, 0x8a, 0x80,

    /* U+00F3 "ó" */
    0x5, 0x0, 0x2, 0xa2, 0x42, 0x90, 0x8a, 0x80,

    /* U+00F4 "ô" */
    0x1d, 0x0, 0x2, 0xa2, 0x42, 0x90, 0x8a, 0x80,

    /* U+00F5 "õ" */
    0x15, 0x4, 0x42, 0xa2, 0x42, 0x90, 0x8a, 0x80,

    /* U+00F6 "ö" */
    0x11, 0x0, 0x2, 0xa2, 0x42, 0x90, 0x8a, 0x80,

    /* U+00F7 "÷" */
    0x8, 0x0, 0x6, 0xa0, 0x20,

    /* U+00F8 "ø" */
    0x0, 0xa, 0xc9, 0x5a, 0x52, 0x3a, 0x0, 0x0,

    /* U+00F9 "ù" */
    0x18, 0x0, 0x5, 0x9, 0x42, 0x60, 0x8a, 0xa0,

    /* U+00FA "ú" */
    0x6, 0x0, 0x5, 0x9, 0x42, 0x60, 0x8a, 0xa0,

    /* U+00FB "û" */
    0x9, 0x0, 0x5, 0x9, 0x42, 0x60, 0x8a, 0xa0,

    /* U+00FC "ü" */
    0x11, 0x0, 0x5, 0x9, 0x42, 0x60, 0x8a, 0xa0,

    /* U+00FD "ý" */
    0x2, 0x0, 0x0, 0x20, 0x80, 0x88, 0xa, 0x0,
    0x60, 0x28, 0x0,

    /* U+00FE "þ" */
    0x60, 0x6, 0x0, 0x7a, 0x46, 0xc, 0x60, 0xc7,
    0xa4, 0x60, 0x0,

    /* U+00FF "ÿ" */
    0x5, 0x0, 0x0, 0x20, 0x80, 0x88, 0xa, 0x0,
    0x60, 0x28, 0x0,

    /* U+2190 "←" */
    0x14, 0xe, 0x82, 0x0, 0x0,

    /* U+2191 "↑" */
    0x1c, 0x6, 0x40, 0x80, 0x10,

    /* U+2192 "→" */
    0x4, 0x1a, 0xc0, 0x10, 0x0,

    /* U+2193 "↓" */
    0x8, 0x2, 0x2, 0xe0, 0x10
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 34, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 34, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 50, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 6, .adv_w = 90, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 79, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 23, .adv_w = 108, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 88, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 27, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 43, .adv_w = 43, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 49, .adv_w = 43, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 53, .adv_w = 51, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 56, .adv_w = 74, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 61, .adv_w = 29, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 63, .adv_w = 49, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 64, .adv_w = 29, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 45, .box_w = 5, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 74, .adv_w = 85, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 47, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 86, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 79, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 82, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 79, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 29, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 29, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 147, .adv_w = 74, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 74, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 158, .adv_w = 74, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 184, .adv_w = 94, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 97, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 93, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 106, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 86, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 81, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 99, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 104, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 40, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 66, .box_w = 5, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 92, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 76, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 122, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 104, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 108, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 92, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 108, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 318, .adv_w = 93, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 79, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 75, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 101, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 91, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 144, .box_w = 9, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 86, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 83, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 84, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 43, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 400, .adv_w = 45, .box_w = 5, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 409, .adv_w = 43, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 413, .adv_w = 75, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 417, .adv_w = 64, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 418, .adv_w = 77, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 419, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 87, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 73, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 78, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 451, .adv_w = 45, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 88, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 464, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 36, .box_w = 3, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 481, .adv_w = 79, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 492, .adv_w = 135, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 87, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 81, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 87, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 518, .adv_w = 87, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 525, .adv_w = 52, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 529, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 533, .adv_w = 53, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 538, .adv_w = 87, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 72, .box_w = 6, .box_h = 4, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 115, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 71, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 562, .adv_w = 72, .box_w = 6, .box_h = 5, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 570, .adv_w = 67, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 574, .adv_w = 45, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 580, .adv_w = 38, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 584, .adv_w = 45, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 590, .adv_w = 74, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 593, .adv_w = 34, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 34, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 596, .adv_w = 73, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 604, .adv_w = 83, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 90, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 621, .adv_w = 90, .box_w = 7, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 630, .adv_w = 38, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 634, .adv_w = 64, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 640, .adv_w = 77, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 641, .adv_w = 103, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 52, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 655, .adv_w = 65, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 658, .adv_w = 74, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 49, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 663, .adv_w = 103, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 674, .adv_w = 77, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 675, .adv_w = 54, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 678, .adv_w = 74, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 685, .adv_w = 55, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 688, .adv_w = 55, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 691, .adv_w = 77, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 692, .adv_w = 87, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 699, .adv_w = 83, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 708, .adv_w = 34, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 709, .adv_w = 77, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 711, .adv_w = 55, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 714, .adv_w = 54, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 717, .adv_w = 65, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 720, .adv_w = 132, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 734, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 746, .adv_w = 132, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 760, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 767, .adv_w = 94, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 780, .adv_w = 94, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 793, .adv_w = 94, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 806, .adv_w = 94, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 819, .adv_w = 94, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 94, .box_w = 7, .box_h = 8, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 134, .box_w = 9, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 93, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 867, .adv_w = 86, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 876, .adv_w = 86, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 885, .adv_w = 86, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 86, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 903, .adv_w = 40, .box_w = 3, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 909, .adv_w = 40, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 40, .box_w = 4, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 922, .adv_w = 40, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 928, .adv_w = 107, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 937, .adv_w = 104, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 948, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 961, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 974, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 987, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1000, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1013, .adv_w = 74, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1017, .adv_w = 108, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1030, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1041, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1052, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1063, .adv_w = 101, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1074, .adv_w = 83, .box_w = 7, .box_h = 7, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1087, .adv_w = 92, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1095, .adv_w = 87, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1104, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1112, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1120, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1128, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1136, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1144, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1152, .adv_w = 127, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1160, .adv_w = 73, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1167, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1175, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1183, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1191, .adv_w = 78, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1199, .adv_w = 36, .box_w = 3, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1204, .adv_w = 36, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1209, .adv_w = 36, .box_w = 4, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 1215, .adv_w = 36, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1220, .adv_w = 82, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1228, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1236, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1244, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1252, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1260, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1268, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1276, .adv_w = 74, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1281, .adv_w = 81, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1289, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1297, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1305, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1313, .adv_w = 87, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1321, .adv_w = 72, .box_w = 6, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1332, .adv_w = 87, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1343, .adv_w = 72, .box_w = 6, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 1354, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1359, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1364, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1369, .adv_w = 77, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 0}
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
    },
    {
        .range_start = 8592, .range_length = 4, .glyph_id_start = 192,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 1, 2, 0, 3, 4, 5,
    2, 6, 7, 8, 9, 10, 9, 10,
    11, 12, 0, 13, 14, 15, 16, 17,
    18, 19, 12, 20, 20, 0, 0, 0,
    21, 22, 23, 24, 25, 22, 26, 27,
    28, 29, 29, 30, 31, 32, 29, 29,
    22, 33, 34, 35, 3, 36, 30, 37,
    37, 38, 39, 40, 41, 42, 43, 0,
    44, 0, 45, 46, 47, 48, 49, 50,
    51, 45, 52, 52, 53, 48, 45, 45,
    46, 46, 54, 55, 56, 57, 51, 58,
    58, 59, 58, 60, 41, 0, 0, 9,
    0, 61, 47, 62, 63, 64, 0, 65,
    0, 22, 8, 66, 0, 9, 22, 0,
    67, 0, 0, 0, 0, 51, 29, 9,
    0, 0, 8, 9, 0, 0, 0, 68,
    23, 23, 23, 23, 23, 23, 26, 25,
    26, 26, 26, 26, 29, 29, 29, 29,
    22, 29, 22, 22, 22, 22, 22, 0,
    22, 30, 30, 30, 30, 39, 69, 46,
    45, 45, 45, 45, 45, 45, 49, 47,
    49, 49, 49, 49, 51, 51, 70, 51,
    71, 45, 46, 46, 46, 46, 46, 9,
    46, 51, 51, 51, 51, 58, 46, 58,
    0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 1, 2, 0, 3, 4, 5,
    2, 6, 7, 8, 9, 10, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 12,
    18, 19, 20, 21, 21, 0, 0, 0,
    22, 23, 24, 25, 23, 25, 25, 25,
    23, 25, 25, 26, 25, 25, 25, 25,
    23, 25, 23, 25, 3, 27, 28, 29,
    29, 30, 31, 32, 33, 34, 35, 0,
    36, 0, 37, 38, 39, 39, 39, 0,
    39, 38, 40, 41, 38, 38, 42, 42,
    39, 42, 39, 42, 43, 44, 45, 46,
    46, 47, 46, 48, 0, 0, 35, 9,
    0, 49, 39, 50, 51, 52, 0, 53,
    0, 23, 8, 9, 9, 9, 23, 0,
    54, 0, 0, 0, 0, 42, 55, 9,
    0, 0, 8, 56, 0, 0, 0, 57,
    24, 24, 24, 24, 24, 24, 24, 23,
    25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 23, 23, 23, 23, 23, 0,
    23, 28, 28, 28, 28, 31, 25, 38,
    37, 37, 37, 37, 37, 37, 37, 39,
    39, 39, 39, 39, 58, 42, 59, 60,
    39, 42, 39, 39, 39, 39, 39, 9,
    39, 45, 45, 45, 45, 46, 38, 46,
    0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 6, 0, 3,
    -3, 0, 0, 0, 0, -7, -8, 1,
    6, 3, 2, -5, 1, 6, 0, 5,
    1, 4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 8, 1, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -3, 3,
    0, 3, 6, 0, -6, 0, 0, 0,
    0, -4, 0, 0, 0, 0, 0, -3,
    2, 3, 0, 0, -1, 0, -1, 1,
    0, -1, 0, -1, -1, -3, 0, 0,
    0, 0, -1, 0, 0, -2, -2, 0,
    0, -1, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, -1, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    -1, 0, 0, 0, 0, -3, 0, -15,
    0, 0, -3, 0, 3, 4, 0, 0,
    -3, 1, 1, 4, 3, -2, 3, 0,
    0, -7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, 0, 0, 0, 0, 0,
    -2, -6, 0, -5, -1, 0, 0, 0,
    0, 0, 5, 0, -4, -1, 0, 0,
    0, -2, 0, 0, -1, -9, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -10, -1, 5, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, -5, -6, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 4, 0,
    1, 0, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 5, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -5, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 3, 1, 4,
    -1, 0, 0, 3, -1, -4, -18, 1,
    3, 3, 0, -2, 0, 5, 0, 4,
    0, 4, 0, -12, 0, -2, 4, 0,
    4, -1, 3, 1, 0, 0, 0, -1,
    0, 0, -2, 10, 0, 10, 0, 4,
    0, 5, 2, 2, 0, 0, 1, 0,
    0, 4, 5, 2, -10, 10, 10, 10,
    0, 0, 0, -5, 0, 0, 0, 0,
    0, -1, 0, 1, -2, -2, -3, 1,
    0, -1, 0, 0, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -8, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, -3, 1, 0, -11, -1,
    0, 0, 0, 0, 0, -7, 0, -8,
    0, 0, 0, 0, -1, 0, 13, -2,
    -2, 1, 1, -1, 0, -2, 1, 0,
    0, -7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -12, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, -4,
    0, -8, -17, 0, 1, 0, 0, 0,
    0, 8, 0, 0, -5, 0, 4, 0,
    -9, -12, -9, -3, 4, 0, 0, -9,
    0, 2, -3, 0, -2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 4, -16, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -8, -1, 0, 5, -3, 6, 0, -6,
    -10, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 1, -2, -3, 0,
    0, 0, -1, 0, 0, -1, 0, 0,
    0, -3, 0, -1, 0, -3, -3, 0,
    -3, -4, -4, -2, 0, -3, 0, -3,
    0, 0, 0, 0, -1, 0, 0, 1,
    0, 1, -1, 0, 0, 0, 0, -2,
    0, 0, 0, 0, -4, 0, 0, 0,
    0, 0, 0, 1, -1, 0, 0, 0,
    -1, 1, 1, 0, 0, 0, 0, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, -1, 0, -2, 0, -2, 0,
    0, -1, 0, 4, 0, 0, -1, 0,
    0, 0, 0, 0, 0, 0, -1, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, -1, 0, -1, -2, 0, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    -1, -1, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, -1, -2, 0, 0, 0, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, -4, -1, -4, 3, 0, 0, -3,
    1, 3, 3, 0, -3, 0, -2, 0,
    0, -6, 1, -1, 1, -7, 1, 0,
    0, 0, -7, 0, -7, -1, -11, -1,
    0, -6, 0, 3, 4, 0, 2, 0,
    0, 0, 0, 0, 0, -2, -2, 0,
    0, 1, 1, -1, 3, -4, -2, 0,
    1, 0, 0, 0, 0, 0, 0, -1,
    0, 0, 0, -1, 0, 0, 0, 0,
    0, -1, -1, 0, -1, -2, 0, 0,
    0, 0, 0, 0, 0, -1, -1, 0,
    -1, -2, -1, 0, 0, -1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, -1, 0, 0, 0, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, -1, 0, -3, 1, 0, 0, -2,
    1, 1, 1, 0, 0, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 0, 1,
    0, 0, -1, 0, -1, -1, -2, 0,
    0, 0, 0, 0, 0, 0, 1, 0,
    -1, 0, 0, 0, 0, -1, -2, 0,
    0, 0, 1, 0, 3, -2, 0, 0,
    0, 0, 0, 0, 0, 4, -1, 0,
    -4, 0, 0, 3, -6, -7, -5, -3,
    1, 0, -1, -8, -2, 0, -2, 0,
    -3, 2, -2, -8, 0, -3, 0, 0,
    1, 0, 1, -1, 0, 1, 0, -4,
    -5, 0, -6, -3, -3, -3, -4, -2,
    -3, 0, -2, -3, -3, -2, -3, 2,
    -1, 1, 3, -2, -8, -3, -3, -3,
    0, 0, 0, -1, 0, 0, 0, 1,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, -1,
    0, 0, -1, 0, -2, -3, -3, 0,
    0, -4, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 1, -1, 0,
    0, 0, 0, -1, 1, 0, 0, 0,
    -3, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 6, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, -1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, -2, 0, 0, 0,
    0, -6, -4, 0, 0, 0, -2, -6,
    0, 0, -1, 1, 0, -3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -2, 0, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, -2, 0, 1, 0, 1,
    -14, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 2, 0, 1, -3, -3, 0,
    -1, -1, -2, 0, 0, 0, 0, 0,
    0, -4, 0, -1, 0, -2, -1, 0,
    -3, -3, -4, -1, 0, -3, 0, -4,
    0, 0, 0, 0, 10, 0, 0, 1,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 1, 0, 0, -3, 0, 0, 0,
    0, -6, 0, 0, 0, 0, 0, -12,
    -2, 4, 4, -1, -5, 0, 1, -2,
    0, -6, -1, -2, 1, -9, -1, 2,
    0, 2, -4, -2, -5, -4, -5, 0,
    0, -8, 0, 7, 0, 0, -1, 0,
    0, 0, -1, -1, -1, -3, -4, 0,
    1, 0, 0, 0, 0, -12, -10, 2,
    3, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    -1, -1, -2, 0, 0, -3, 0, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, 0, 0, 3,
    0, 2, 0, -3, 1, -1, 0, -3,
    -1, 0, -2, -1, -1, 0, -2, -2,
    0, 0, -1, 0, -1, -2, -2, 0,
    0, -1, 0, 1, -1, 0, -3, 0,
    0, 0, -3, 0, -2, 0, -2, -2,
    -1, 0, 0, 0, -1, 1, 1, 0,
    -2, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, 1, 0, -2,
    0, -1, -2, -4, -1, -1, -1, 0,
    -1, -2, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, 0, 0, 0, 0, 2,
    -1, 0, -1, 0, 0, 0, -1, -2,
    -1, -1, -2, -1, -1, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    1, 5, 0, 0, -3, 0, -1, 3,
    0, -1, -5, -2, 2, 0, 0, -6,
    -2, 1, -2, 1, 0, -1, -1, -4,
    0, -2, 1, 0, 0, -2, 0, 0,
    0, 1, 1, -3, -2, 0, -2, -1,
    -2, -1, -1, 0, -2, 1, -2, -2,
    -1, 0, 0, 0, -1, 4, 1, -2,
    -6, -1, 2, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -2, 0, 0, -2, 0,
    0, -1, -1, 0, 0, 0, 0, -1,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, -2, 0, -3, 0, 0, 0,
    -4, 0, 1, -3, 3, 0, -1, -6,
    0, 0, -3, -1, 0, -5, -3, -4,
    0, 0, -6, -1, -5, -5, -6, 0,
    -3, 0, 1, 9, -2, 0, -3, -1,
    0, -1, -2, -3, -2, -5, -5, -3,
    0, 0, 0, 0, 0, -1, -1, 0,
    0, -1, -1, -1, 0, 0, -1, 0,
    0, 0, 0, -9, -1, 4, 3, -3,
    -5, 0, 0, -4, 0, -6, -1, -1,
    3, -12, -2, 0, 0, 0, -8, -2,
    -7, -1, -9, 0, 0, -9, 0, 8,
    0, 0, -1, 0, 0, 0, 0, -1,
    -1, -5, -1, 0, 0, 0, 0, 0,
    0, -8, -10, 1, 0, 0, 0, 0,
    0, 0, 0, 0, -4, 0, -1, 0,
    0, -4, -6, 0, 0, -1, -2, -4,
    -1, 0, -1, 0, 0, 0, 0, -6,
    -1, -4, -4, -1, -2, -3, -1, -2,
    0, -3, -1, -4, -2, 0, -2, -2,
    -1, -2, 0, 1, 0, -1, -4, 0,
    0, 0, 0, 0, 0, 3, 0, 0,
    -7, -2, -2, -2, 0, -2, 0, 0,
    0, 0, 2, 0, 1, -3, 5, 0,
    -1, -1, -2, 0, 0, 0, 0, 0,
    0, -4, 0, -1, 0, -2, -1, 0,
    -3, -3, -4, -1, 0, -3, 1, 5,
    0, 0, 0, 0, 10, 0, 0, 1,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 1, 0, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, -3,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, -1, -1, 0, 0, -3, -1, 0,
    0, -3, 0, 2, -1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0,
    -1, 0, 0, 0, 3, 1, -1, 0,
    -4, -2, 0, 4, -4, -4, -3, -3,
    5, 2, 1, -11, -1, 3, -1, 0,
    -1, 1, -1, -4, 0, -1, 1, -2,
    -1, -4, -1, 0, 0, 4, 3, 0,
    -4, 0, -7, -2, 4, -2, -5, 0,
    -2, -4, -4, -1, -3, 0, 0, 0,
    0, 5, 4, -2, -7, 2, 4, 2,
    1, 0, -2, 0, -3, 0, 1, 4,
    -3, -5, -5, -3, 4, 0, 0, -9,
    -1, 1, -2, -1, -3, 0, -3, -5,
    -2, -2, -1, 0, 0, -3, -3, -1,
    0, 4, 3, -1, -7, 0, -7, -2,
    0, -4, -7, 0, -4, -2, -4, -4,
    -4, 0, 0, 0, 0, 3, 3, -2,
    -9, 2, 5, 2, 0, 0, -2, 0,
    -3, -1, 0, -1, -2, 0, 2, -4,
    1, 0, 0, -7, 0, -1, -3, -2,
    -1, -4, -3, -4, -3, 0, -4, -1,
    -3, -2, -4, -1, 0, 0, 0, 6,
    -2, 0, -4, -1, 0, -1, -3, -3,
    -3, -4, -5, -2, -1, 0, 0, 0,
    0, -3, 0, -1, 0, -1, -1, -1,
    3, 0, -2, 0, -6, -2, 1, 3,
    -4, -5, -3, -4, 4, -1, 1, -12,
    -2, 3, -3, -2, -5, 0, -4, -5,
    -2, -1, -1, -1, -3, -4, 0, 0,
    0, 4, 4, -1, -8, 0, -8, -3,
    3, -5, -9, -3, -4, -5, -6, -4,
    -5, 0, 0, 0, -3, 3, 3, -2,
    -10, 4, 4, 3, 0, 0, 0, 0,
    -2, 0, 0, 1, -2, 3, 1, -2,
    3, 0, 0, -4, 0, 0, 0, 0,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 1, 4,
    0, 0, -2, 0, 0, 0, 0, -1,
    -1, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 1, 0,
    -1, 0, 5, 0, 2, 0, 0, -2,
    0, 3, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 4, 0, 4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, 0, -1,
    2, 0, 4, 0, 0, 13, 2, -3,
    -3, 1, 1, -1, 0, -6, 0, 0,
    6, -8, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -9, 5, 18,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 5, 0, 0, 0,
    4, -8, -4, 1, 4, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, 0, -2, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, -3,
    0, 0, 0, 0, 0, 1, 17, -3,
    -1, 4, 3, -3, 1, 0, 0, 1,
    1, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -17, 4, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 2, -1, -1,
    3, 0, -9, 1, 0, 0, 0, 0,
    0, 0, 0, -4, 0, 0, 0, -3,
    0, 0, 0, 0, -3, -1, 0, 0,
    0, -3, 0, -2, 0, -6, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -9, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, 0, 0, 0, 0, -2, -3, 0,
    0, 0, 0, 0, 0, -2, 0, -3,
    0, 0, 0, -2, 1, -2, 0, 0,
    -3, -1, -3, 0, 0, -3, 0, -1,
    0, -6, 0, -1, 0, 0, -10, -2,
    -5, -1, -5, 0, 0, -9, 0, -3,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, -2, -2, -1, 0, 0, 0, 0,
    0, -2, -3, 0, -1, 0, 0, 0,
    0, 0, 0, 0, -3, 0, -3, 2,
    -1, 3, 0, -1, -3, -1, -2, -2,
    0, -2, -1, -1, 1, -3, 0, 0,
    0, 0, -11, -1, -2, 0, -3, 0,
    -1, -6, -1, 0, 0, -1, -1, 0,
    0, 0, 0, 1, 0, -1, -2, -1,
    0, 0, 0, 0, -1, 2, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, 0, -1, 0, 0, 0, -3,
    1, 0, 0, 0, -3, -1, -3, 0,
    0, -4, 0, -1, 0, -6, 0, 0,
    0, 0, -12, 0, -3, -5, -6, 0,
    0, -9, 0, -1, -2, 0, 0, 0,
    0, 0, 0, 0, 0, -1, -2, -1,
    0, 0, 0, 0, 0, -2, -3, 0,
    -1, 0, 0, 0, 0, 0, 0, 2,
    -2, 0, 4, 6, -1, -1, -4, 2,
    6, 2, 3, -3, 2, 5, 2, 4,
    3, 3, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 8, 6, -2,
    -1, 0, -1, 10, 6, 10, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0,
    0, 5, 5, 0, -2, 9, 6, 8,
    0, 0, -2, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    0, 0, 0, 0, 0, 2, 0, 0,
    0, 0, -11, -2, -1, -5, -6, 0,
    0, -9, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, -11, -2,
    -1, -5, -6, 0, 0, -5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, 0,
    -3, 1, 0, -1, 1, 2, 1, -4,
    0, 0, -1, 1, 0, 1, 0, 0,
    0, 0, -3, 0, -1, -1, -3, 0,
    -1, -5, 0, 8, -1, 0, -3, -1,
    0, -1, -2, 0, -1, -4, -3, -2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, -1, -1, 0, 0, -2, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 0, 0, 0, -11, -2,
    -1, -5, -6, 0, 0, -9, 0, 0,
    0, 0, 0, 0, 6, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, -2, 0, -4, -2, -1, 4,
    -1, -1, -5, 0, -1, 0, -1, -3,
    0, 3, 0, 1, 0, 1, -3, -5,
    -2, 0, -5, -2, -3, -5, -5, 0,
    -2, -3, -2, -2, -1, -1, -2, -1,
    0, -1, 0, 2, 0, 2, -1, 0,
    0, 0, 0, 0, 1, 4, 4, 0,
    -5, -1, -1, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, -1, -1, 0, 0, -3, 0, -1,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, -1, 0, 0, 0, 0, 0,
    0, -2, -3, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 0, -2,
    -1, 1, 0, -2, -2, -1, 0, -4,
    -1, -3, -1, -2, 0, -2, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -9, 0, 4, 0, 0, -2, 0,
    0, 0, 0, -2, 0, -1, 0, 0,
    0, 0, 0, 0, -1, -1, -3, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    -3, 0, 0, 5, -2, -4, -4, 1,
    1, 1, 0, -4, 1, 2, 1, 4,
    1, 4, -1, -3, 0, 0, -5, 0,
    0, -4, -3, 0, 0, -3, 0, -2,
    -2, 0, -2, 0, -2, 0, -1, 2,
    0, -1, -4, -1, 0, 0, 0, 0,
    0, 5, 4, 0, -4, 0, 0, 0,
    0, 0, -1, 0, -3, 0, 0, 2,
    -3, 0, 1, -1, 1, 0, 0, -4,
    0, -1, 0, 0, -1, 1, -1, 0,
    0, 0, -5, -2, -3, 0, -4, 0,
    0, -6, 0, 5, -1, 0, -2, 0,
    1, 0, -1, 0, -1, -4, 0, -1,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 1, -2, 0, 0, 0,
    -2, -1, 0, -2, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -8, 0, 3,
    0, 0, -1, 0, 0, 0, 0, 0,
    0, -1, -1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 5, 0, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -7, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, -1, 0,
    -5, 0, 0, 3, -4, 0, 2, -5,
    2, -1, -1, -4, -2, 1, -4, -3,
    -4, 0, -2, -6, 0, -3, 0, 0,
    0, -1, 1, 0, 0, 2, 0, 2,
    -5, 0, -4, -3, -3, -3, -5, -3,
    -3, -3, -3, -5, -3, 0, 0, 0,
    -1, 0, 2, -2, -3, -3, 4, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, -1, 0, 0, 0, -1, 0, 0,
    0, -2, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -1, 0, 0, 0, 1, 3, 0, 0,
    0, 0, 2, 0, -3, -4, -4, -2,
    4, 0, 1, -1, 0, 3, -1, 0,
    -5, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 6, 2, -1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -3, 0, 0, 0,
    0, 0, 0, 0, -5, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 3, 0, 0, 0, 0, 2,
    0, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -1, 0, 1, 0,
    0, 0, 0, 1, 1, 0, 0, 0,
    0, -1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -7, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0,
    0, 3, 0, 3, 0, 0, 0, 0,
    0, -8, -7, 0, 6, 4, 2, -5,
    1, 5, 0, 5, 0, 3, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 7, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -8, 0, 0, 0, 0, -4, 0, -8,
    -2, 0, 0, 0, -5, 0, 5, -6,
    -6, 0, 0, -6, 0, -6, -6, 0,
    0, -17, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -12, 1, 6,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, -2, -6,
    0, -7, -11, -1, -3, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -3, -4, 0, -3, -3, -3, 0,
    -1, 0, 0, 0, 0, -4, 0, -4,
    0, -5, -3, 0, -1, -4, -4, -2,
    0, -5, 0, -4, -1, 0, 0, 0,
    -1, 0, 0, 1, 0, 0, -1, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -3, 0, 0, 0, 0, 0, -2, 0,
    0, 0, 0, 10, 0, 0, 0, 0,
    -1, 0, 0, 0, 0, 0, 0, 0,
    0, 5, 0, 0, 0, 0, -11, -2,
    -1, -5, -6, 0, 0, -9, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1,
    0, -2, 0, 1, 0, 0, -1, 0,
    0, 0, 0, 0, 0, -1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -1, 0, -3, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 3, 0, 0,
    -1, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 71,
    .right_class_cnt     = 60,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static const lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 3,
    .bpp = 2,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t tw_sans_8 = {
#else
const lv_font_t tw_sans_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if TW_SANS_8*/

