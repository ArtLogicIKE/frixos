//
// display-related functions
//
// This module handles:
// - Display initialization and management
// - Time display with digit sprites
// - Weather and moon phase display
// - Message scrolling and centering (messages shorter than 88 pixels are centered, optimized to only recalculate when needed)
// - Display brightness control based on ambient light
// - Integration status display (Dexcom glucose, etc.)

#include "frixos.h"
#include "f-display.h"
#include "f-pwm.h"
#include "f-time.h"
#include "f-wifi.h"
#include "f-settings.h"
#include "f-integrations.h"
#include "f-dexcom.h"

#include "time.h"
#include "math.h"
#include <string.h>
#include "esp_timer.h"
#include "esp_lcd_st7735.h"
#include <unistd.h>
#include "lvgl.h"
#include "draw/lv_image_decoder.h"
#include "draw/lv_draw_buf.h"

static const char *TAG = "f-display";
char days[][10] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char months[][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// Language-specific translations
// Language index: 0=en, 1=de, 2=fr, 3=it, 4=pt, 5=sv, 6=da, 7=pl, 8=es
static const char *greetings[9][4] = {
    // English
    {"Good Morning", "Good Afternoon", "Good Evening", "Good Night"},
    // German
    {"Guten Morgen", "Guten Nachmittag", "Guten Abend", "Gute Nacht"},
    // French
    {"Bonjour", "Bon Après-midi", "Bonsoir", "Bonne Nuit"},
    // Italian
    {"Buongiorno", "Buon Pomeriggio", "Buonasera", "Buona Notte"},
    // Portuguese
    {"Bom Dia", "Boa Tarde", "Boa Noite", "Boa Noite"},
    // Swedish
    {"God Morgon", "God Eftermiddag", "God Kväll", "God Natt"},
    // Danish
    {"God Morgen", "God Eftermiddag", "God Aften", "God Nat"},
    // Polish
    {"Dzień Dobr", "Dzień Dobr", "Dobry Wieczór", "Dobranoc"},
    // Spanish
    {"Buenos Días", "Buenas Tardes", "Buenas Noches", "Buenas Noches"}};

static const char *day_names[9][7] = {
    // English
    {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},
    // German
    {"Son", "Mon", "Die", "Mit", "Don", "Fre", "Sam"},
    // French
    {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"},
    // Italian
    {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"},
    // Portuguese
    {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"},
    // Swedish
    {"Sön", "Mån", "Tis", "Ons", "Tor", "Fre", "Lör"},
    // Danish
    {"Søn", "Man", "Tir", "Ons", "Tor", "Fre", "Lør"},
    // Polish
    {"Nie", "Pon", "Wt", "Śr", "Czw", "Pt", "Sob"},
    // Spanish
    {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"}};

static const char *month_names[9][12] = {
    // English
    {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"},
    // German
    {"Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"},
    // French
    {"Jan", "Fév", "Mar", "Avr", "Mai", "Jun", "Jul", "Aoû", "Sep", "Oct", "Nov", "Déc"},
    // Italian
    {"Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic"},
    // Portuguese
    {"Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez"},
    // Swedish
    {"Jan", "Feb", "Mar", "Apr", "Maj", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec"},
    // Danish
    {"Jan", "Feb", "Mar", "Apr", "Maj", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec"},
    // Polish
    {"Sty", "Lut", "Mar", "Kwi", "Maj", "Cze", "Lip", "Sie", "Wrz", "Paź", "Lis", "Gru"},
    // Spanish
    {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"}};
char msg_scrolling[SCROLL_MSG_LENGTH]; // scrolling message text
double lux = 0;
// Include the generated sprite sheet image
lv_obj_t *img_digits_sprite = NULL,
         *img_weather = NULL,
         *img_moon = NULL,
         *img_ampm = NULL,
         *img_logo = NULL,
         *img_glucose = NULL,
         *img_glucose_trend = NULL,
         *img_wifi = NULL,
         *img_mgdl = NULL; // all the images on screen - ok, digits is off-screen

lv_obj_t *label_msg = NULL;

// Define digit width & height (adjust based on actual sprite sheet)
#define DIGIT_WIDTH 18          // Width of each digit in the sprite sheet
#define DIGIT_HEIGHT 36         // Height of each digit
#define SPRITE_SHEET_COLUMNS 10 // Number of digits in the sprite sheet

#define NUM_DIGITS 4                 // the 4 time digits
#define label_msg_ofs_y (25 + 25 + 14) // y offset for the message label
#define MSG_WIDTH 105                // width of the message area
#define MSG_EXTRA_WIDTH 7            // extra width for the message area, useful for scolling but bad for centering
#define FADE_STEPS 14
#define FADE_INTERVAL 200       // Time between steps in ms
#define MAX_TOKEN_COUNT 100     // Maximum number of tokens to prevent memory issues
#define MAX_FONT_INDEX 2        // Maximum font index (0-2)
#define MIN_ANIMATION_TIME 1000 // Minimum animation time in milliseconds

int label_scroll_pos = 0, label_max_pos = MSG_WIDTH;
lv_obj_t *digit_objs[NUM_DIGITS] = {NULL, NULL, NULL, NULL}; // LVGL objects for digit images
lv_obj_t *dots[2] = {NULL, NULL};                            // Colon dots between HH and MM

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;

int last_minute = -1;      // Store the last updated minute
time_t ota_start_time = 0; // Track when OTA update started
time_t now;
struct tm timeinfo;

// Alternate time/CGM display state
bool alternate_display_active = false; // True if alternating display is enabled
bool showing_glucose = false;          // True if currently showing glucose, false if showing time
time_t alternate_display_start = 0;    // When the current display mode started

// fader stuff
static int fade_step = 0;
static bool fading_in = true;                // Flag to track direction
static esp_timer_handle_t fade_timer = NULL; // ESP timer handle
static bool fade_update_needed = false;      // Flag to indicate fade update is needed
static int label_size = 0;

LV_FONT_DECLARE(frixos_8);
LV_FONT_DECLARE(frixos_9);
LV_FONT_DECLARE(frixos_11);

static void fade_timer_cb(void *arg);
void create_grid(lv_obj_t *scr);

// Helper functions for modular display_task
static void handle_als_and_brightness(uint32_t loop_counter);
static void handle_wifi_status_icon(void);
static void handle_integration_and_messages(void);
static void handle_alternate_mode_switching(time_t now, uint32_t loop_counter, bool *should_update_display);
static void update_display_content(time_t now);
void update_weather_msg(void);
void display_string_substring(const char *text, int32_t x, int32_t y,
                              int32_t start_pixel, int32_t width_pixels,
                              lv_obj_t *label_obj, const lv_font_t *font);
void init_char_width_cache(const lv_font_t *font);
void invalidate_char_width_cache(void);
static bool is_glucose_on();
static bool is_glucose_fresh();

// Gamma curve definitions for ST7735S
#define ST7735_GMCTRP1 0xE0 // Positive gamma correction
#define ST7735_GMCTRN1 0xE1 // Negative gamma correction

#define ST77XX_MADCTL 0x36
#define ST77XX_MADCTL_RGB 0x00
#define ST7735_MADCTL_MH 0x04
#define ST7735_MADCTL_BGR 0x08
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MY 0x80

// values for rotating display
// 8 values, the first for are the 'regular' ones, the next 4 are the mirrored ones (flipper LCD)
// horizontal/vertical are relative here, our display is mounted sideways, so our horizontal is really vertical...
uint8_t madctl_value[8] =
    {
        ST7735_MADCTL_BGR,                                                         // vertical bottom-to-top
        ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR,                   // horizontal upside-down
        ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR,                   // vertical top-to-bottom
        ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST7735_MADCTL_BGR,                   // horizontal
        ST77XX_MADCTL_MY | ST7735_MADCTL_BGR,                                      // vertical bottom-to-top
        ST77XX_MADCTL_MV | ST7735_MADCTL_BGR,                                      // horizontal upside-down
        ST77XX_MADCTL_MX | ST7735_MADCTL_BGR,                                      // vertical top-to-bottom
        ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_MY | ST7735_MADCTL_BGR // horizontal
};

void set_display_color_filter(uint8_t curve)
{
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Display color filter: %d", curve);
  /*
  uint16_t color_filters[4] = {0xFFFF, 0x07E0, 0x001F, 0xF800};
  uint16_t color_mask = color_filters[curve];
  */
  panel_st7735_set_color_channel(lcd_panel, curve); // lcd driver will actually touch all the data before drawing, unless curve is 0
}

void st7735_set_rotation_and_mirror(uint8_t rotation, uint8_t mirror)
{
  if (mirror)
    rotation += 4; // use the mirrored values

  esp_lcd_panel_io_tx_param(lcd_io, ST77XX_MADCTL, &madctl_value[rotation], 1);
}

esp_err_t startup_lcd(void)
{
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "LCD startup");
  esp_err_t ret = ESP_OK;

  /* LCD backlight */
  gpio_config_t bk_gpio_config = {
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << LCD_GPIO_BL};
  ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

  /* LCD initialization */
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "SPI bus init");
  const spi_bus_config_t buscfg = {
      .sclk_io_num = LCD_GPIO_SCLK,
      .mosi_io_num = LCD_GPIO_MOSI,
      .miso_io_num = GPIO_NUM_NC,
      .quadwp_io_num = GPIO_NUM_NC,
      .quadhd_io_num = GPIO_NUM_NC,
      .max_transfer_sz = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
  };

  ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Panel IO install");
  const esp_lcd_panel_io_spi_config_t io_config = {
      .dc_gpio_num = LCD_GPIO_DC,
      .cs_gpio_num = LCD_GPIO_CS,
      .pclk_hz = LCD_PIXEL_CLK_HZ,
      .lcd_cmd_bits = LCD_CMD_BITS,
      .lcd_param_bits = LCD_PARAM_BITS,
      .spi_mode = 0,
      .trans_queue_depth = 10,
  };
  ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &io_config, &lcd_io), err, TAG, "New panel IO failed");

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "LCD driver install");
  const esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = LCD_GPIO_RST,
      .color_space = LCD_COLOR_SPACE,
      .bits_per_pixel = LCD_BITS_PER_PIXEL,
  };

  // ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");
  ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7735(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");
  esp_lcd_panel_reset(lcd_panel);
  esp_lcd_panel_init(lcd_panel);
  esp_lcd_panel_mirror(lcd_panel, true, true);
  esp_lcd_panel_disp_on_off(lcd_panel, true);

  /* LCD backlight on */
  // ESP_ERROR_CHECK(gpio_set_level(LCD_GPIO_BL, LCD_BL_ON_LEVEL));
  // we don't use backlight on a projection LCD :)

  return ret;

err:
  if (lcd_panel)
  {
    esp_lcd_panel_del(lcd_panel);
  }
  if (lcd_io)
  {
    esp_lcd_panel_io_del(lcd_io);
  }
  spi_bus_free(LCD_SPI_NUM);

  return ret;
}

esp_err_t startup_lvgl(void)
{
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "LVGL startup");

  /* Initialize LVGL port - this will create the LVGL task and call lv_init() internally */
  const lvgl_port_cfg_t lvgl_cfg = {
      .task_priority = 8,       /* LVGL task priority - increased from 1 */
      .task_stack = 8192+2048,       /* LVGL task stack size SUPER LARGE STACK */
      .task_affinity = 1,       /* LVGL task pinned to core (core 1 for our display tasks, -1 would be no affinity) */
      .task_max_sleep_ms = 100, /* 100ms */
      .timer_period_ms = 5      /* Reduced timer period for smoother animations */
  };
  ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

  /* Add LCD screen */
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Add LCD screen");
  const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = lcd_io,
      .panel_handle = lcd_panel,
      .buffer_size = LCD_H_RES * LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
      .double_buffer = LCD_DRAW_BUFF_DOUBLE,
      .hres = LCD_H_RES,
      .vres = LCD_V_RES,
      .monochrome = false,
      /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
      .rotation = {
          .swap_xy = false,
          .mirror_x = true,
          .mirror_y = true,
      },
      .flags = {
          .buff_dma = true,
          .full_refresh = false, // Disable full refresh to avoid memory issues
          .buff_spiram = false,  // Explicitly disable SPIRAM for better performance
#if LVGL_VERSION_MAJOR >= 9
          .swap_bytes = true,
#endif
      }};
  lvgl_disp = lvgl_port_add_disp(&disp_cfg);
  lv_display_set_rotation(lvgl_disp, LV_DISPLAY_ROTATION_90);

  return ESP_OK;
}

lv_obj_t *grid[64];
int grid_objs = 0;

void show_grid(uint8_t show)
{
  if (show)
  {
    if (!grid_objs) // has grid been created yet?
      create_grid(lv_scr_act());

    for (int i = 0; i < grid_objs; i++)
      lv_obj_clear_flag(grid[i], LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    for (int i = 0; i < grid_objs; i++)
      lv_obj_add_flag(grid[i], LV_OBJ_FLAG_HIDDEN);
  }
}

void create_grid(lv_obj_t *scr)
{
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Create grid");

  static lv_point_precise_t v_points[][2] = {
      {{20, 0}, {20, 128}},
      {{30, 0}, {30, 128}},
      {{40, 0}, {40, 128}},
      {{90, 0}, {90, 128}},
      {{100, 0}, {100, 128}},
      {{110, 0}, {110, 128}},
  };

  static lv_point_precise_t h_points[][2] = {
      {{0, 20}, {128, 20}},
      {{0, 30}, {128, 30}},
      {{0, 40}, {128, 40}},
      {{0, 90}, {128, 90}},
      {{0, 100}, {128, 100}},
      {{0, 110}, {128, 110}},
  };

  // 1) Vertical lines (red)
  // Optimization: Fix out-of-bounds loop condition by using < instead of <=
  for (int i = 0; i < sizeof(v_points) / sizeof(v_points[0]); i++)
  {

    lv_obj_t *line_v = lv_line_create(scr);
    lv_line_set_points(line_v, v_points[i], 2);

    lv_obj_set_style_line_color(line_v, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_line_width(line_v, 1, 0); // Changed to 1 pixel
    lv_obj_set_style_line_opa(line_v, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(line_v, LV_OPA_TRANSP, 0);
    grid[grid_objs++] = line_v;
  }

  // 2) Horizontal lines (green)
  // Optimization: Fix out-of-bounds loop condition by using < instead of <=
  for (int i = 0; i < sizeof(h_points) / sizeof(h_points[0]); i++)
  {
    lv_obj_t *line_h = lv_line_create(scr);
    lv_line_set_points(line_h, h_points[i], 2);

    lv_obj_set_style_line_color(line_h, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_line_width(line_h, 1, 0); // Changed to 1 pixel
    lv_obj_set_style_line_opa(line_h, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_opa(line_h, LV_OPA_TRANSP, 0);
    grid[grid_objs++] = line_h;
  }
}

// warning - assumes it is running in a port lock/unlock wrapper
void set_scroll_message(const char *msg)
{
  // Add null pointer check
  if (msg == NULL || label_msg == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "set_scroll_message: null pointer");
    return;
  }
  // Validate font index bounds
  if (eeprom_msg_font > MAX_FONT_INDEX)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "set_scroll_message: invalid font %d, using default", eeprom_msg_font);
    eeprom_msg_font = 0; // Reset to default
  }
  // Get the selected font and validate it
  const lv_font_t *font = get_selected_font(eeprom_msg_font);
  if (font == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "set_scroll_message: invalid font, using default");
    font = &frixos_8; // Use default font
  }

  // Create a temporary buffer to measure text size
  lv_point_t size;

  lvgl_port_lock(0);
  // Use LVGL's safe text setting function
  lv_label_set_text(label_msg, msg);
  lv_text_get_size(&size, msg, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  label_size = size.x;
  if (label_size > MSG_WIDTH)
  { // scrolling, left aligned
    lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(label_msg, LV_SIZE_CONTENT);
    lv_obj_set_pos(label_msg, eeprom_ofs_x + MSG_WIDTH, eeprom_ofs_y + label_msg_ofs_y);
    label_max_pos = (label_size);
  }
  else
  { // centered
    lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_msg, MSG_WIDTH - MSG_EXTRA_WIDTH);
    lv_obj_set_pos(label_msg, eeprom_ofs_x, eeprom_ofs_y + label_msg_ofs_y);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "set_scroll_message: centered");
    label_max_pos = 0;
  }
  lvgl_port_unlock();
  // Calculate optimal animation time based on text length
  // Use a minimum time for short text and scale up for longer text
  // uint32_t anim_time = LV_MAX(MIN_ANIMATION_TIME, eeprom_scroll_delay * size.x);
  // lv_obj_set_style_anim_time(label_msg, anim_time, LV_PART_MAIN);
  // lv_obj_update_layout(label_msg);
  // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "set_scroll_message: anim_time %d, delay %d", anim_time, eeprom_scroll_delay);
}

// this is reboot only stuff
void startup_display(void)
{
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Display startup");
  lv_obj_t *scr = lv_scr_act();

  lvgl_port_lock(0);
  st7735_set_rotation_and_mirror(eeprom_rotation, eeprom_mirroring);
  show_grid(eeprom_show_grid); // will create the grid if needed

  // Remove eeprom_ofs_x/y assignments since we'll use eeprom_ofs_x/y directly

  // Set the background color to BLACK
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); // (R,G,B)
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  img_logo = lv_image_create(lv_scr_act());
  lv_image_set_src(img_logo, "S:/logo.jpg");
  lv_image_set_inner_align(img_logo, LV_ALIGN_CENTER);
  lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 20, eeprom_ofs_y + 10);

  label_msg = lv_label_create(lv_scr_act());
  if (label_msg == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create label_msg");
    return;
  }
  lv_label_set_long_mode(label_msg, LV_LABEL_LONG_CLIP);
  lv_obj_set_height(label_msg, LV_SIZE_CONTENT);
  lv_obj_set_width(label_msg, LV_SIZE_CONTENT); // Set fixed width to match available space
  lv_obj_set_style_text_color(label_msg, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(label_msg, get_selected_font(eeprom_msg_font), 0);
  lv_obj_set_style_bg_color(label_msg, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(label_msg, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(label_msg, eeprom_ofs_x, eeprom_ofs_y + label_msg_ofs_y);

  // Optimize label for smooth scrolling
  // lv_obj_set_style_anim_speed(label_msg, 150, LV_PART_MAIN); // Optimized animation speed for smoothness
  // lv_obj_set_style_anim_time(label_msg, 1000, LV_PART_MAIN); // Base animation time

  set_scroll_message("Starting...");

  img_digits_sprite = lv_image_create(lv_scr_act());
  img_weather = lv_image_create(lv_scr_act());
  img_moon = lv_image_create(lv_scr_act());
  img_glucose = lv_image_create(lv_scr_act());
  img_glucose_trend = lv_image_create(lv_scr_act());
  img_wifi = lv_image_create(lv_scr_act());
  img_mgdl = lv_image_create(lv_scr_act());
  img_ampm = lv_image_create(lv_scr_act()); // AMPM indicator
  for (int i = 0; i < 4; i++)
  {
    digit_objs[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(digit_objs[i], LV_OBJ_FLAG_HIDDEN); // Hide
  }
  for (int i = 0; i < 2; i++)
  {
    dots[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(dots[i], LV_OBJ_FLAG_HIDDEN); // Hide
  }

  // hide objects
  lv_obj_add_flag(img_digits_sprite, LV_OBJ_FLAG_HIDDEN); // Hide master copy
  lv_obj_add_flag(img_moon, LV_OBJ_FLAG_HIDDEN);          // Hide
  lv_obj_add_flag(img_ampm, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_weather, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_glucose, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_glucose_trend, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_wifi, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_mgdl, LV_OBJ_FLAG_HIDDEN);
  lvgl_port_unlock(); // Unlock LVGL

  display_changed(); // set the objects to the initial state
}

int check_file(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (fp != NULL)
  {
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "check_file: %s found", filename);
    fclose(fp);
    return 1;
  }
  else
  {
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "check_file: %s not found", filename);
    return 0;
  }
}

void find_file(char *filename, char len, char *theme, char *item)
{

  // look for BMP files first, then JPG
  // Try theme-specific file first
  snprintf(filename, len, "/spiffs/%s-%s.bmp", theme, item);
  if (check_file(filename))
  {
    // Theme-specific file exists, return without /spiffs/
    snprintf(filename, len, "S:/%s-%s.bmp", theme, item);
    return;
  }

  // Try default file
  snprintf(filename, len, "/spiffs/default-%s.bmp", item);
  if (check_file(filename))
  {
    // Default file exists, return without /spiffs/
    snprintf(filename, len, "S:/default-%s.bmp", item);
    return;
  }

  // Try theme-specific file first
  snprintf(filename, len, "/spiffs/%s-%s.jpg", theme, item);
  if (check_file(filename))
  {
    // Theme-specific file exists, return without /spiffs/
    snprintf(filename, len, "S:/%s-%s.jpg", theme, item);
    return;
  }

  // Try default file
  snprintf(filename, len, "/spiffs/default-%s.jpg", item);
  if (check_file(filename))
  {
    // Default file exists, return without /spiffs/
    snprintf(filename, len, "S:/default-%s.jpg", item);
    return;
  }

  // Fallback to logo
  snprintf(filename, len, "S:/logo.jpg");
}

void show_object(lv_obj_t *obj, bool show)
{
  if (!obj) // sanity check
    return;

  if (show) // show the object
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  else // hide the object
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

const lv_font_t *get_selected_font(uint8_t font_index)
{
  static const lv_font_t *font_array[] = {
      &frixos_8, // index 0
      &frixos_9, // index 1
      &frixos_11 // index 3
  };

  // Bounds check and return default if out of range
  if (font_index > MAX_FONT_INDEX)
  {
    return &frixos_8; // Default fallback
  }

  return font_array[font_index];
}

uint8_t get_selected_font_height(uint8_t font_index)
{
  static const uint8_t height_array[] = {
      8, // frixos_8 (index 0)
      9, // lv_font_montserrat_8 (index 1)
      11 // lv_font_montserrat_12 (index 3)
  };

  // Bounds check and return default if out of range
  if (font_index > MAX_FONT_INDEX)
  {
    return 8; // Default fallback
  }

  return height_array[font_index];
}

static bool is_glucose_on()
{
  return (wifi_connected && (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT]));
}

static bool is_glucose_fresh()
{
  // Guard against division by zero and invalid timestamp
  time_t now = time(NULL);
  time_t timestamp = glucose_data.timestamp;
  bool is_fresh = false;

  // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "is_glucose_fresh: timestamp: %ld, now: %ld, diff %ld, duration: %ld", (long)timestamp, (long)now, (long)now - timestamp, glucose_validity_duration);
  // Only calculate if timestamp is valid (non-zero)
  if (timestamp > 0 && glucose_validity_duration > 0)
  {
    is_fresh = ((now - timestamp) <= (time_t)(glucose_validity_duration * 60));
  }

  return is_fresh;
}

// this is the initialization needed when stuff about the display has changed
void display_changed(void)
{
  lv_obj_t *scr = lv_scr_act();
  char buf[128];
  /* Task lock */

  // Reset alternate display state to force update
  alternate_display_start = 0;
  showing_glucose = false;


  lvgl_port_lock(0);
  st7735_set_rotation_and_mirror(eeprom_rotation, eeprom_mirroring);
  // Set the background color to BLACK
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); // (R,G,B)
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  show_grid(eeprom_show_grid);
  find_file(buf, sizeof(buf), eeprom_font[font_index], "font");
  lv_image_set_src(img_digits_sprite, buf);
  lv_obj_align(img_digits_sprite, LV_ALIGN_TOP_LEFT, 0, 0);
  lvgl_port_unlock();

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Display change: lx %.1f idx %i flt %i fnt %s",
              lux, font_index, eeprom_color_filter[font_index], buf);

  lvgl_port_lock(0);

  find_file(buf, sizeof(buf), eeprom_font[font_index], "weather");
  lv_image_set_src(img_weather, buf);
  lv_image_set_inner_align(img_weather, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_weather, 32, 22);
  lv_obj_align(img_weather, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 32, eeprom_ofs_y + 2);

  find_file(buf, sizeof(buf), eeprom_font[font_index], "moon");
  lv_image_set_src(img_moon, buf);
  lv_image_set_inner_align(img_moon, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_moon, 14, 14);
  lv_obj_align(img_moon, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 32 + 32 + 1, eeprom_ofs_y + 7);

  // Load glucose icon
  find_file(buf, sizeof(buf), eeprom_font[font_index], "glucose");
  lv_image_set_src(img_glucose, buf);
  lv_image_set_inner_align(img_glucose, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_glucose, 14, 20);
  lv_obj_align(img_glucose, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 5, eeprom_ofs_y + 5);

  // Load WiFi disabled icon
  find_file(buf, sizeof(buf), eeprom_font[font_index], "wifi-off");
  lv_image_set_src(img_wifi, buf);
  lv_image_set_inner_align(img_wifi, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_wifi, 20, 20);
  lv_obj_align(img_wifi, LV_ALIGN_TOP_LEFT, eeprom_ofs_x, eeprom_ofs_y + 5);

  find_file(buf, sizeof(buf), eeprom_font[font_index], "trend");
  lv_image_set_src(img_glucose_trend, buf);
  lv_image_set_inner_align(img_glucose_trend, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_glucose_trend, 12, 14);
  lv_obj_align(img_glucose_trend, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 5 + 14 + 1, eeprom_ofs_y + 8);

  // Load mgdl image
  find_file(buf, sizeof(buf), eeprom_font[font_index], "units");
  lv_image_set_src(img_mgdl, buf);
  lv_image_set_inner_align(img_mgdl, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_mgdl, 12, 24);
  lv_obj_align(img_mgdl, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 79, eeprom_ofs_y + 30); // align to the right of the 4th digit

  find_file(buf, sizeof(buf), eeprom_font[font_index], "ampm");
  lv_image_set_src(img_ampm, buf);
  lv_image_set_inner_align(img_ampm, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_ampm, 10, 20);

  lvgl_port_unlock(); // Unlock LVGL

  lvgl_port_lock(0);
  // now setup the 4 visible digits
  for (int i = 0; i < 4; i++)
  {
    lv_image_set_src(digit_objs[i], lv_image_get_src(img_digits_sprite));
    lv_image_set_inner_align(digit_objs[i], LV_ALIGN_TOP_LEFT);
    lv_obj_set_size(digit_objs[i], DIGIT_WIDTH, DIGIT_HEIGHT);
    lv_obj_align(digit_objs[i], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + i * 18 + (i > 1 ? 6 : 0), eeprom_ofs_y + 25);
    // i>1: extra 6 pixels for the colon
  }
  lv_obj_align(img_ampm, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 4 * 18 + 6 + 1, eeprom_ofs_y + 32);

  // now setup the colon dots
  find_file(buf, sizeof(buf), eeprom_font[font_index], "dot");
  for (int i = 0; i < 2; i++)
    lv_image_set_src(dots[i], buf);

  lv_obj_align(dots[0], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 2 * 18 + 1, eeprom_ofs_y + 25 + 10);
  lv_obj_align(dots[1], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 2 * 18 + 1, eeprom_ofs_y + 25 + 26);

  lv_obj_set_style_text_color(label_msg, lv_color_make(eeprom_msg_red[font_index], eeprom_msg_green[font_index], eeprom_msg_blue[font_index]), 0);
  // Set the font based on eeprom_msg_font parameter
  lv_obj_set_style_text_font(label_msg, get_selected_font(eeprom_msg_font), 0);
  // Invalidate character width cache when font changes
  invalidate_char_width_cache();
  lv_obj_set_height(label_msg, LV_SIZE_CONTENT);
  lv_obj_set_pos(label_msg, eeprom_ofs_x, eeprom_ofs_y + label_msg_ofs_y);

  // display_changed invalidates tokens, so it causes a set_scroll_message from the main thread anyways
  // set_scroll_message(msg_scrolling);

  show_object(img_ampm, eeprom_12hour && time_valid);                                  // show AM/PM indicator if 12 hour time AND we have valid time
  show_object(img_weather, eeprom_quiet_weather && last_weather_update && time_valid); // show weather forecast icon if weather data is valid
  show_object(img_moon, eeprom_quiet_weather && time_valid);                           // show moon icon if we have valid time
  show_object(label_msg, eeprom_quiet_scroll || !time_valid);                          // show scrolling information message if quiet scroll is enabled or time is not valid

  // Show/hide glucose icon, WiFi icon, and trend arrow
  if (wifi_disabled_by_active_hours)
  {
    show_object(img_wifi, true);
    show_object(img_glucose, false);
    show_object(img_glucose_trend, false); // Hide trend arrow when WiFi is disabled
    show_object(img_mgdl, false);          // Hide mgdl when WiFi is disabled
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi off, trend arrow hidden");
  }
  else
  {
    show_object(img_wifi, false);
  }

  if (weather_valid) // only if we have valid weather data
    update_weather_msg();
  lvgl_port_unlock(); // Unlock LVGL

  // Apply gamma curve based on eeprom_color_filter, after we get on wifi
  if (wifi_connected)
  {
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Color filter: %u (font %u, wifi %s)",
                eeprom_color_filter[font_index], font_index, wifi_connected ? "true" : "false");
    set_display_color_filter(eeprom_color_filter[font_index]);
  }

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Display changed");
}

// allow -1 to display nothing
void display_digit(int position, int digit)
{
  if (digit < -1 || digit > 9)
    return;

  lv_image_set_offset_x(digit_objs[position], -digit * DIGIT_WIDTH);
  // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Displayed digit %d at position %d (x=%d)", digit, position, digit * DIGIT_WIDTH);
}

float ease_in_quad(float t)
{
  return t * t; // Starts slow, accelerates
}

float ease_in_out_quad(float t)
{
  // Optimization: replace pow(..., 2) with direct multiplication for better performance on ESP32
  return t < 0.5 ? 2 * t * t : 1 - ((-2 * t + 2) * (-2 * t + 2)) / 2;
}

// Fade effect callback
static void fade_timer_cb(void *arg)
{
  // Calculate opacity (0 to 255)
  float t = (float)fade_step / FADE_STEPS;    // Normalize 0-1
  float opacity_factor = ease_in_out_quad(t); // Apply easing
  int opacity = (int)(opacity_factor * 255);  // Convert to LVGL opacity

  // Store the opacity values for the display task to apply
  static int last_opacity = -1;
  if (opacity != last_opacity)
  {
    last_opacity = opacity;
    fade_update_needed = true;
  }

  if (fading_in)
  {
    fade_step++;
    if (fade_step > FADE_STEPS)
    {
      fading_in = false;
      fade_step = FADE_STEPS - 1;
    }
  }
  else
  {
    fade_step--;
    if (fade_step < 0)
    {
      fading_in = true;
      fade_step = 0;
    }
  }
}

void update_weather_msg(void)
{
  lvgl_port_lock(0);
  integration_tokens_updated = true; // signal that tokens have been updated
  show_object(img_weather, eeprom_quiet_weather && last_weather_update > 0);
  lvgl_port_unlock();
}

void show_qr_code(void)
{
  lvgl_port_lock(0);
  lv_image_set_src(img_logo, "S:/wifi-qr.jpg");
  lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 15, eeprom_ofs_y + 10);
  set_scroll_message(" Scan QR code to connect to your Frixos ");
  lvgl_port_unlock();
}

// Watchdog callback function (defined outside display_task)
static volatile bool watchdog_triggered = false;

static void watchdog_callback(void *arg)
{
  ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Watchdog: display task hung");
  watchdog_triggered = true;
  // Force a system restart after 5 seconds if the task is still hung
  ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Restart in 5s (display hang)");
  // Note: In a real implementation, you might want to call esp_restart() directly
  // or use a proper restart mechanism. For now, we'll just log the issue.
}

static int last_integration_update_hour = -1; // Track last hour we updated integration message
static time_t lastrun = 0;

static void handle_als_and_brightness(uint32_t loop_counter)
{
  // read ALS sensor every 5 seconds. each pass is 50ms, so 100 passes = 5 seconds
  if (loop_counter % 100 == 1)
  {
    // Get the current lux value
    lux = ltr303_get_frixos_lux();
    int8_t pwmpct = -1;
    uint8_t old_font_index = font_index;

    if (lux > eeprom_lux_threshold + eeprom_lux_sensitivity)
      font_index = 0;
    else if (lux < eeprom_lux_threshold - eeprom_lux_sensitivity)
      font_index = 1;

    // if dim_disable is enabled or we are in manufacturer mode, force full brightness
    if (eeprom_dim_disable || manufacturer_mode)
      font_index = 0; // force full brightness

    pwmpct = eeprom_brightness_LED[font_index];
    if (font_index != old_font_index)
    {
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Font %u->%u (lux %.1f thres %.1f)",
                  old_font_index, font_index, lux, eeprom_lux_threshold);
      set_led_pwm_brightness(pwmpct);
      display_changed();
    }
  }
}

