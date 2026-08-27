#ifndef F_SCREEN_LAYOUT_BIN_H
#define F_SCREEN_LAYOUT_BIN_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "frixos.h"

/* Little-endian magic: "FSXL" */
#define FRIXOS_SCREEN_BIN_MAGIC 0x4653584Cu
#define FRIXOS_SCREEN_BIN_FORMAT 1

#define FRIXOS_SCREEN_BIN_FONT_LEN 12

typedef struct __attribute__((packed))
{
    uint32_t magic;
    uint8_t format;
    uint8_t layout_version;
    uint8_t scroll_delay;
    uint8_t day_color_filter;
    uint8_t night_color_filter;
    uint8_t reserved[3];
    char day_font[FRIXOS_SCREEN_BIN_FONT_LEN];
    char night_font[FRIXOS_SCREEN_BIN_FONT_LEN];
    char day_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN];
    char night_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN];
    uint16_t w;
    uint16_t h;
} screen_layout_bin_header_t;

typedef struct __attribute__((packed))
{
    screen_layout_bin_header_t header;
    screen_layout_profile_t profile[FRIXOS_SCREEN_LAYOUT_PROFILES];
} screen_layout_wire_t;

#define FRIXOS_SCREEN_LAYOUT_WIRE_SIZE ((size_t)sizeof(screen_layout_wire_t))

#define SCREEN_LAYOUT_FILE_PATH "/spiffs/current.layout.bin"

bool screen_layout_wire_encode(uint8_t *out, size_t out_size, size_t *written);
esp_err_t screen_layout_wire_decode(const uint8_t *data, size_t len, screen_layout_t *layout,
                                    char day_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char night_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char day_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    char night_aux_font[FRIXOS_SCREEN_BIN_FONT_LEN],
                                    uint8_t *day_color_filter, uint8_t *night_color_filter);

/* Persist the live layout as a wire-format file on LittleFS. Load always
 * leaves a valid eeprom_screen_layout (file, leftover NVS blob, or factory). */
void screen_layout_file_load(void);
esp_err_t screen_layout_file_save(void);
void screen_layout_file_remove(void);

#endif /* F_SCREEN_LAYOUT_BIN_H */
