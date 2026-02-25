/*******************************************************************************
 * Size: 9 px
 * Bpp: 1
 * Opts: --bpp 1 --size 9 --no-compress --stride 1 --align 1 --font F9-DePixelHalbfett.ttf --range 0-255 --format lvgl -o frixos_9.c
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



#ifndef FRIXOS_9
#define FRIXOS_9 1
#endif

#if FRIXOS_9

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0000 "\u0000" */
    0x0,

    /* U+000D "\r" */
    0x0,

    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3, 0xc0,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x3b, 0x38, 0xc6, 0x31, 0x8c, 0x60,

    /* U+0024 "$" */
    0x18, 0x7e, 0xd9, 0xf8, 0x7e, 0x1f, 0x1b, 0x9b,
    0x7e, 0x18,

    /* U+0025 "%" */
    0x41, 0xa2, 0x44, 0x8, 0x10, 0x22, 0x45, 0x82,

    /* U+0026 "&" */
    0x78, 0xcc, 0xcd, 0x61, 0xce, 0xcc, 0xcc, 0xcc,
    0x78,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x6a, 0xa9,

    /* U+0029 ")" */
    0x95, 0x56,

    /* U+002A "*" */
    0xaa, 0x80,

    /* U+002B "+" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x44, 0x22, 0x11, 0x8, 0x84, 0x0,

    /* U+0030 "0" */
    0x7b, 0x3c, 0xf7, 0xff, 0xbc, 0xf3, 0x78,

    /* U+0031 "1" */
    0x3f, 0x33, 0x33, 0x33, 0x30,

    /* U+0032 "2" */
    0x7a, 0x30, 0xc3, 0x18, 0xc6, 0x30, 0xfc,

    /* U+0033 "3" */
    0xfc, 0x63, 0x1e, 0xc, 0x30, 0xe3, 0x78,

    /* U+0034 "4" */
    0xc, 0x38, 0xb2, 0x68, 0xdf, 0xc3, 0x6, 0xc,

    /* U+0035 "5" */
    0xff, 0xc, 0x3e, 0xc, 0x30, 0xe3, 0x78,

    /* U+0036 "6" */
    0x39, 0x8c, 0x3e, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0037 "7" */
    0xfc, 0x30, 0xc3, 0x18, 0xc3, 0xc, 0x30,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xde, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xf0, 0xc6, 0x70,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0xf0, 0x3d, 0x80,

    /* U+003C "<" */
    0x12, 0x4c, 0x42, 0x10,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0x84, 0x23, 0x24, 0x80,

    /* U+003F "?" */
    0x74, 0xc6, 0x77, 0x30, 0xc, 0x60,

    /* U+0040 "@" */
    0x3e, 0x20, 0xa7, 0x34, 0x9a, 0x4c, 0xd9, 0x0,
    0x7c,

    /* U+0041 "A" */
    0x7b, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x3c, 0xf3, 0xf8,

    /* U+0043 "C" */
    0x7b, 0x1c, 0x30, 0xc3, 0xc, 0x31, 0x78,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0045 "E" */
    0xfe, 0x31, 0x8f, 0x63, 0x18, 0xf8,

    /* U+0046 "F" */
    0xfe, 0x31, 0x8f, 0x63, 0x18, 0xc0,

    /* U+0047 "G" */
    0x7b, 0x1c, 0x30, 0xdf, 0x3c, 0xf3, 0x78,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0x66, 0xf0,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xcf, 0x37,
    0x80,

    /* U+004B "K" */
    0xc7, 0x9b, 0x67, 0x8e, 0x1e, 0x36, 0x66, 0xc6,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x18, 0xf8,

    /* U+004D "M" */
    0x80, 0x70, 0x3e, 0x1f, 0xcf, 0xbe, 0xe7, 0x38,
    0x8e, 0x3, 0x80, 0xc0,

    /* U+004E "N" */
    0x83, 0x87, 0x8f, 0x9b, 0xb3, 0xe3, 0xc3, 0x82,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30, 0xc0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78, 0x60,
    0xc0,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x3c, 0xf3, 0xcc,

    /* U+0053 "S" */
    0x76, 0x71, 0xc7, 0x1c, 0x73, 0x70,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xde, 0x30,

    /* U+0057 "W" */
    0xcc, 0xf3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x34,
    0xc8, 0xcc,

    /* U+0058 "X" */
    0xcf, 0x3c, 0xde, 0x31, 0xec, 0xf3, 0xcc,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0x78, 0xc3, 0xc, 0x30,

    /* U+005A "Z" */
    0xfc, 0x30, 0xc6, 0x31, 0x8c, 0x30, 0xfc,

    /* U+005B "[" */
    0xea, 0xaa, 0xac,

    /* U+005C "\\" */
    0x84, 0x10, 0x82, 0x10, 0x42, 0x8, 0x40,

    /* U+005D "]" */
    0xd5, 0x55, 0x5c,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x88, 0x80,

    /* U+0061 "a" */
    0x7a, 0x37, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0063 "c" */
    0x76, 0x71, 0x8c, 0x65, 0xc0,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0065 "e" */
    0x7b, 0x3c, 0xff, 0xc3, 0x17, 0x80,

    /* U+0066 "f" */
    0x3b, 0x3c, 0xc6, 0x31, 0x8c, 0x60,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0x8d, 0xe0,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xcc,

    /* U+0069 "i" */
    0xcf, 0xff, 0xc0,

    /* U+006A "j" */
    0x18, 0x6, 0x31, 0x8c, 0x63, 0x1c, 0xdc,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xf3, 0x8f, 0x36, 0xcc,

    /* U+006C "l" */
    0xff, 0xff, 0xc0,

    /* U+006D "m" */
    0xff, 0xb3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcc,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3f, 0xb0, 0xc0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xc,

    /* U+0072 "r" */
    0xdf, 0x31, 0x8c, 0x63, 0x0,

    /* U+0073 "s" */
    0x76, 0x78, 0xe3, 0xcd, 0xc0,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x66, 0x30,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xe3, 0x0,

    /* U+0077 "w" */
    0xcc, 0xf3, 0x3c, 0xcf, 0x33, 0xcc, 0xd3, 0x23,
    0x30,

    /* U+0078 "x" */
    0xcf, 0x37, 0x8c, 0x7b, 0x3c, 0xc0,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0x8d, 0xe0,

    /* U+007A "z" */
    0xfc, 0x31, 0x8c, 0x63, 0xf, 0xc0,

    /* U+007B "{" */
    0x29, 0x49, 0x22, 0x44,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0x89, 0x12, 0x4a, 0x50,

    /* U+007E "~" */
    0x66, 0x60,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0xf3, 0xff, 0xc0,

    /* U+00A2 "¢" */
    0x88,

    /* U+00A4 "¤" */
    0x3c, 0xc5, 0x87, 0xe6, 0x1f, 0x18, 0x31, 0x3c,

    /* U+00A5 "¥" */
    0x0,

    /* U+00A7 "§" */
    0x0,

    /* U+00A8 "¨" */
    0xcc,

    /* U+00A9 "©" */
    0x64, 0xe9, 0x49, 0xb0,

    /* U+00AB "«" */
    0x19, 0x99, 0x99, 0x99, 0x86, 0x61, 0x98, 0x66,

    /* U+00AE "®" */
    0x75, 0x6d, 0x58, 0xb8,

    /* U+00B0 "°" */
    0x55, 0x0,

    /* U+00B4 "´" */
    0x2a, 0x0,

    /* U+00B5 "µ" */
    0xff, 0xff, 0xb3, 0x33, 0x3c, 0xcc, 0xcf, 0x33,
    0x33, 0xcc, 0xcc, 0xf3, 0x33, 0x3c, 0xcc, 0xcc,

    /* U+00B6 "¶" */
    0xff, 0xf6, 0xdb,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x60,

    /* U+00BB "»" */
    0xcc, 0x33, 0xc, 0xc3, 0x33, 0x33, 0x33, 0x30,

    /* U+00BF "¿" */
    0x63, 0x0, 0xc7, 0x1c, 0x73, 0x70,

    /* U+00C0 "À" */
    0x20, 0x40, 0x9e, 0xcf, 0x3c, 0xff, 0xcf, 0x3c,
    0xf3,

    /* U+00C1 "Á" */
    0x10, 0x84, 0x1e, 0xcf, 0x3c, 0xff, 0xcf, 0x3c,
    0xf3,

    /* U+00C2 "Â" */
    0x31, 0x27, 0xb3, 0xcf, 0x3f, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+00C3 "Ã" */
    0x66, 0x67, 0xb3, 0xcf, 0x3f, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+00C4 "Ä" */
    0xcc, 0x7, 0xb3, 0xcf, 0x3f, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+00C5 "Å" */
    0x21, 0x42, 0x1e, 0xcf, 0x3c, 0xff, 0xcf, 0x3c,
    0xf3,

    /* U+00C6 "Æ" */
    0x7f, 0xe6, 0x33, 0x19, 0x8f, 0xf6, 0x63, 0x31,
    0x98, 0xcf, 0x80,

    /* U+00C7 "Ç" */
    0x7b, 0x1c, 0x30, 0xc3, 0xc, 0x31, 0x78, 0x42,
    0x0,

    /* U+00C8 "È" */
    0x41, 0x5, 0xfc, 0x63, 0x1e, 0xc6, 0x31, 0xf0,

    /* U+00C9 "É" */
    0x11, 0x11, 0xfc, 0x63, 0x1e, 0xc6, 0x31, 0xf0,

    /* U+00CA "Ê" */
    0x32, 0x41, 0xfc, 0x63, 0x1e, 0xc6, 0x31, 0xf0,

    /* U+00CB "Ë" */
    0xd8, 0x3f, 0x8c, 0x63, 0xd8, 0xc6, 0x3e,

    /* U+00CC "Ì" */
    0x42, 0x1f, 0x66, 0x66, 0x66, 0x6f,

    /* U+00CD "Í" */
    0x12, 0x4f, 0x66, 0x66, 0x66, 0x6f,

    /* U+00CE "Î" */
    0x69, 0xf, 0x66, 0x66, 0x66, 0x6f,

    /* U+00CF "Ï" */
    0xd8, 0x3c, 0xc6, 0x31, 0x8c, 0x63, 0x3c,

    /* U+00D0 "Ð" */
    0x7c, 0xcd, 0x9b, 0x3f, 0x6c, 0xd9, 0xb3, 0x7c,

    /* U+00D1 "Ñ" */
    0x32, 0x9a, 0xe, 0x1e, 0x3e, 0x6e, 0xcf, 0x8f,
    0xe, 0x8,

    /* U+00D2 "Ò" */
    0x20, 0x40, 0x9e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D3 "Ó" */
    0x10, 0x84, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D4 "Ô" */
    0x31, 0x20, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D5 "Õ" */
    0x66, 0x67, 0xb3, 0xcf, 0x3c, 0xf3, 0xcf, 0x37,
    0x80,

    /* U+00D6 "Ö" */
    0xcc, 0x7, 0xb3, 0xcf, 0x3c, 0xf3, 0xcf, 0x37,
    0x80,

    /* U+00D8 "Ø" */
    0x2, 0xfb, 0x36, 0x6d, 0xdb, 0xbb, 0x76, 0xcc,
    0xf2, 0x0,

    /* U+00D9 "Ù" */
    0x20, 0x40, 0xb3, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DA "Ú" */
    0x8, 0x42, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DB "Û" */
    0x31, 0x20, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DC "Ü" */
    0xcc, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0x37,
    0x80,

    /* U+00DD "Ý" */
    0x8, 0x42, 0x33, 0xcf, 0x3c, 0xde, 0x30, 0xc3,
    0xc,

    /* U+00DE "Þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3f, 0xb0, 0xc0,

    /* U+00DF "ß" */
    0x79, 0x8b, 0x16, 0xcc, 0xd8, 0xf1, 0xe3, 0xdc,

    /* U+00E0 "à" */
    0x20, 0x40, 0x9e, 0x8d, 0xfc, 0xf3, 0xcd, 0xf0,

    /* U+00E1 "á" */
    0x10, 0x84, 0x1e, 0x8d, 0xfc, 0xf3, 0xcd, 0xf0,

    /* U+00E2 "â" */
    0x31, 0x20, 0x1e, 0x8d, 0xfc, 0xf3, 0xcd, 0xf0,

    /* U+00E3 "ã" */
    0x66, 0x67, 0xa3, 0x7f, 0x3c, 0xf3, 0x7c,

    /* U+00E4 "ä" */
    0xcc, 0x7, 0xa3, 0x7f, 0x3c, 0xf3, 0x7c,

    /* U+00E5 "å" */
    0x10, 0xa1, 0x1e, 0x8d, 0xfc, 0xf3, 0xcd, 0xf0,

    /* U+00E6 "æ" */
    0x7f, 0xa3, 0x37, 0xcf, 0x3f, 0xcc, 0x33, 0x17,
    0xf8,

    /* U+00E7 "ç" */
    0x76, 0x71, 0x8c, 0x65, 0xc2, 0x20,

    /* U+00E8 "è" */
    0x20, 0x40, 0x9e, 0xcf, 0x3f, 0xf0, 0xc5, 0xe0,

    /* U+00E9 "é" */
    0x8, 0x42, 0x1e, 0xcf, 0x3f, 0xf0, 0xc5, 0xe0,

    /* U+00EA "ê" */
    0x31, 0x20, 0x1e, 0xcf, 0x3f, 0xf0, 0xc5, 0xe0,

    /* U+00EB "ë" */
    0xcc, 0x7, 0xb3, 0xcf, 0xfc, 0x31, 0x78,

    /* U+00EC "ì" */
    0x88, 0xed, 0xb6, 0xd8,

    /* U+00ED "í" */
    0x2a, 0x36, 0xdb, 0x6c,

    /* U+00EE "î" */
    0x69, 0x6, 0x66, 0x66, 0x66,

    /* U+00EF "ï" */
    0xcc, 0x3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+00F0 "ð" */
    0x70, 0x60, 0xdf, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F1 "ñ" */
    0x66, 0x6f, 0xb3, 0xcf, 0x3c, 0xf3, 0xcc,

    /* U+00F2 "ò" */
    0x20, 0x40, 0x9e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F3 "ó" */
    0x10, 0x84, 0x1e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F4 "ô" */
    0x31, 0x20, 0x1e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F5 "õ" */
    0x66, 0x67, 0xb3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F6 "ö" */
    0xcc, 0x7, 0xb3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+00F8 "ø" */
    0x5, 0xec, 0xf7, 0xdf, 0xbe, 0xde, 0x40,

    /* U+00F9 "ù" */
    0x20, 0x40, 0xb3, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FA "ú" */
    0x8, 0x42, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FB "û" */
    0x31, 0x20, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FC "ü" */
    0xcc, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+00FD "ý" */
    0x8, 0x42, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,
    0xe3, 0x78,

    /* U+00FE "þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xfb, 0xc,
    0x0,

    /* U+00FF "ÿ" */
    0xcc, 0xc, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c, 0x38,
    0xde
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 49, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 49, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 49, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 6, .adv_w = 65, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 7, .adv_w = 98, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 147, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 23, .adv_w = 147, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 147, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 33, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 41, .adv_w = 49, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 49, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 65, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 47, .adv_w = 131, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 49, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 55, .adv_w = 82, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 56, .adv_w = 49, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 98, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 64, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 98, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 105, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 49, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 49, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 138, .adv_w = 82, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 114, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 145, .adv_w = 82, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 114, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 180, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 114, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 114, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 98, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 114, .box_w = 6, .box_h = 11, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 232, .adv_w = 147, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 98, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 196, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 147, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 289, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 114, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 98, .box_w = 6, .box_h = 9, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 196, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 333, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 347, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 49, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 357, .adv_w = 98, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 364, .adv_w = 49, .box_w = 2, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 367, .adv_w = 98, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 369, .adv_w = 65, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 370, .adv_w = 0, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 372, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 98, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 98, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 417, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 424, .adv_w = 65, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 98, .box_w = 5, .box_h = 11, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 434, .adv_w = 114, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 65, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 196, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 472, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 479, .adv_w = 98, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 114, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 98, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 196, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 515, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 529, .adv_w = 131, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 535, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 539, .adv_w = 33, .box_w = 1, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 541, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 545, .adv_w = 98, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 547, .adv_w = 68, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 49, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 147, .box_w = 1, .box_h = 5, .ofs_x = 7, .ofs_y = 1},
    {.bitmap_index = 552, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 560, .adv_w = 39, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 63, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 562, .adv_w = 0, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 563, .adv_w = 98, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 567, .adv_w = 163, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 98, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 579, .adv_w = 65, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 581, .adv_w = 0, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 583, .adv_w = 327, .box_w = 18, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 599, .adv_w = 90, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 602, .adv_w = 49, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 603, .adv_w = 0, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 604, .adv_w = 163, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 114, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 618, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 627, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 654, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 180, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 683, .adv_w = 114, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 692, .adv_w = 114, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 700, .adv_w = 114, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 114, .box_w = 5, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 114, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 723, .adv_w = 98, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 729, .adv_w = 98, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 735, .adv_w = 98, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 98, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 748, .adv_w = 131, .box_w = 7, .box_h = 9, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 756, .adv_w = 147, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 766, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 775, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 793, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 802, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 811, .adv_w = 131, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 821, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 830, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 839, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 848, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 857, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 866, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 873, .adv_w = 147, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 881, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 897, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 912, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 927, .adv_w = 196, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 936, .adv_w = 98, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 942, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 950, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 958, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 966, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 973, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 977, .adv_w = 65, .box_w = 3, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 981, .adv_w = 65, .box_w = 4, .box_h = 10, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 986, .adv_w = 65, .box_w = 6, .box_h = 9, .ofs_x = -2, .ofs_y = 0},
    {.bitmap_index = 993, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1000, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1007, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1015, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1023, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1031, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1038, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1045, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1052, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1060, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1068, .adv_w = 131, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1076, .adv_w = 131, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1083, .adv_w = 131, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1093, .adv_w = 131, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1102, .adv_w = 131, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0xd
};

static const uint8_t glyph_id_ofs_list_2[] = {
    0, 1, 2, 0, 3, 4, 0, 5,
    6, 7, 0, 8, 0, 0, 9, 0,
    10, 0, 0, 0, 11, 12, 13, 14,
    15, 0, 0, 16
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 0, .range_length = 14, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 2, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 3,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 28, .glyph_id_start = 98,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_2, .list_length = 28, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 191, .range_length = 24, .glyph_id_start = 115,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 216, .range_length = 31, .glyph_id_start = 139,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 248, .range_length = 8, .glyph_id_start = 170,
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
    .cmap_num = 6,
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
const lv_font_t frixos_9 = {
#else
lv_font_t frixos_9 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
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



#endif /*#if FRIXOS_9*/