static void handle_wifi_status_icon(void)
{
  static bool last_wifi_disabled_state = false;
  if (wifi_disabled_by_active_hours != last_wifi_disabled_state)
  {
    last_wifi_disabled_state = wifi_disabled_by_active_hours;
    lvgl_port_lock(0);
    if (wifi_disabled_by_active_hours)
    {
      show_object(img_wifi, true);
      show_object(img_glucose, false);
      show_object(img_glucose_trend, false);
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi disabled (active hours)");
    }
    else
    {
      show_object(img_wifi, false);
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi enabled");
    }
    lvgl_port_unlock();
  }
}

static void handle_integration_and_messages(void)
{
  if (integration_tokens_updated || (timeinfo.tm_min == 0 && timeinfo.tm_hour != last_integration_update_hour) || (!ota_update_in_progress && ota_updating_message) || (show_ip_on_boot && !ip_message_set))
  {
    // Glucose icon handling
    if (is_glucose_on())
    {
      int glucose_index = 1; // green
      if (glucose_data.current_gl_mgdl > eeprom_glucose_high)
        glucose_index = 2; // yellow
      else if (glucose_data.current_gl_mgdl < eeprom_glucose_low)
        glucose_index = 0; // red
      if (!is_glucose_fresh())
        glucose_index = 3; // gray

      lvgl_port_lock(0);
      lv_image_set_offset_x(img_glucose, -glucose_index * 14);
      lv_image_set_offset_x(img_glucose_trend, -glucose_data.trend_arrow * 12);
      show_object(img_glucose, is_glucose_on());
      show_object(img_glucose_trend, is_glucose_fresh());
      lvgl_port_unlock();
    }
    else
    {
      lvgl_port_lock(0);
      show_object(img_glucose, false);
      show_object(img_glucose_trend, false);
      lvgl_port_unlock();
    }

    // IP address and OTA "updating..." message display handling
    if (show_ip_on_boot && !ip_message_set)
    {
      char ip_message[64];
      snprintf(ip_message, sizeof(ip_message), "%s ", boot_ip_address);
      lvgl_port_lock(0);
      set_scroll_message(ip_message);
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Displaying IP address: %s for %d seconds", ip_message, IP_DISPLAY_DURATION_SEC);
      lvgl_port_unlock();
      ip_message_set = true;
      ip_display_start_time = esp_timer_get_time();
    }
    else if (!ota_update_in_progress && !show_ip_on_boot)
    {
      replace_placeholders(eeprom_message, msg_scrolling, sizeof(msg_scrolling));
      lvgl_port_lock(0);
      set_scroll_message(msg_scrolling);
      lvgl_port_unlock();
    }
    else if (ota_update_in_progress)
    {
      if (!ota_updating_message)
      {
        set_scroll_message("Updating...");
        ota_updating_message = true;
      }
      if (ota_start_time == 0)
        ota_start_time = time(NULL);
      if (time(NULL) - ota_start_time > 300)
      {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "OTA timeout, restoring message");
        ota_update_in_progress = false;
        ota_updating_message = false;
        ota_start_time = 0;
      }
    }

    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Scroll message: %s", msg_scrolling);

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Mem: total %d free %d used %d%% frag %d%% largest %d",
                mon.total_size, mon.free_size, mon.used_pct, mon.frag_pct, mon.free_biggest_size);

    if (mon.free_size < 1024)
      ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low memory: %d bytes", mon.free_size);

    integration_tokens_updated = false;
    if (timeinfo.tm_min == 0)
      last_integration_update_hour = timeinfo.tm_hour;
  }
}

