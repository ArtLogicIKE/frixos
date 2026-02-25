#ifndef DISPLAY_H
#define DISPLAY_H    // Guard to prevent multiple inclusion  

#include "frixos.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"


// public variables
extern char screen_ofsx;
extern char screen_ofsy;
extern char msg_scrolling[SCROLL_MSG_LENGTH];


esp_err_t startup_lcd(void);
esp_err_t startup_lvgl(void);
void startup_display(void);
void display_changed(void);
void show_qr_code(void);
void display_digit(int position, int digit);
void display_task(void *pvParameters);
void replace_placeholders(const char *input, char* output, size_t output_size);
void set_scroll_message(const char *message);
void prepare_tokens(void);
const lv_font_t* get_selected_font(uint8_t font_index);
uint8_t get_selected_font_height(uint8_t font_index);

#endif // DISPLAY_H