static void handle_alternate_mode_switching(time_t now, uint32_t loop_counter, bool *should_update_display)
{
  if (loop_counter % 20 == 1)
  {
    bool mode_changed = false;
    static bool last_showing_glucose = false;
    static time_t last_minute_slot = 0;
    time_t current_minute_slot = now / 60;
    bool minute_changed = (current_minute_slot != last_minute_slot);

    /* Alternate only when both phases have a non-zero duration; otherwise keep the clock on the digits. */
    alternate_display_active = (eeprom_sec_time > 0 && eeprom_sec_cgm > 0);

    static time_t last_alternate_display_start = 0;
    if (alternate_display_start == 0 && last_alternate_display_start != 0)
    {
      mode_changed = true;
      last_showing_glucose = false;
    }
    last_alternate_display_start = alternate_display_start;

    if (alternate_display_active && time_valid)
    {
      if (alternate_display_start == 0)
      {
        showing_glucose = false; /* always start with time, then switch to CGM after sec_time */
        alternate_display_start = now;
        mode_changed = true;
      }
      else
      {
        time_t elapsed = now - alternate_display_start;
        bool should_switch = false;
        if (showing_glucose)
        {
          if (eeprom_sec_cgm > 0 && elapsed >= eeprom_sec_cgm && eeprom_sec_time > 0)
            should_switch = true;
        }
        else
        {
          if (eeprom_sec_time > 0 && elapsed >= eeprom_sec_time && is_glucose_fresh())
            should_switch = true;
        }
        if (should_switch)
        {
          showing_glucose = !showing_glucose;
          alternate_display_start = now;
          mode_changed = true;
        }
      }
    }
    else
    {
      if (showing_glucose != false) mode_changed = true;
      showing_glucose = false;
      alternate_display_start = 0;
    }

    if (mode_changed || (showing_glucose != last_showing_glucose))
    {
      *should_update_display = true;
      last_showing_glucose = showing_glucose;
    }
    else if (!showing_glucose)
    {
      *should_update_display = ((minute_changed || (now % 60 == 0) || (time_just_validated == 1) || weather_has_updated) && time_valid && (now - lastrun > 3));
      last_minute_slot = current_minute_slot;
    }
  }
}

static void update_display_content(time_t now)
{
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_min % 10 == 1)
  {
    char buf[128];
    snprintf(buf, sizeof(buf), "Tck %li", (int32_t)(now - lastrun));
    ESP_LOGI_STACK(TAG, buf);
  }

  if (weather_has_updated || (timeinfo.tm_min == 1))
  {
    update_weather_msg();
    weather_has_updated = false;
  }

  int digit1, digit2, digit3, digit4;
  bool show_dots = true;
  bool show_ampm = false;

  if (showing_glucose && alternate_display_active && is_glucose_on() && glucose_data.current_gl_mgdl > 0)
  {
    if (eeprom_glucose_unit == 1)
    {
      float glucose_mmol = glucose_data.current_gl_mgdl / 18.0182f;
      if (glucose_mmol > 22.0f) glucose_mmol = 22.0f;
      if (glucose_mmol < 0.0f) glucose_mmol = 0.0f;
      int whole_part = (int)glucose_mmol;
      int decimal_part = (int)((glucose_mmol - whole_part) * 10.0f + 0.5f);
      if (decimal_part >= 10) { whole_part++; decimal_part = 0; }
      digit1 = -1;
      if (whole_part >= 10) { digit2 = whole_part / 10; digit3 = whole_part % 10; digit4 = decimal_part; }
      else { digit2 = -1; digit3 = whole_part; digit4 = decimal_part; }
      show_dots = true;
      show_ampm = false;
    }
    else
    {
      int glucose_value = (int)glucose_data.current_gl_mgdl;
      if (glucose_value > 999) glucose_value = 999;
      if (glucose_value < 0) glucose_value = 0;
      digit1 = -1;
      if (glucose_value >= 100) { digit2 = glucose_value / 100; digit3 = (glucose_value / 10) % 10; digit4 = glucose_value % 10; }
      else { digit2 = -1; digit3 = glucose_value / 10; digit4 = glucose_value % 10; }
      show_dots = false;
      show_ampm = false;
    }
  }
  else
  {
    bool is_pm = false;
    if (eeprom_12hour)
    {
      if (timeinfo.tm_hour == 0) timeinfo.tm_hour = 12;
      else if (timeinfo.tm_hour > 12) { timeinfo.tm_hour -= 12; is_pm = true; }
    }
    digit1 = timeinfo.tm_hour / 10;
    digit2 = timeinfo.tm_hour % 10;
    digit3 = timeinfo.tm_min / 10;
    digit4 = timeinfo.tm_min % 10;
    if (!eeprom_show_leading_zero && digit1 == 0) digit1 = -1;
    show_dots = true;
    show_ampm = (eeprom_12hour && is_pm);
  }

  lvgl_port_lock(0);
  display_digit(0, digit1);
  display_digit(1, digit2);
  display_digit(2, digit3);
  display_digit(3, digit4);

  if (showing_glucose && alternate_display_active)
  {
    lv_obj_align(digit_objs[1], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 1 * 18 + 6, eeprom_ofs_y + 25);
    show_object(img_ampm, false);
  }
  else
  {
    lv_obj_align(digit_objs[1], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 1 * 18, eeprom_ofs_y + 25);
    show_object(img_ampm, eeprom_12hour && time_valid);
  }

  lv_image_set_offset_x(img_weather, -weather_icon_index * 32);
  lv_image_set_offset_x(img_moon, -moon_icon_index * 14);
  lv_image_set_offset_x(img_ampm, show_ampm ? -10 : 0);

  if (showing_glucose && alternate_display_active && eeprom_glucose_unit == 1)
  {
    show_object(dots[0], true);
    show_object(dots[1], false);
    int dot_x_pos = eeprom_ofs_x + 3 * 18 + 6 - 4;
    lv_obj_align(dots[0], LV_ALIGN_TOP_LEFT, dot_x_pos, eeprom_ofs_y + 25 + 30);
  }
  else
  {
    show_object(dots[0], show_dots);
    show_object(dots[1], show_dots);
    if (!showing_glucose || !alternate_display_active)
    {
      lv_obj_align(dots[0], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 2 * 18 + 1, eeprom_ofs_y + 25 + 10);
      lv_obj_align(dots[1], LV_ALIGN_TOP_LEFT, eeprom_ofs_x + 2 * 18 + 1, eeprom_ofs_y + 25 + 26);
    }
  }
  show_object(img_mgdl, showing_glucose && alternate_display_active && is_glucose_on() && glucose_data.current_gl_mgdl > 0);
  lv_image_set_offset_x(img_mgdl, -eeprom_glucose_unit * 12);
  lvgl_port_unlock();

  last_minute = timeinfo.tm_min;

  if (time_just_validated == 1)
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Time validated, showing clock");
    time_just_validated = 0;
    lvgl_port_lock(0);
    if (img_logo) lv_obj_add_flag(img_logo, LV_OBJ_FLAG_HIDDEN);
    for (int i = 0; i < 4; i++) lv_obj_remove_flag(digit_objs[i], LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(img_moon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dots[0], LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(dots[1], LV_OBJ_FLAG_HIDDEN);
    lvgl_port_unlock();
    if (weather_valid) update_weather_msg();
  }
  lastrun = now;
}

void display_task(void *pvParameters)
{
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "display_task started");
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Display state: font %u, day_flt %u, night_flt %u",
              font_index, eeprom_color_filter[0], eeprom_color_filter[1]);

  esp_timer_create_args_t fade_args = {.callback = fade_timer_cb, .name = "fade_timer"};
  esp_timer_create(&fade_args, &fade_timer);
  esp_timer_start_periodic(fade_timer, FADE_INTERVAL * 1000);

  esp_timer_handle_t watchdog_timer = NULL;
  esp_timer_create_args_t watchdog_args = {.callback = watchdog_callback, .name = "display_watchdog"};
  esp_timer_create(&watchdog_args, &watchdog_timer);
  esp_timer_start_periodic(watchdog_timer, 30000000);

  TickType_t lastrun_tick = xTaskGetTickCount();
  time_t now;
  static uint32_t loop_counter = 0;

  while (1)
  {
    loop_counter++;
    if (loop_counter % 1000 == 1)
    {
      UBaseType_t stack_high_water = uxTaskGetStackHighWaterMark(NULL);
      ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Display loop %lu, stack %lu", loop_counter, stack_high_water);
      if (stack_high_water < 1024) ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low stack: %lu bytes", stack_high_water);
    }

    time(&now);

    if (settings_updated)
    {
      ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Settings updated");
      alternate_display_start = 0;
      showing_glucose = false;
      display_changed();
      settings_updated = false;
    }

    handle_wifi_status_icon();
    handle_integration_and_messages();
    handle_als_and_brightness(loop_counter);

    bool should_update_display = false;
    handle_alternate_mode_switching(now, loop_counter, &should_update_display);

    if (should_update_display)
    {
      update_display_content(now);
    }

    // Handle fade updates
    if (fade_update_needed)
    {
      fade_update_needed = false;
      // Calculate current opacity
      float t = (float)fade_step / FADE_STEPS;
      float opacity_factor = ease_in_out_quad(t);
      int opacity = (int)(opacity_factor * 255);

      // Apply opacity to the dots
      lvgl_port_lock(0);
      if (eeprom_dots_breathe == 1 || showing_glucose)
      {
        // Breathing disabled - show dots at full brightness
        lv_obj_set_style_opa(dots[0], 255, LV_PART_MAIN);
        lv_obj_set_style_opa(dots[1], 255, LV_PART_MAIN);
      }
      else
      {
        // Breathing enabled - use variable opacity
        lv_obj_set_style_opa(dots[0], opacity, LV_PART_MAIN);
        lv_obj_set_style_opa(dots[1], 255 - opacity, LV_PART_MAIN);
      }
      lvgl_port_unlock();
    }

    // Check if IP display timer has expired
    if (show_ip_on_boot && ip_display_start_time > 0)
    {
      int64_t elapsed_us = esp_timer_get_time() - ip_display_start_time;
      int64_t elapsed_sec = elapsed_us / 1000000; // Convert microseconds to seconds
      if (elapsed_sec >= IP_DISPLAY_DURATION_SEC)
      {
        // Done showing IP, return to normal message
        show_ip_on_boot = false;
        ip_message_set = false;
        ip_display_start_time = 0;
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "IP display done (%lld s)", (long long)elapsed_sec);
        integration_tokens_updated = true; // Force message update
      }
    }

    if (label_size > MSG_WIDTH)
    { // scrolling, left aligned
      // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Label pos %d, max %d", label_scroll_pos, label_max_pos);
      lvgl_port_lock(0);
      label_scroll_pos++;
      if (label_scroll_pos >= label_max_pos)
        label_scroll_pos = -MSG_WIDTH;

      display_string_substring(msg_scrolling, (eeprom_ofs_x > 6 ? eeprom_ofs_x - 6 : eeprom_ofs_x), eeprom_ofs_y + label_msg_ofs_y,
                               label_scroll_pos, MSG_WIDTH, label_msg, get_selected_font(eeprom_msg_font));
      lvgl_port_unlock();
    }

    // NOTE: Do NOT call lv_task_handler() here! The esp_lvgl_port creates its own
    // task that runs lv_timer_handler. Calling it from display_task causes two tasks
    // to run the LVGL timer handler, corrupting the event list and crashing in
    // lv_event_mark_deleted when objects are deleted. See LVGL issue #6677.

    // Check if watchdog was triggered
    if (watchdog_triggered)
    {
      ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Watchdog triggered, recovering");
      watchdog_triggered = false;
      // Try to recover by forcing a display refresh
      display_changed();
    }

    // Feed the watchdog timer by stopping and restarting it
    esp_timer_stop(watchdog_timer);
    esp_timer_start_periodic(watchdog_timer, 30000000);

    // vTaskDelay(pdMS_TO_TICKS(eeprom_scroll_delay));
    xTaskDelayUntil(&lastrun_tick, pdMS_TO_TICKS(eeprom_scroll_delay)); // this is probably causing jumpy scrolling
  } // end of while (1) loop

  ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Display task exit (unexpected)");

  // Cleanup timers (should never reach here)
  if (watchdog_timer)
  {
    esp_timer_stop(watchdog_timer);
    esp_timer_delete(watchdog_timer);
  }
  if (fade_timer)
  {
    esp_timer_stop(fade_timer);
    esp_timer_delete(fade_timer);
  }
}

// Define token types
typedef enum
{
  TOKEN_TYPE_BASE = 0, // Base tokens like [device], [greeting], etc.
  TOKEN_TYPE_HA,       // Home Assistant tokens
  TOKEN_TYPE_STOCK,    // Stock tokens
  TOKEN_TYPE_GLUCOSE,  // Any CGM Meter (Dexcom, Freestyle, etc.)
  TOKEN_TYPE_WEATHER,  // Weather tokens
  TOKEN_TYPE_TIME,     // Time-related tokens
                       // Add new token types here
} token_type_t;

// Define token structure
typedef struct
{
  const char *token;
  char *value;
  int id;
  token_type_t type;
} token_t;

// Define base tokens array
static const token_t base_tokens[] = {
    {"[device]", NULL, 1, TOKEN_TYPE_BASE},
    {"[greeting]", NULL, 2, TOKEN_TYPE_BASE},
    {"[day]", NULL, 3, TOKEN_TYPE_TIME},
    {"[date]", NULL, 4, TOKEN_TYPE_TIME},
    {"[mon]", NULL, 5, TOKEN_TYPE_TIME},
    {"[temp]", NULL, 6, TOKEN_TYPE_WEATHER},
    {"[hum]", NULL, 7, TOKEN_TYPE_WEATHER},
    {"[high]", NULL, 8, TOKEN_TYPE_WEATHER},
    {"[low]", NULL, 9, TOKEN_TYPE_WEATHER},
    {"[rise]", NULL, 10, TOKEN_TYPE_WEATHER},
    {"[set]", NULL, 11, TOKEN_TYPE_WEATHER},
    {NULL, NULL, 0, TOKEN_TYPE_BASE} // End marker
};

// Global token list that persists between calls
static const token_t *prepared_tokens = NULL;
static int prepared_tokens_count = 0;

// Function to prepare tokens - called when tokens change
void prepare_tokens(void)
{
  // Free existing tokens if any
  if (prepared_tokens != NULL && prepared_tokens != base_tokens)
  {
    // don't free any of the strings, they are pointers to the integration_active_tokens strings (which are dynamically allocated)
    free((void *)prepared_tokens);
    prepared_tokens = NULL;
    prepared_tokens_count = 0;
  }

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Prepare tokens");

  // Start with base tokens
  prepared_tokens = base_tokens;
  prepared_tokens_count = 11; // Base tokens count

  // Allocate space for all tokens
  int tokencount = 0;
  for (int i = 0; i < AVAILABLE_INTEGRATIONS; i++)
  {
    if (integration_active_tokens[i] != NULL && integration_active_tokens_count[i] > 0)
      tokencount += integration_active_tokens_count[i];
  }

  // Add space for CGM tokens (2 tokens per active CGM integration: [CGM:glucose] and [CGM:reading])
  // These tokens are always added when integration is active, regardless of token array
  if (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT])
  {
    tokencount += 2;
  }

  // Add safety check for reasonable token count
  if (tokencount > MAX_TOKEN_COUNT)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Token count %d, limit %d", tokencount, MAX_TOKEN_COUNT);
    tokencount = MAX_TOKEN_COUNT;
  }

  token_t *all_tokens = calloc(prepared_tokens_count + tokencount + 1, sizeof(token_t));
  if (all_tokens == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token alloc failed");
    return;
  }

  // Copy existing base tokens
  memcpy(all_tokens, base_tokens, prepared_tokens_count * sizeof(token_t));

  // Check if we have any active HA tokens
  if (integration_active_tokens[INTEGRATION_HA] != NULL && integration_active_tokens_count[INTEGRATION_HA] > 0)
  {

    // Add HA tokens with bounds checking
    for (int i = 0; i < integration_active_tokens_count[INTEGRATION_HA]; i++)
    {
      all_tokens[prepared_tokens_count].token = integration_active_tokens[INTEGRATION_HA][i].name;
      all_tokens[prepared_tokens_count].id = prepared_tokens_count + i + 1;
      all_tokens[prepared_tokens_count].type = TOKEN_TYPE_HA;
      all_tokens[prepared_tokens_count].value = integration_active_tokens[INTEGRATION_HA][i].value;
      prepared_tokens_count++;
    }
  }

  if (integration_active_tokens[INTEGRATION_STOCK] != NULL && integration_active_tokens_count[INTEGRATION_STOCK] > 0)
  {
    // Add stock tokens with bounds checking
    for (int i = 0; i < integration_active_tokens_count[INTEGRATION_STOCK]; i++)
    {
      all_tokens[prepared_tokens_count].token = (integration_active_tokens[INTEGRATION_STOCK][i].name);
      all_tokens[prepared_tokens_count].id = prepared_tokens_count + i + 1;
      all_tokens[prepared_tokens_count].type = TOKEN_TYPE_STOCK;
      all_tokens[prepared_tokens_count].value = integration_active_tokens[INTEGRATION_STOCK][i].value;
      prepared_tokens_count++;
    }
  }

  // Add CGM tokens - Dexcom and Freestyle use glucose_data directly (no token arrays)
  // Always add tokens if integration is active (regardless of current glucose data)
  // The actual value check happens in replace_placeholders() when replacing
  if (integration_active[INTEGRATION_DEXCOM])
  {
    // Add [CGM:glucose] token
    all_tokens[prepared_tokens_count].token = "[CGM:glucose]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
  }
  // Check if Freestyle Libre is active
  else if (integration_active[INTEGRATION_FREESTYLE])
  {
    // Add [CGM:glucose] token
    all_tokens[prepared_tokens_count].token = "[CGM:glucose]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
  }
  else if (integration_active[INTEGRATION_NIGHTSCOUT])
  {
    // Add [CGM:glucose] token
    all_tokens[prepared_tokens_count].token = "[CGM:glucose]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    prepared_tokens_count++;
  }

  // Add end marker
  all_tokens[prepared_tokens_count].token = NULL;
  all_tokens[prepared_tokens_count].id = 0;
  all_tokens[prepared_tokens_count].type = TOKEN_TYPE_BASE;

  // Update prepared tokens
  prepared_tokens = (const token_t *)all_tokens;
}

// Function to replace tokens in a string
void replace_placeholders(const char *input, char *output, size_t output_size)
{
  // Add null pointer and size checks
  if (input == NULL || output == NULL || output_size == 0)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "replace_placeholders: invalid params");
    if (output != NULL && output_size > 0)
    {
      output[0] = '\0';
    }
    return;
  }

  // Ensure tokens are prepared
  if (prepared_tokens == NULL)
  {
    prepare_tokens();
  }

  char temp[SCROLL_MSG_LENGTH];
  char *src = temp;
  char *dst = output;
  char *end = output + output_size - 1;

  // Get current local time for greeting calculation
  time_t now;
  struct tm current_timeinfo;
  time(&now);
  localtime_r(&now, &current_timeinfo);

  // replace greeting with good morning, good afternoon, good evening, or good night.
  // Use language-specific greetings based on eeprom_language
  uint8_t lang_index = eeprom_language;
  if (lang_index >= 9)
  {
    lang_index = 0; // Default to English if invalid language index
  }

  int greeting_index;
  if (current_timeinfo.tm_hour < 5)
  {
    greeting_index = 3; // good night
  }
  else if (current_timeinfo.tm_hour < 12)
  {
    greeting_index = 0; // good morning
  }
  else if (current_timeinfo.tm_hour < 18)
  {
    greeting_index = 1; // good afternoon
  }
  else if (current_timeinfo.tm_hour < 21)
  {
    greeting_index = 2; // good evening
  }
  else
  {
    greeting_index = 3; // good night
  }

  strcpy(greeting, greetings[lang_index][greeting_index]);

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Greeting hour %d: %s", current_timeinfo.tm_hour, greeting);

  // Copy input to temp buffer with proper bounds checking
  size_t input_len = strlen(input);
  if (input_len >= sizeof(temp))
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Input string truncated");
    input_len = sizeof(temp) - 1;
  }
  strncpy(temp, input, input_len);
  temp[input_len] = '\0';

  // Initialize output buffer
  output[0] = '\0';

  // ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Prepared tokens: %d", prepared_tokens_count);
  // debug: show all tokens
  for (int i = 0; i < prepared_tokens_count; i++)
  {
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Token %d: %s=%s", i,
                prepared_tokens[i].token ? prepared_tokens[i].token : "null",
                prepared_tokens[i].value ? prepared_tokens[i].value : "null");
  }

  while (*src && dst < end)
  {
    if (*src == '[')
    {
      // Check if we have a matching token
      const token_t *token = prepared_tokens;
      while (token->token)
      {
        if (strncmp(src, token->token, strlen(token->token)) == 0)
        {
          // Found a match, handle it in switch
          char replacement[64];
          strcpy(replacement, "-");
          switch (token->type)
          {
          case TOKEN_TYPE_BASE:
            switch (token->id)
            {
            case 1: // [device]
              snprintf(replacement, sizeof(replacement), "%s", eeprom_hostname);
              break;
            case 2: // [greeting]
              snprintf(replacement, sizeof(replacement), "%s", greeting);
              break;
            }
            break;

          case TOKEN_TYPE_TIME:
            switch (token->id)
            {
            case 3: // [day]
              snprintf(replacement, sizeof(replacement), "%s", day_names[lang_index][timeinfo.tm_wday]);
              break;
            case 4: // [date]
              snprintf(replacement, sizeof(replacement), "%d", timeinfo.tm_mday);
              break;
            case 5: // [mon]
              snprintf(replacement, sizeof(replacement), "%s", month_names[lang_index][timeinfo.tm_mon]);
              break;
            }
            break;

          case TOKEN_TYPE_WEATHER:
            if (!weather_valid)
            {
              strcpy(replacement, "-");
              break;
            }
            switch (token->id)
            {
            case 6: // [temp]
              if (eeprom_fahrenheit)
              {
                snprintf(replacement, sizeof(replacement), "%.0f°F", (weather_temp * 9.0 / 5.0) + 32);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.0f°C", weather_temp);
              }
              break;
            case 7: // [hum]
              snprintf(replacement, sizeof(replacement), "%.0f%%", weather_humidity);
              break;
            case 8: // [high]
              if (eeprom_fahrenheit)
              {
                snprintf(replacement, sizeof(replacement), "%.0f°F", (weather_high * 9.0 / 5.0) + 32);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.0f°C", weather_high);
              }
              break;
            case 9: // [low]
              if (eeprom_fahrenheit)
              {
                snprintf(replacement, sizeof(replacement), "%.0f°F", (weather_low * 9.0 / 5.0) + 32);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.0f°C", weather_low);
              }
              break;
            case 10: // [rise]
            {
              struct tm sunrise_tm;
              localtime_r(&sunrise, &sunrise_tm);
              if (eeprom_12hour)
              {
                bool is_pm = sunrise_tm.tm_hour >= 12;
                int hour = sunrise_tm.tm_hour;
                if (hour == 0)
                  hour = 12;
                else if (hour > 12)
                  hour -= 12;
                snprintf(replacement, sizeof(replacement), "%d:%02i%s", hour, sunrise_tm.tm_min, is_pm ? "pm" : "am");
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%02i:%02i", sunrise_tm.tm_hour, sunrise_tm.tm_min);
              }
            }
            break;
            case 11: // [set]
            {
              struct tm sunset_tm;
              localtime_r(&sunset, &sunset_tm);
              if (eeprom_12hour)
              {
                bool is_pm = sunset_tm.tm_hour >= 12;
                int hour = sunset_tm.tm_hour;
                if (hour == 0)
                  hour = 12;
                else if (hour > 12)
                  hour -= 12;
                snprintf(replacement, sizeof(replacement), "%d:%02i%s", hour, sunset_tm.tm_min, is_pm ? "pm" : "am");
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%02i:%02i", sunset_tm.tm_hour, sunset_tm.tm_min);
              }
            }
            break;
            }
            break;

          case TOKEN_TYPE_HA:
            // Find the corresponding HA token by name
            if (integration_active_tokens[INTEGRATION_HA] != NULL)
            {
              for (int i = 0; i < integration_active_tokens_count[INTEGRATION_HA]; i++)
              {
                if (integration_active_tokens[INTEGRATION_HA][i].name != NULL && integration_active_tokens[INTEGRATION_HA][i].name[0] != '\0' && integration_active_tokens[INTEGRATION_HA][i].value[0] != '\0')
                {
                  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Check HA %s", integration_active_tokens[INTEGRATION_HA][i].name);
                  if (integration_active_tokens[INTEGRATION_HA][i].name != NULL && strcmp(integration_active_tokens[INTEGRATION_HA][i].name, token->token) == 0)
                  {
                    snprintf(replacement, sizeof(replacement), "%s", integration_active_tokens[INTEGRATION_HA][i].value);
                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "HA %s=%s", token->token, integration_active_tokens[INTEGRATION_HA][i].value);
                    break;
                  }
                }
              }
            }
            break;

          case TOKEN_TYPE_STOCK:
            // Find the corresponding stock token by name
            if (integration_active_tokens[INTEGRATION_STOCK] != NULL)
            {
              for (int i = 0; i < integration_active_tokens_count[INTEGRATION_STOCK]; i++)
              {
                if (integration_active_tokens[INTEGRATION_STOCK][i].name != NULL && integration_active_tokens[INTEGRATION_STOCK][i].name[0] != '\0' && integration_active_tokens[INTEGRATION_STOCK][i].value[0] != '\0')
                {
                  if (integration_active_tokens[INTEGRATION_STOCK][i].name != NULL && strcmp(integration_active_tokens[INTEGRATION_STOCK][i].name, token->token) == 0)
                  {
                    snprintf(replacement, sizeof(replacement), "%s", integration_active_tokens[INTEGRATION_STOCK][i].value);
                    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Stock %s=%s", token->token, integration_active_tokens[INTEGRATION_STOCK][i].value);
                    break;
                  }
                }
              }
            }
            break;

          case TOKEN_TYPE_GLUCOSE:
            // Check if this is [CGM:reading] (plain) or [CGM:glucose] (formatted)
            // Format directly from glucose_data (no token arrays for Dexcom/Freestyle/Nightscout)
            if (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT])
            {
              if (glucose_data.current_gl_mgdl > 0)
              {
                if (strcmp(token->token, "[CGM:reading]") == 0)
                {
                  // Plain reading - just the number
                  get_glucose_reading_plain(replacement, sizeof(replacement));
                }
                else
                {
                  // Formatted reading - [CGM:glucose]
                  format_glucose_token(replacement, sizeof(replacement));
                }
              }
            }
            break;
          }

          // Copy the replacement to output with bounds checking
          int len = snprintf(dst, end - dst + 1, "%s", replacement);
          if (len > 0 && len < (end - dst + 1))
            dst += len;
          src += strlen(token->token);
          goto next_char;
        }
        token++;
      }
    }
    // If no token match, copy the character
    if (dst < end)
    {
      *dst++ = *src++;
    }
    else
    {
      src++; // Skip character if no space
    }
  next_char:
    continue;
  }

  // Ensure null termination
  if (dst <= end)
  {
    *dst = '\0';
  }
  else
  {
    output[output_size - 1] = '\0';
  }
}

// Function to cleanup tokens when no longer needed
void cleanup_tokens(void)
{
  if (prepared_tokens != NULL && prepared_tokens != base_tokens)
  {
    free((void *)prepared_tokens);
    prepared_tokens = NULL;
    prepared_tokens_count = 0;
  }
}

// Character width cache for Unicode code points
// Using a simple hash table approach for Unicode characters
#define CACHE_SIZE 512
static struct
{
  uint32_t code_point;
  uint8_t width;
} char_width_cache[CACHE_SIZE] = {0};
static const lv_font_t *cached_font = NULL;
static bool cache_valid = false;

// Function to initialize the character width cache
void init_char_width_cache(const lv_font_t *font)
{
  if (font == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "init_char_width_cache: null font");
    return;
  }

  // Check if cache is already valid for this font
  if (cache_valid && cached_font == font)
  {
    return; // Cache is already valid
  }

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Char width cache init");

  // Clear the cache (initialize with -1 to indicate empty, as 0 is a valid code point but not really used for widths)
  // Actually code_point 0 is space in many mappings, so let's use 0xFFFFFFFF for empty
  for (int i = 0; i < CACHE_SIZE; i++) {
    char_width_cache[i].code_point = 0xFFFFFFFF;
    char_width_cache[i].width = 0;
  }

  // Pre-calculate width for ASCII characters (0-127)
  for (int i = 0; i < 128; i++)
  {
    int cache_index = i % CACHE_SIZE;
    char_width_cache[cache_index].code_point = i;
    // We ignore kerning (letter_next) to keep cache hits consistent.
    // Frixos fonts currently don't use kerning tables.
    char_width_cache[cache_index].width = (uint8_t)lv_font_get_glyph_width(font, i, '\0');
  }

  // Cache common Unicode characters
  uint32_t common_chars[] = {0xB0, 0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7, 0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF};
  for (int i = 0; i < sizeof(common_chars) / sizeof(common_chars[0]); i++)
  {
    int cache_index = common_chars[i] % CACHE_SIZE;
    char_width_cache[cache_index].code_point = common_chars[i];
    char_width_cache[cache_index].width = (uint8_t)lv_font_get_glyph_width(font, common_chars[i], '\0');
  }

  cached_font = font;
  cache_valid = true;

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Char width cache ready");
}

// Function to get cached character width for Unicode code points
// Optimized with O(1) hash-style lookup to reduce CPU usage during scrolling message measurement.
static uint8_t get_cached_char_width(uint32_t code_point, const lv_font_t *font, const char *text, int text_pos)
{
  (void)text;
  (void)text_pos;

  if (!cache_valid)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "get_cached_char_width: not init");
    return 0;
  }

  // O(1) Lookup: Use modulo for direct indexing. Handle collisions by simple replacement.
  int cache_index = code_point % CACHE_SIZE;

  if (char_width_cache[cache_index].code_point == code_point)
  {
    return char_width_cache[cache_index].width;
  }

  // Cache miss: calculate, store and return.
  // We ignore kerning (letter_next) to keep cache hits consistent and avoid O(L^2) complexity from strlen/scans.
  // Frixos fonts currently don't use kerning tables.
  uint8_t width = (uint8_t)lv_font_get_glyph_width(font, code_point, '\0');

  char_width_cache[cache_index].code_point = code_point;
  char_width_cache[cache_index].width = width;

  return width;
}

// Function to invalidate the cache (call when font changes)
void invalidate_char_width_cache(void)
{
  cache_valid = false;
  cached_font = NULL;
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Char width cache invalidated");
}

// UTF-8 decoder function
// Returns the Unicode code point and sets bytes_consumed to the number of bytes read
static uint32_t decode_utf8(const char *text, int text_len, int pos, int *bytes_consumed)
{
  if (pos >= text_len)
  {
    *bytes_consumed = 0;
    return 0;
  }

  uint8_t first_byte = (uint8_t)text[pos];

  if (first_byte < 0x80)
  {
    // ASCII character (1 byte)
    *bytes_consumed = 1;
    return first_byte;
  }
  else if ((first_byte & 0xE0) == 0xC0)
  {
    // 2-byte UTF-8 sequence
    if (pos + 1 < text_len && ((uint8_t)text[pos + 1] & 0xC0) == 0x80)
    {
      *bytes_consumed = 2;
      uint32_t code_point = ((first_byte & 0x1F) << 6) | (text[pos + 1] & 0x3F);
      // Validate range (0x80 to 0x7FF)
      if (code_point >= 0x80 && code_point <= 0x7FF)
      {
        return code_point;
      }
    }
  }
  else if ((first_byte & 0xF0) == 0xE0)
  {
    // 3-byte UTF-8 sequence
    if (pos + 2 < text_len &&
        ((uint8_t)text[pos + 1] & 0xC0) == 0x80 &&
        ((uint8_t)text[pos + 2] & 0xC0) == 0x80)
    {
      *bytes_consumed = 3;
      uint32_t code_point = ((first_byte & 0x0F) << 12) |
                            (((uint8_t)text[pos + 1] & 0x3F) << 6) |
                            (text[pos + 2] & 0x3F);
      // Validate range (0x800 to 0xFFFF, excluding surrogates)
      if (code_point >= 0x800 && code_point <= 0xFFFF &&
          (code_point < 0xD800 || code_point > 0xDFFF))
      {
        return code_point;
      }
    }
  }
  else if ((first_byte & 0xF8) == 0xF0)
  {
    // 4-byte UTF-8 sequence
    if (pos + 3 < text_len &&
        ((uint8_t)text[pos + 1] & 0xC0) == 0x80 &&
        ((uint8_t)text[pos + 2] & 0xC0) == 0x80 &&
        ((uint8_t)text[pos + 3] & 0xC0) == 0x80)
    {
      *bytes_consumed = 4;
      uint32_t code_point = ((first_byte & 0x07) << 18) |
                            (((uint8_t)text[pos + 1] & 0x3F) << 12) |
                            (((uint8_t)text[pos + 2] & 0x3F) << 6) |
                            (text[pos + 3] & 0x3F);
      // Validate range (0x10000 to 0x10FFFF)
      if (code_point >= 0x10000 && code_point <= 0x10FFFF)
      {
        return code_point;
      }
    }
  }

  // Invalid UTF-8 sequence - return replacement character
  *bytes_consumed = 1;

  return 0xFFFD; // Unicode replacement character
}

void display_string_substring(const char *text, int32_t x, int32_t y,
                              int32_t start_pixel, int32_t width_pixels,
                              lv_obj_t *label_obj, const lv_font_t *font)
{
  if (text == NULL || label_obj == NULL || font == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "display_string_substring: null ptr");
    return;
  }

  if (width_pixels <= 0)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "display_string_substring: invalid width");
    return;
  }

  // Initialize cache if needed
  init_char_width_cache(font);

  lvgl_port_lock(0);

  // Only set font if it changed (avoid unnecessary style updates)
  static const lv_font_t *last_font = NULL;
  if (last_font != font)
  {
    lv_obj_set_style_text_font(label_obj, font, 0);
    last_font = font;
  }

  int32_t text_len = strlen(text);

  // Early exit if text is empty
  if (text_len == 0)
  {
    lv_label_set_text(label_obj, "");
    lv_obj_set_pos(label_obj, x, y);
    lvgl_port_unlock();
    return;
  }

  // Handle negative start_pixel by adding padding
  int32_t effective_start_pixel = start_pixel;
  int32_t padding_offset = 0;
  if (start_pixel < 0)
  {
    effective_start_pixel = 0;
    padding_offset = -start_pixel;
  }

  // Calculate which characters to display based on pixel positions
  // Optimized: Single pass to find both start and end character indices,
  // reducing redundant UTF-8 decoding and character width lookups.
  int32_t current_pixel = 0;
  int32_t start_char_index = 0;
  int32_t end_char_index = text_len;
  int32_t char_offset = 0;
  int32_t end_pixel = effective_start_pixel + width_pixels;
  bool start_found = false;

  for (int i = 0; i < text_len; )
  {
    int bytes_consumed;
    uint32_t code_point = decode_utf8(text, text_len, i, &bytes_consumed);

    if (bytes_consumed == 0)
    {
      break; // End of string
    }

    uint8_t char_width = get_cached_char_width(code_point, font, text, i);

    // Find the starting character (first character that contains effective_start_pixel)
    if (!start_found && current_pixel + char_width > effective_start_pixel)
    {
      start_char_index = i;
      char_offset = effective_start_pixel - current_pixel;
      start_found = true;
    }

    // Find the ending character (last character that contains effective_start_pixel + width_pixels)
    if (current_pixel + char_width > end_pixel)
    {
      end_char_index = i;
      break; // Found both indices, can stop scanning
    }

    current_pixel += char_width;
    i += bytes_consumed;
  }

  // Create substring with only the characters that will be visible
  int32_t substring_len = end_char_index - start_char_index;
  if (substring_len <= 0)
  {
    // No characters to display
    lv_label_set_text(label_obj, "");
    lv_obj_set_pos(label_obj, x, y);
    lvgl_port_unlock();
    return;
  }

  // Use static buffer (128 characters max)
  static char substring_buffer[128];
  if (substring_len >= sizeof(substring_buffer))
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "display_string_substring: truncated");
    substring_len = sizeof(substring_buffer) - 1;
  }

  // Copy only the characters that will be visible
  if (substring_len > 0)
  {
    memcpy(substring_buffer, text + start_char_index, substring_len);
    substring_buffer[substring_len] = '\0';
  }

  // Set only the substring text if it has changed
  // Optimization: use strcmp to avoid redundant LVGL updates during scrolling
  if (strcmp(lv_label_get_text(label_obj), substring_buffer) != 0)
  {
    lv_label_set_text(label_obj, substring_buffer);
  }

  // Cache position to avoid unnecessary LVGL calls
  static int32_t last_x = -1, last_y = -1, last_width = -1;
  int32_t new_x = x - char_offset + padding_offset;

  if (last_x != new_x || last_y != y)
  {
    lv_obj_set_pos(label_obj, new_x, y);
    last_x = new_x;
    last_y = y;
  }

  // Set the width to limit what's visible
  if (last_width != width_pixels)
  {
    lv_obj_set_width(label_obj, width_pixels);
    last_width = width_pixels;
  }

  lvgl_port_unlock();
}
