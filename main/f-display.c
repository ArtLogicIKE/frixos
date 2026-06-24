//
// display-related functions
//
// This module handles:
// - Display initialization and management
// - Time display with digit sprites
// - Weather and moon phase display
// - Message scrolling and centering (messages that fit in the centered area are static, otherwise scroll)
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
#include "f-graph.h"

#include "time.h"
#include "math.h"
#include <stdlib.h>
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_lcd_st7735.h"
#include <unistd.h>
#include "lvgl.h"
#include "draw/lv_image_decoder.h"
#include "draw/lv_draw_buf.h"

/* Bitmap fonts have unused rows above glyphs inside line_height; trim bg only, not text.
 * Keep in sync with SCREEN_LABEL_BG_TOP_TRIM in spiffs/index.js. */
#define LABEL_BG_TOP_TRIM 2

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
         *img_digits_sprite_aux = NULL,
         *img_weather = NULL,
         *img_moon = NULL,
         *img_ampm = NULL,
         *img_logo = NULL,
         *img_glucose = NULL,
         *img_glucose_trend = NULL,
         *img_wifi = NULL,
         *img_mgdl = NULL,
         *img_mgdl_aux = NULL;

lv_obj_t *label_msg = NULL;
lv_obj_t *label_msg_loop = NULL; // second label for seamless infinite scrolling
lv_obj_t *label_static[SCREEN_STATIC_TEXT_COUNT] = {NULL};

// User JPG icons: one lv_image per slot, pointed at S:/icon<N>.jpg. Static images
// kept in flash and decoded on draw (LV cache is 0), so no bitmap is held in RAM.
// icon_src_ok[i] caches whether icon<i+1>.jpg exists (refreshed in display_changed)
// so the hot visibility path doesn't stat SPIFFS every loop.
lv_obj_t *img_icon[SCREEN_ICON_COUNT] = {NULL};
static bool icon_src_ok[SCREEN_ICON_COUNT] = {false};

// Generic graph widget canvas (RGB565). The buffer is lazily (re)allocated at
// the configured width x height to keep heap pressure low — important when the
// graph coincides with a heavy TLS fetch (e.g. CGM). Realloc only happens on
// the display task under the LVGL lock, so it never races the renderer.
lv_obj_t *graph_canvas = NULL;
static uint8_t *graph_canvas_buf = NULL;
static size_t graph_canvas_buf_size = 0;
// Set by display_changed() so the next update_graph() redraws immediately on a
// config edit instead of waiting for the next sample.
static bool graph_force_redraw = false;
lv_obj_t *label_digit = NULL;
lv_obj_t *label_digit_aux = NULL;
static lv_obj_t *label_degree = NULL;
static lv_obj_t *label_degree_aux = NULL;

// Define digit width & height (adjust based on actual sprite sheet)
#define DIGIT_WIDTH 18          // Width of each digit in the sprite sheet
#define DIGIT_HEIGHT 36         // Height of each digit
#define SPRITE_SHEET_COLUMNS 10 // Number of digits in the sprite sheet
/** Free pixels between decimal dot and adjacent digit glyphs (mmol/L glucose display). */
#define MMOL_DECIMAL_SIDE_GAP_PX 1
/** If LVGL has not laid out the dot image yet, assume this width (typical theme dot). */
#define MMOL_DOT_WIDTH_FALLBACK_PX 4
/** Gap between the right edge of digit slot 2 and the glucose unit label (standalone widget). */
#define GLUCOSE_WIDGET_UNIT_GAP_AFTER_DIGIT2 3
/** Trim leading space for standalone mmol/L glucose (pixels). */
#define GLUCOSE_WIDGET_MMOL_LEADING_TRIM_PX 2

#define NUM_DIGITS 4                   // the 4 time digits
#define label_msg_ofs_y (25 + 25 + 14) // y offset for the message label
#define MSG_WIDTH 128                  // width of the message area
#define MSG_EXTRA_WIDTH 0            // extra width for the message area, useful for scolling but bad for centering
#define MSG_CENTER_WIDTH (MSG_WIDTH - MSG_EXTRA_WIDTH) // visible width when message is centered
#define FADE_STEPS 14
#define FADE_INTERVAL 200       // Time between steps in ms
#define MAX_TOKEN_COUNT 100     // Maximum number of tokens to prevent memory issues
#define MAX_FONT_INDEX 5        // Maximum font index (0=8pt .. 4=16pt, 5=5pt tiny)
#define MIN_ANIMATION_TIME 1000 // Minimum animation time in milliseconds

int label_scroll_pos = 0, label_max_pos = MSG_WIDTH;
bool label_scroll_swap = false; // which physical label is leftmost in the scroll belt

// Time-based scroll belt state (suggestion #1: position from elapsed wall-clock
// time; suggestion #2: sampled every fast loop pass, redraw only on pixel change).
#define SCROLL_LOOP_MS 20          // fixed, fast display-loop period, decoupled from scroll speed
static int64_t scroll_t0_us = 0;   // time origin of the current message
static int scroll_seed_px = 0;     // belt offset (px) at scroll_t0_us
static int scroll_last_ipos = 0;   // last integer offset applied to the labels
static bool scroll_last_valid = false; // false => force a reposition on the next pass
lv_obj_t *digit_objs[NUM_DIGITS] = {NULL, NULL, NULL, NULL};
lv_obj_t *dots[2] = {NULL, NULL};
lv_obj_t *aux_digit_objs[NUM_DIGITS] = {NULL, NULL, NULL, NULL};
lv_obj_t *aux_dots[2] = {NULL, NULL};

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;

int last_minute = -1;      // Store the last updated minute
time_t ota_start_time = 0; // Track when OTA update started
time_t now;
struct tm timeinfo;

typedef struct
{
  const display_slot_t *schedule;
  int count;
  int current_idx;
  time_t slot_start;
  bool alternate_active;
  bool showing_glucose;
  bool showing_weather;
  bool showing_ha;
} schedule_runner_t;

static schedule_runner_t primary_runner;
static schedule_runner_t aux_runner;

static void sync_schedule_runners(void);
static bool layout_show_weather_on_digits(const screen_layout_profile_t *layout,
                                          const schedule_runner_t *runner,
                                          screen_element_id_t elem);
static bool layout_show_ha_on_digits(const screen_layout_profile_t *layout,
                                     const schedule_runner_t *runner,
                                     screen_element_id_t elem);
static bool layout_show_time_digits(const screen_layout_profile_t *layout,
                                    const schedule_runner_t *runner,
                                    screen_element_id_t elem);
static bool layout_show_glucose_on_digits(const screen_layout_profile_t *layout,
                                          const schedule_runner_t *runner,
                                          screen_element_id_t elem);

// fader stuff
static int fade_step = 0;
static bool fading_in = true;                // Flag to track direction
static esp_timer_handle_t fade_timer = NULL; // ESP timer handle
static bool fade_update_needed = false;      // Flag to indicate fade update is needed
static int label_size = 0;
static char last_scroll_msg[SCROLL_MSG_LENGTH];
static uint8_t last_scroll_font = 0xFF; // font index active when last_scroll_msg was set

LV_FONT_DECLARE(frixos_5);    // Tiny5 pixel font, full ASCII, label font index 5 ("5pt")
LV_FONT_DECLARE(frixos_8);
LV_FONT_DECLARE(graph_tiny5); // 5px pixel font (Tiny5) subset for graph axis labels
LV_FONT_DECLARE(frixos_10);
LV_FONT_DECLARE(frixos_11);
LV_FONT_DECLARE(frixos_12);
LV_FONT_DECLARE(frixos_14);
LV_FONT_DECLARE(frixos_16);

static void fade_timer_cb(void *arg);
void create_grid(lv_obj_t *scr);
void show_object(lv_obj_t *obj, bool show);

// Helper functions for modular display_task
static void handle_als_and_brightness(uint32_t loop_counter);
static void handle_wifi_status_icon(void);
static void handle_screen_layout_on_wifi(void);
static void handle_integration_and_messages(void);
static void update_static_text_labels(void);
static void apply_screen_layout_positions(void);
static void apply_screen_layout_z_order(const screen_layout_profile_t *layout);
static uint8_t screen_scroll_delay_ms(void);
static void handle_alternate_mode_switching(time_t now, uint32_t loop_counter, bool *should_update_display);
static void update_display_content(time_t now);
static inline const screen_widget_t *active_screen_widget(screen_element_id_t id)
{
  return &eeprom_screen_layout.profile[font_index].widget[id];
}
static inline int layout_abs_x(const screen_widget_t *w)
{
  return w->x;
}
static inline int layout_abs_y(const screen_widget_t *w)
{
  return w->y;
}

/* Scroll message position before layout coordinates are applied (pre-WiFi). */
#define BOOT_MSG_X 0
#define BOOT_MSG_Y 86

static bool screen_layout_positions_live = false;
static inline uint8_t active_message_font_index(void)
{
  const screen_widget_t *w_msg = active_screen_widget(SCREEN_ELEM_MESSAGE);
  return w_msg->font <= MAX_FONT_INDEX ? w_msg->font : 0;
}

static uint8_t screen_scroll_delay_ms(void)
{
  return eeprom_screen_layout.scroll_delay > 0 ? eeprom_screen_layout.scroll_delay : eeprom_scroll_delay;
}

static void text_label_draw_trim_bg(lv_event_t *e)
{
  if (lv_event_get_code(e) != LV_EVENT_DRAW_MAIN_BEGIN)
    return;

  lv_obj_t *label = lv_event_get_current_target(e);
  lv_layer_t *layer = lv_event_get_layer(e);
  lv_area_t bg_coords;
  lv_obj_get_coords(label, &bg_coords);
  bg_coords.y1 += LABEL_BG_TOP_TRIM;
  if (bg_coords.y1 > bg_coords.y2)
    return;

  lv_draw_rect_dsc_t draw_dsc;
  lv_draw_rect_dsc_init(&draw_dsc);
  draw_dsc.base.layer = layer;
  draw_dsc.bg_color = lv_obj_get_style_bg_color_filtered(label, LV_PART_MAIN);
  draw_dsc.bg_opa = LV_OPA_COVER;
  draw_dsc.radius = lv_obj_get_style_radius(label, LV_PART_MAIN);
  lv_draw_rect(layer, &draw_dsc, &bg_coords);
}

static lv_obj_t *trim_bg_labels[SCREEN_STATIC_TEXT_COUNT + 4];
static int trim_bg_labels_count = 0;

static bool label_has_trim_bg_draw(lv_obj_t *label)
{
  for (int i = 0; i < trim_bg_labels_count; i++)
  {
    if (trim_bg_labels[i] == label)
      return true;
  }
  return false;
}

static void register_label_trim_bg_draw(lv_obj_t *label)
{
  if (label == NULL || label_has_trim_bg_draw(label))
    return;
  if (trim_bg_labels_count < (int)(sizeof(trim_bg_labels) / sizeof(trim_bg_labels[0])))
    trim_bg_labels[trim_bg_labels_count++] = label;
  lv_obj_add_event_cb(label, text_label_draw_trim_bg, LV_EVENT_DRAW_MAIN_BEGIN, NULL);
}

static void apply_text_widget_background(lv_obj_t *label, const screen_widget_t *w, uint8_t font_idx)
{
  if (font_idx > MAX_FONT_INDEX)
    font_idx = 0;
  lv_obj_set_style_pad_all(label, 0, LV_PART_MAIN);
  lv_obj_set_height(label, get_selected_font_height(font_idx));
  lv_obj_set_style_bg_color(label, lv_color_make(w->bg_r, w->bg_g, w->bg_b), LV_PART_MAIN);
  /* Default widget bg is suppressed; trimmed bg is drawn in text_label_draw_trim_bg. */
  lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN);
}

static void apply_message_widget_styles(const screen_widget_t *w)
{
  if (label_msg == NULL || w == NULL)
    return;

  lv_color_t msg_color = lv_color_make(w->color_r, w->color_g, w->color_b);
  lv_obj_set_style_text_color(label_msg, msg_color, 0);
  apply_text_widget_background(label_msg, w, active_message_font_index());
  if (label_msg_loop != NULL)
  {
    lv_obj_set_style_text_color(label_msg_loop, msg_color, 0);
    apply_text_widget_background(label_msg_loop, w, active_message_font_index());
  }
}

static void apply_static_text_widget(lv_obj_t *label, const screen_widget_t *w, const char *text)
{
  const uint8_t font_idx = w->font <= MAX_FONT_INDEX ? w->font : 0;
  lv_obj_set_style_text_font(label, get_selected_font(font_idx), 0);
  lv_obj_set_style_text_color(label, lv_color_make(w->color_r, w->color_g, w->color_b), 0);
  apply_text_widget_background(label, w, font_idx);

  int label_w = w->width > 0 ? w->width : (LCD_H_RES - layout_abs_x(w));
  if (label_w < 1)
    label_w = 1;
  lv_obj_set_width(label, label_w);
  lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);

  lv_text_align_t align = LV_TEXT_ALIGN_LEFT;
  if (w->align == SCREEN_MSG_ALIGN_CENTER)
    align = LV_TEXT_ALIGN_CENTER;
  else if (w->align == SCREEN_MSG_ALIGN_RIGHT)
    align = LV_TEXT_ALIGN_RIGHT;
  lv_obj_set_style_text_align(label, align, 0);
  if (screen_layout_positions_live)
    lv_obj_set_pos(label, layout_abs_x(w), layout_abs_y(w));
  lv_label_set_text(label, text);
}

static void update_static_text_labels(void)
{
  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];

  for (int i = 0; i < SCREEN_STATIC_TEXT_COUNT; i++)
  {
    if (label_static[i] == NULL)
      continue;

    const screen_widget_t *w = &layout->widget[SCREEN_ELEM_TEXT_1 + i];
    char rendered[SCREEN_STATIC_TEXT_LENGTH];
    replace_placeholders(layout->static_text[i], rendered, sizeof(rendered));
    apply_static_text_widget(label_static[i], w, rendered);
    show_object(label_static[i], w->enabled != 0 && time_valid);
  }
}

int check_file(char *filename); // defined later; used by update_icon_srcs

// Point each icon image at S:/icon<N>.jpg if that file exists, caching the result
// in icon_src_ok[] so the visibility path doesn't stat SPIFFS every loop. Called
// from display_changed() (boot, layout/theme change, and after an icon upload +
// re-apply). The JPG stays in flash; LVGL decodes it on draw (cache is 0).
static void update_icon_srcs(void)
{
  char path[32];
  char src[24];
  for (int i = 0; i < SCREEN_ICON_COUNT; i++)
  {
    if (img_icon[i] == NULL)
      continue;
    snprintf(path, sizeof(path), "/spiffs/icon%d.jpg", i + 1);
    if (check_file(path))
    {
      snprintf(src, sizeof(src), "S:/icon%d.jpg", i + 1);
      lv_image_set_src(img_icon[i], src);
      lv_image_set_inner_align(img_icon[i], LV_ALIGN_TOP_LEFT);
      icon_src_ok[i] = true;
    }
    else
    {
      icon_src_ok[i] = false;
      show_object(img_icon[i], false);
    }
  }
}

static void update_digit_label_widget(lv_obj_t *label,
                                      const screen_widget_t *w,
                                      const schedule_runner_t *runner,
                                      screen_element_id_t pair_elem,
                                      const char *configured_text)
{
  if (label == NULL || w->enabled == 0 || !time_valid)
  {
    if (label)
      show_object(label, false);
    return;
  }

  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];
  const bool show_time = layout_show_time_digits(layout, runner, pair_elem);
  const bool show_glucose = layout_show_glucose_on_digits(layout, runner, pair_elem);
  const bool show_weather = layout_show_weather_on_digits(layout, runner, pair_elem);
  const bool show_ha = layout_show_ha_on_digits(layout, runner, pair_elem);
  const bool slot_named = (show_time || show_glucose || show_weather || show_ha)
                          && runner->count > 0
                          && runner->schedule[runner->current_idx].name[0] != '\0';

  char rendered[SCREEN_STATIC_TEXT_LENGTH];
  const char *display_text = configured_text;

  if (slot_named)
    display_text = runner->schedule[runner->current_idx].name;
  else
  {
    replace_placeholders(configured_text, rendered, sizeof(rendered));
    display_text = rendered;
  }

  apply_static_text_widget(label, w, display_text);
  show_object(label, true);
}

static void update_digit_label_widgets(void)
{
  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];

  sync_schedule_runners();
  update_digit_label_widget(label_digit,
                            &layout->widget[SCREEN_ELEM_DIGIT_LABEL],
                            &primary_runner,
                            SCREEN_ELEM_TIME,
                            layout->digit_label_text);
  update_digit_label_widget(label_digit_aux,
                            &layout->widget[SCREEN_ELEM_DIGIT_LABEL_AUX],
                            &aux_runner,
                            SCREEN_ELEM_TIME_AUX,
                            layout->digit_label_aux_text);
}
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
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER,
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

// The calibration grid is drawn as a screen-level overlay, NOT as a set of
// lv_line objects. On this heap-constrained device the min free heap can dip to
// ~1 KB during TLS handshakes (weather/CGM); the previous ~14 line objects'
// persistent object/style allocations were enough to tip the system into OOM
// and reset it once the graph/CGM/weather paths were active. A draw callback
// costs zero persistent heap, and because it runs on LV_EVENT_DRAW_POST of the
// screen (after all children are rendered) it is always on top.

static bool grid_cb_registered = false;

// Reference grid line positions in 128x128 panel space. Vertical lines are red,
// horizontal lines green, with a green centre cross at x=64 / y=64.
static const int32_t grid_v_lines[] = {20, 30, 40, 90, 100, 110};
static const int32_t grid_h_lines[] = {20, 30, 40, 90, 100, 110};

static void grid_draw_event_cb(lv_event_t *e)
{
  if (!eeprom_show_grid)
    return;

  lv_layer_t *layer = lv_event_get_layer(e);

  lv_draw_line_dsc_t dsc;
  lv_draw_line_dsc_init(&dsc);
  dsc.width = 1;
  dsc.opa = LV_OPA_COVER;

  // Vertical lines (red).
  dsc.color = lv_color_make(0xFF, 0x00, 0x00);
  for (size_t i = 0; i < sizeof(grid_v_lines) / sizeof(grid_v_lines[0]); i++)
  {
    dsc.p1.x = grid_v_lines[i]; dsc.p1.y = 0;
    dsc.p2.x = grid_v_lines[i]; dsc.p2.y = 128;
    lv_draw_line(layer, &dsc);
  }

  // Horizontal lines + centre cross (green).
  dsc.color = lv_color_make(0x00, 0xFF, 0x00);
  for (size_t i = 0; i < sizeof(grid_h_lines) / sizeof(grid_h_lines[0]); i++)
  {
    dsc.p1.x = 0;   dsc.p1.y = grid_h_lines[i];
    dsc.p2.x = 128; dsc.p2.y = grid_h_lines[i];
    lv_draw_line(layer, &dsc);
  }
  dsc.p1.x = 64; dsc.p1.y = 0;   dsc.p2.x = 64;  dsc.p2.y = 128; // centre V
  lv_draw_line(layer, &dsc);
  dsc.p1.x = 0;  dsc.p1.y = 64;  dsc.p2.x = 128; dsc.p2.y = 64;  // centre H
  lv_draw_line(layer, &dsc);
}

// Retained for source compatibility with the old object-based grid: the draw
// callback already renders on top of all children, so nothing to re-raise.
static void raise_grid_foreground(void) {}

void create_grid(lv_obj_t *scr)
{
  if (grid_cb_registered)
    return;
  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Register grid overlay");
  lv_obj_add_event_cb(scr, grid_draw_event_cb, LV_EVENT_DRAW_POST, NULL);
  grid_cb_registered = true;
}

void show_grid(uint8_t show)
{
  // eeprom_show_grid is the source of truth read by the draw callback; just make
  // sure the overlay callback is installed, then invalidate so the toggle shows.
  (void)show;
  create_grid(lv_scr_act());
  lv_obj_invalidate(lv_scr_act());
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
  uint8_t msg_font = active_message_font_index();
  // Validate font index bounds
  if (msg_font > MAX_FONT_INDEX)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "set_scroll_message: invalid font %d, using default", msg_font);
    msg_font = 0;
  }
  // Get the selected font and validate it
  const lv_font_t *font = get_selected_font(msg_font);
  if (font == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_WARN, TAG, "set_scroll_message: invalid font, using default");
    font = &frixos_8; // Use default font
  }

  // Measure new text width
  lv_point_t size;
  lv_text_get_size(&size, msg, font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

  // If the text content AND font are unchanged, preserve the in-progress
  // scroll state so the belt never resets to the right edge mid-cycle.
  bool content_changed = (strcmp(msg, last_scroll_msg) != 0) || (msg_font != last_scroll_font);

  strncpy(last_scroll_msg, msg, sizeof(last_scroll_msg) - 1);
  last_scroll_msg[sizeof(last_scroll_msg) - 1] = '\0';
  last_scroll_font = msg_font;

  lvgl_port_lock(0);
  lv_obj_set_style_text_font(label_msg, font, 0);
  lv_obj_set_style_text_font(label_msg_loop, font, 0);
  lv_label_set_text(label_msg, msg);
  lv_label_set_text(label_msg_loop, msg);
  label_size = size.x;
  const screen_widget_t *w_msg = active_screen_widget(SCREEN_ELEM_MESSAGE);
  apply_message_widget_styles(w_msg);
  const int msg_x = screen_layout_positions_live ? layout_abs_x(w_msg) : BOOT_MSG_X;
  const int msg_y = screen_layout_positions_live ? layout_abs_y(w_msg) : BOOT_MSG_Y;

  if (label_size > MSG_CENTER_WIDTH)
  { // scrolling, left aligned
    lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_align(label_msg_loop, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(label_msg, LV_SIZE_CONTENT);
    lv_obj_set_width(label_msg_loop, LV_SIZE_CONTENT);
    label_max_pos = label_size;
    if (content_changed)
    {
      // New text or font: reset the time origin and enter from the right edge.
      const int scroll_period = label_max_pos + 20;
      scroll_seed_px = MSG_WIDTH;
      scroll_t0_us = esp_timer_get_time();
      int ipos = scroll_seed_px % scroll_period;
      if (ipos > 0) ipos -= scroll_period; // normalize into (-period, 0]
      scroll_last_ipos = ipos;
      scroll_last_valid = true;
      lv_obj_set_pos(label_msg,      msg_x + ipos,                msg_y);
      lv_obj_set_pos(label_msg_loop, msg_x + ipos + scroll_period, msg_y);
    }
    // else: same content, same font — belt keeps scrolling uninterrupted.
    lv_obj_clear_flag(label_msg_loop, LV_OBJ_FLAG_HIDDEN);
  }
  else
  { // centered
    const int centered_label_width = MSG_CENTER_WIDTH;

    lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_msg, centered_label_width);
    lv_obj_set_pos(label_msg, msg_x, msg_y);
    lv_obj_add_flag(label_msg_loop, LV_OBJ_FLAG_HIDDEN);
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
  graph_init(); // graph ring mutex + zeroed state, before any sampler/render
  lv_obj_t *scr = lv_scr_act();

  lvgl_port_lock(0);
  st7735_set_rotation_and_mirror(eeprom_rotation, eeprom_mirroring);
  show_grid(eeprom_show_grid); // will create the grid if needed

  // Set the background color to BLACK
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); // (R,G,B)
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  img_logo = lv_image_create(lv_scr_act());
  lv_image_set_src(img_logo, "S:/logo.jpg");
  lv_image_set_inner_align(img_logo, LV_ALIGN_CENTER);
  lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, 42, 32);

  label_msg = lv_label_create(lv_scr_act());
  label_msg_loop = lv_label_create(lv_scr_act());
  if (label_msg == NULL || label_msg_loop == NULL)
  {
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Failed to create message labels");
    return;
  }
  lv_label_set_long_mode(label_msg, LV_LABEL_LONG_CLIP);
  lv_label_set_long_mode(label_msg_loop, LV_LABEL_LONG_CLIP);
  lv_obj_set_width(label_msg, LV_SIZE_CONTENT);
  lv_obj_set_width(label_msg_loop, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(label_msg, get_selected_font(active_message_font_index()), 0);
  lv_obj_set_style_text_font(label_msg_loop, get_selected_font(active_message_font_index()), 0);
  lv_obj_set_style_text_align(label_msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_align(label_msg_loop, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(label_msg, BOOT_MSG_X, BOOT_MSG_Y);
  lv_obj_add_flag(label_msg_loop, LV_OBJ_FLAG_HIDDEN); // hidden until scrolling active

  // Optimize label for smooth scrolling
  // lv_obj_set_style_anim_speed(label_msg, 150, LV_PART_MAIN); // Optimized animation speed for smoothness
  // lv_obj_set_style_anim_time(label_msg, 1000, LV_PART_MAIN); // Base animation time

  set_scroll_message("Starting...");

  for (int i = 0; i < SCREEN_STATIC_TEXT_COUNT; i++)
  {
    label_static[i] = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label_static[i], LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(label_static[i], LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(label_static[i], LV_OPA_COVER, LV_PART_MAIN);
  }

  label_digit = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label_digit, LV_LABEL_LONG_CLIP);
  lv_obj_add_flag(label_digit, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(label_digit, LV_OPA_COVER, LV_PART_MAIN);
  label_digit_aux = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label_digit_aux, LV_LABEL_LONG_CLIP);
  lv_obj_add_flag(label_digit_aux, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_bg_opa(label_digit_aux, LV_OPA_COVER, LV_PART_MAIN);

  img_digits_sprite = lv_image_create(lv_scr_act());
  img_weather = lv_image_create(lv_scr_act());
  img_moon = lv_image_create(lv_scr_act());
  img_glucose = lv_image_create(lv_scr_act());
  img_glucose_trend = lv_image_create(lv_scr_act());
  img_wifi = lv_image_create(lv_scr_act());

  // Generic graph: create the canvas object but DON'T allocate its ~10.8 KB
  // RGB565 buffer yet. The buffer is allocated lazily the first time a graph
  // element is enabled (update_graph), so devices without a graph pay nothing.
  graph_canvas = lv_canvas_create(lv_scr_act());
  lv_obj_add_flag(graph_canvas, LV_OBJ_FLAG_HIDDEN);
  img_mgdl = lv_image_create(lv_scr_act());
  img_mgdl_aux = lv_image_create(lv_scr_act());
  img_ampm = lv_image_create(lv_scr_act()); // AMPM indicator
  label_degree = lv_label_create(lv_scr_act());
  lv_label_set_text(label_degree, "\xc2\xb0");
  lv_obj_set_style_text_color(label_degree, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(label_degree, &frixos_11, 0);
  lv_obj_add_flag(label_degree, LV_OBJ_FLAG_HIDDEN);
  label_degree_aux = lv_label_create(lv_scr_act());
  lv_label_set_text(label_degree_aux, "\xc2\xb0");
  lv_obj_set_style_text_color(label_degree_aux, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(label_degree_aux, &frixos_11, 0);
  lv_obj_add_flag(label_degree_aux, LV_OBJ_FLAG_HIDDEN);
  for (int i = 0; i < 4; i++)
  {
    digit_objs[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(digit_objs[i], LV_OBJ_FLAG_HIDDEN);
    aux_digit_objs[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(aux_digit_objs[i], LV_OBJ_FLAG_HIDDEN);
  }
  for (int i = 0; i < 2; i++)
  {
    dots[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(dots[i], LV_OBJ_FLAG_HIDDEN);
    aux_dots[i] = lv_image_create(lv_scr_act());
    lv_obj_add_flag(aux_dots[i], LV_OBJ_FLAG_HIDDEN);
  }
  for (int i = 0; i < SCREEN_ICON_COUNT; i++)
  {
    img_icon[i] = lv_image_create(lv_scr_act());
    lv_image_set_inner_align(img_icon[i], LV_ALIGN_TOP_LEFT);
    lv_obj_add_flag(img_icon[i], LV_OBJ_FLAG_HIDDEN);
  }

  // hide objects
  lv_obj_add_flag(img_digits_sprite, LV_OBJ_FLAG_HIDDEN);
  img_digits_sprite_aux = lv_image_create(lv_scr_act());
  lv_obj_add_flag(img_digits_sprite_aux, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_moon, LV_OBJ_FLAG_HIDDEN);          // Hide
  lv_obj_add_flag(img_ampm, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_weather, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_glucose, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_glucose_trend, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_wifi, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_mgdl, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_mgdl_aux, LV_OBJ_FLAG_HIDDEN);

  register_label_trim_bg_draw(label_msg);
  for (int i = 0; i < SCREEN_STATIC_TEXT_COUNT; i++)
    register_label_trim_bg_draw(label_static[i]);
  register_label_trim_bg_draw(label_digit);
  register_label_trim_bg_draw(label_digit_aux);

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
      &frixos_8,  // index 0: 8pt (bitmap)
      &frixos_10, // index 1: 10pt (bitmap)
      &frixos_12, // index 2: 12pt (bitmap)
      &frixos_14, // index 3: 14pt (bitmap)
      &frixos_16, // index 4: 16pt (bitmap)
      &frixos_5,  // index 5: 5pt (Tiny5 pixel font, full ASCII)
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
  const lv_font_t *font = get_selected_font(font_index);
  if (!font)
    return 8;
  return (uint8_t)lv_font_get_line_height(font);
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

/* Return cached HA value string for the given entity, or NULL if not available. */
static const char *get_ha_slot_value(const char *entity)
{
  if (!entity || entity[0] == '\0') return NULL;
  for (int i = 0; i < integration_active_tokens_count[INTEGRATION_HA]; i++)
  {
    integration_token_t *tok = &integration_active_tokens[INTEGRATION_HA][i];
    if (tok->entity && strcmp(tok->entity, entity) == 0)
      return (strcmp(tok->value, "-") != 0 && tok->value[0] != '\0') ? tok->value : NULL;
  }
  return NULL;
}

/* Return true if the given slot currently has data available to display. */
static bool slot_is_available(const display_slot_t *s)
{
  switch (s->type)
  {
    case SLOT_TYPE_TIME:         return true;
    case SLOT_TYPE_CGM:          return is_glucose_on() && is_glucose_fresh();
    case SLOT_TYPE_WEATHER_TEMP: return last_weather_update > 0;
    case SLOT_TYPE_HA:           return get_ha_slot_value(s->entity) != NULL;
    default:                     return false;
  }
}

static bool layout_widget_enabled(const screen_layout_profile_t *layout, screen_element_id_t elem)
{
  return layout->widget[elem].enabled != 0;
}

static bool layout_show_weather_on_digits(const screen_layout_profile_t *layout,
                                          const schedule_runner_t *runner,
                                          screen_element_id_t elem)
{
  return layout_widget_enabled(layout, elem) && time_valid && runner->alternate_active &&
         runner->showing_weather && last_weather_update > 0;
}

static bool layout_show_ha_on_digits(const screen_layout_profile_t *layout,
                                     const schedule_runner_t *runner,
                                     screen_element_id_t elem)
{
  if (!layout_widget_enabled(layout, elem) || !time_valid || !runner->alternate_active || !runner->showing_ha)
    return false;
  if (runner->count == 0)
    return false;
  return get_ha_slot_value(runner->schedule[runner->current_idx].entity) != NULL;
}

static bool layout_show_time_digits(const screen_layout_profile_t *layout,
                                    const schedule_runner_t *runner,
                                    screen_element_id_t elem)
{
  if (!layout_widget_enabled(layout, elem) || !time_valid)
    return false;
  if (runner->alternate_active && (runner->showing_weather || runner->showing_ha || runner->showing_glucose))
    return false;
  return true;
}

static bool layout_show_glucose_on_digits(const screen_layout_profile_t *layout,
                                          const schedule_runner_t *runner,
                                          screen_element_id_t elem)
{
  return layout_widget_enabled(layout, elem) && time_valid && runner->alternate_active &&
         runner->showing_glucose && is_glucose_on() && is_glucose_fresh() &&
         glucose_data.current_gl_mgdl > 0;
}

typedef struct
{
  int digit[4];
  bool show_dots;
  bool is_mmol;
} digit_display_t;

static void fill_glucose_digits(digit_display_t *out, bool time_slot)
{
  out->show_dots = false;
  out->is_mmol = false;
  for (int i = 0; i < 4; i++)
    out->digit[i] = -1;

  if (!is_glucose_on() || glucose_data.current_gl_mgdl <= 0)
    return;

  if (eeprom_glucose_unit == 1)
  {
    float glucose_mmol = glucose_data.current_gl_mgdl / 18.0182f;
    if (glucose_mmol > 22.0f)
      glucose_mmol = 22.0f;
    if (glucose_mmol < 0.0f)
      glucose_mmol = 0.0f;
    int whole_part = (int)glucose_mmol;
    int decimal_part = (int)((glucose_mmol - whole_part) * 10.0f + 0.5f);
    if (decimal_part >= 10)
    {
      whole_part++;
      decimal_part = 0;
    }
    if (time_slot)
    {
      out->digit[0] = -1;
      if (whole_part >= 10)
      {
        out->digit[1] = whole_part / 10;
        out->digit[2] = whole_part % 10;
        out->digit[3] = decimal_part;
      }
      else
      {
        out->digit[1] = -1;
        out->digit[2] = whole_part;
        out->digit[3] = decimal_part;
      }
    }
    else if (whole_part >= 10)
    {
      out->digit[0] = whole_part / 10;
      out->digit[1] = whole_part % 10;
      out->digit[2] = decimal_part;
    }
    else
    {
      out->digit[1] = whole_part;
      out->digit[2] = decimal_part;
    }
    out->show_dots = true;
    out->is_mmol = true;
  }
  else
  {
    int glucose_value = (int)glucose_data.current_gl_mgdl;
    if (glucose_value > 999)
      glucose_value = 999;
    if (glucose_value < 0)
      glucose_value = 0;
    if (time_slot)
    {
      out->digit[0] = -1;
      if (glucose_value >= 100)
      {
        out->digit[1] = glucose_value / 100;
        out->digit[2] = (glucose_value / 10) % 10;
        out->digit[3] = glucose_value % 10;
      }
      else
      {
        out->digit[1] = -1;
        out->digit[2] = glucose_value / 10;
        out->digit[3] = glucose_value % 10;
      }
    }
    else if (glucose_value >= 100)
    {
      out->digit[0] = glucose_value / 100;
      out->digit[1] = (glucose_value / 10) % 10;
      out->digit[2] = glucose_value % 10;
    }
    else
    {
      out->digit[1] = glucose_value / 10;
      out->digit[2] = glucose_value % 10;
    }
  }
}

static void fill_time_digits(digit_display_t *out, struct tm *ti)
{
  out->show_dots = true;
  out->is_mmol = false;
  bool is_pm = false;
  struct tm local = *ti;

  if (eeprom_12hour)
  {
    if (local.tm_hour == 0)
      local.tm_hour = 12;
    else if (local.tm_hour > 12)
    {
      local.tm_hour -= 12;
      is_pm = true;
    }
  }

  out->digit[0] = local.tm_hour / 10;
  out->digit[1] = local.tm_hour % 10;
  out->digit[2] = local.tm_min / 10;
  out->digit[3] = local.tm_min % 10;
  if (!eeprom_show_leading_zero && out->digit[0] == 0)
    out->digit[0] = -1;
  (void)is_pm;
}

/* Fill a digit_display_t with an integer value (0..999), right-aligned into digit[1..3],
 * leaving digit[0] blank. Used for weather temperature and HA entity readouts. */
static void fill_integer_digits(digit_display_t *out, int value)
{
  out->show_dots = false;
  out->is_mmol = false;
  for (int i = 0; i < 4; i++)
    out->digit[i] = -1;

  if (value < 0) value = 0;
  if (value > 999) value = 999;

  if (value >= 100)
  {
    out->digit[1] = value / 100;
    out->digit[2] = (value / 10) % 10;
    out->digit[3] = value % 10;
  }
  else if (value >= 10)
  {
    out->digit[2] = value / 10;
    out->digit[3] = value % 10;
  }
  else
  {
    out->digit[3] = value;
  }
}

static void fill_weather_digits(digit_display_t *out)
{
  double display_temp = eeprom_fahrenheit ? ((weather_temp * 9.0 / 5.0) + 32.0) : weather_temp;
  int temp_val = (int)round(display_temp < 0 ? -display_temp : display_temp);
  fill_integer_digits(out, temp_val);
}

static void fill_ha_digits(digit_display_t *out, const char *ha_val)
{
  int ha_int = ha_val ? (int)round(fabs(atof(ha_val))) : 0;
  fill_integer_digits(out, ha_int);
}

static void set_digit_sprite(lv_obj_t *objs[NUM_DIGITS], int position, int digit)
{
  if (digit < -1 || digit > 9)
    return;
  lv_image_set_offset_x(objs[position], -digit * DIGIT_WIDTH);
}

static void align_time_style_digits(lv_obj_t *objs[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                    int ox, int y, const digit_display_t *dd, bool show_dots)
{
  lv_obj_align(objs[0], LV_ALIGN_TOP_LEFT, ox + 0, y);
  lv_obj_align(objs[1], LV_ALIGN_TOP_LEFT, ox + 1 * 18, y);
  lv_obj_align(objs[2], LV_ALIGN_TOP_LEFT, ox + 2 * 18 + 6, y);
  lv_obj_align(objs[3], LV_ALIGN_TOP_LEFT, ox + 3 * 18 + 6, y);
  show_object(dot_objs[0], show_dots);
  show_object(dot_objs[1], show_dots);
  lv_obj_align(dot_objs[0], LV_ALIGN_TOP_LEFT, ox + 2 * 18 + 1, y + 10);
  lv_obj_align(dot_objs[1], LV_ALIGN_TOP_LEFT, ox + 2 * 18 + 1, y + 26);
}

static void align_glucose_mgdl_digits(lv_obj_t *objs[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                      int ox, int y, const digit_display_t *dd)
{
  lv_obj_align(objs[0], LV_ALIGN_TOP_LEFT, ox + 0, y);
  lv_obj_align(objs[1], LV_ALIGN_TOP_LEFT, ox + 1 * 18 + 6, y);
  lv_obj_align(objs[2], LV_ALIGN_TOP_LEFT, ox + 2 * 18 + 6, y);
  lv_obj_align(objs[3], LV_ALIGN_TOP_LEFT, ox + 3 * 18 + 6, y);
  show_object(dot_objs[0], false);
  show_object(dot_objs[1], false);
}

static void align_glucose_mmol_digits(lv_obj_t *objs[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                      int ox, int y, const digit_display_t *dd)
{
  int dot_w = lv_obj_get_width(dot_objs[0]);
  if (dot_w <= 0)
    dot_w = MMOL_DOT_WIDTH_FALLBACK_PX;

  const int g = MMOL_DECIMAL_SIDE_GAP_PX;
  const int shift = g + dot_w;
  const int x0 = ox + 0 - shift;
  const int x1 = ox + 1 * 18 + 6 - shift;
  const int x2 = ox + 2 * 18 + 6 - shift;
  const int x3 = x2 + DIGIT_WIDTH + g + dot_w + g;

  lv_obj_align(objs[0], LV_ALIGN_TOP_LEFT, x0, y);
  lv_obj_align(objs[1], LV_ALIGN_TOP_LEFT, x1, y);
  lv_obj_align(objs[2], LV_ALIGN_TOP_LEFT, x2, y);
  lv_obj_align(objs[3], LV_ALIGN_TOP_LEFT, x3, y);
  show_object(dot_objs[0], true);
  show_object(dot_objs[1], false);
  lv_obj_align(dot_objs[0], LV_ALIGN_TOP_LEFT, x2 + DIGIT_WIDTH + g, y + 30);
}

static void align_glucose_digits(lv_obj_t *objs[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                 int ox, int y, const digit_display_t *dd)
{
  if (dd->is_mmol)
    align_glucose_mmol_digits(objs, dot_objs, ox, y, dd);
  else
    align_glucose_mgdl_digits(objs, dot_objs, ox, y, dd);
}

/* Lay out the significant digits of a weather/HA value left-to-right starting at ox,
 * followed by an optional unit label (degree symbol or HA unit). Anchored to the time
 * widget so it follows the user's layout. */
static void align_value_with_label_digits(lv_obj_t *objs[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                          int ox, int y, const digit_display_t *dd,
                                          const char *unit, lv_obj_t *unit_label)
{
  int n_sig = (dd->digit[1] >= 0) ? 3 : (dd->digit[2] >= 0) ? 2 : 1;
  int first = NUM_DIGITS - n_sig; /* index of first significant digit */

  for (int i = 0; i < NUM_DIGITS; i++)
  {
    int rel = i - first;
    int x = ox + (rel > 0 ? rel : 0) * DIGIT_WIDTH;
    lv_obj_align(objs[i], LV_ALIGN_TOP_LEFT, x, y);
  }

  show_object(dot_objs[0], false);
  show_object(dot_objs[1], false);

  if (unit_label)
  {
    if (unit && unit[0] != '\0')
    {
      lv_label_set_text(unit_label, unit);
      lv_obj_align(unit_label, LV_ALIGN_TOP_LEFT, ox + n_sig * DIGIT_WIDTH + 1, y);
      show_object(unit_label, true);
    }
    else
    {
      show_object(unit_label, false);
    }
  }
}

static void render_digit_group(lv_obj_t *objs[NUM_DIGITS], const digit_display_t *dd)
{
  for (int i = 0; i < 4; i++)
    set_digit_sprite(objs, i, dd->digit[i]);
}

static void update_cgm_dots_visibility(lv_obj_t *dot_objs[2], bool show_slot, const digit_display_t *dd)
{
  // Only adjust the dots when showing glucose: dot[0] becomes the mmol decimal
  // point, dot[1] is unused. For time (or weather/HA) leave them exactly as the
  // caller set them, otherwise the time colon gets hidden and isn't restored
  // when switching back from glucose.
  if (!show_slot)
    return;
  const bool is_mmol = (dd && dd->is_mmol) || eeprom_glucose_unit == 1;
  show_object(dot_objs[1], false);
  show_object(dot_objs[0], is_mmol);
}

static void apply_digit_display_visibility(lv_obj_t *digits[NUM_DIGITS], lv_obj_t *dot_objs[2],
                                           lv_obj_t *mgdl_img, lv_obj_t *unit_label,
                                           const screen_layout_profile_t *layout,
                                           const schedule_runner_t *runner,
                                           screen_element_id_t elem)
{
  const bool show_time = layout_show_time_digits(layout, runner, elem);
  const bool show_glucose = layout_show_glucose_on_digits(layout, runner, elem);
  const bool show_weather = layout_show_weather_on_digits(layout, runner, elem);
  const bool show_ha = layout_show_ha_on_digits(layout, runner, elem);
  const bool show_slot_digits = show_time || show_glucose || show_weather || show_ha;

  for (int i = 0; i < 4; i++)
    show_object(digits[i], show_slot_digits);
  show_object(dot_objs[0], show_time || show_glucose);
  show_object(dot_objs[1], show_time || show_glucose);
  update_cgm_dots_visibility(dot_objs, show_glucose, NULL);
  show_object(mgdl_img, show_glucose);
  bool degree_visible = show_weather ||
                        (show_ha && runner->count > 0 &&
                         runner->schedule[runner->current_idx].label[0] != '\0');
  show_object(unit_label, degree_visible);
}

static void apply_widget_visibility(const screen_layout_profile_t *layout)
{
  // Boot splash: until the clock is synced, keep only the frixos logo and the
  // boot message (Starting.../IP/etc.) on screen. Everything else was hidden at
  // startup and is revealed here only once we have a valid time. (This runs at
  // boot via display_changed(), before the first NTP sync.)
  if (!time_valid)
  {
    show_object(label_msg, true);
    return;
  }

  const screen_widget_t *w_weather = &layout->widget[SCREEN_ELEM_WEATHER];
  const screen_widget_t *w_moon = &layout->widget[SCREEN_ELEM_MOON];
  const screen_widget_t *w_msg = &layout->widget[SCREEN_ELEM_MESSAGE];

  const bool show_time_digits = layout_show_time_digits(layout, &primary_runner, SCREEN_ELEM_TIME);

  apply_digit_display_visibility(digit_objs, dots, img_mgdl, label_degree, layout, &primary_runner, SCREEN_ELEM_TIME);
  apply_digit_display_visibility(aux_digit_objs, aux_dots, img_mgdl_aux, label_degree_aux,
                                 layout, &aux_runner, SCREEN_ELEM_TIME_AUX);
  show_object(img_ampm, show_time_digits && eeprom_12hour);

  show_object(img_weather, w_weather->enabled && weather_valid && time_valid);
  show_object(img_moon, w_moon->enabled && time_valid);
  show_object(label_msg, w_msg->enabled || !time_valid);

  if (wifi_disabled_by_active_hours)
  {
    show_object(img_wifi, layout->widget[SCREEN_ELEM_WIFI_OFF].enabled);
    show_object(img_glucose, false);
    show_object(img_glucose_trend, false);
  }
  else
  {
    show_object(img_wifi, false);
    show_object(img_glucose, layout->widget[SCREEN_ELEM_GLUCOSE_LEVEL].enabled && is_glucose_on());
    show_object(img_glucose_trend, layout->widget[SCREEN_ELEM_GLUCOSE_TREND].enabled && is_glucose_on() && is_glucose_fresh());
  }

  // User icons: shown when enabled and the JPG exists (icon_src_ok cached by
  // update_icon_srcs). Hidden during the boot splash via the !time_valid return.
  for (int i = 0; i < SCREEN_ICON_COUNT; i++)
    show_object(img_icon[i], icon_src_ok[i] && layout->widget[SCREEN_ELEM_ICON_1 + i].enabled);
}

#define SCREEN_LAYER_MAX 4

static uint8_t screen_widget_layer(const screen_widget_t *w)
{
  if (!w)
    return 0;
  return w->z <= SCREEN_LAYER_MAX ? w->z : SCREEN_LAYER_MAX;
}

static void screen_layout_push_obj(lv_obj_t *obj, lv_obj_t **objs, int *count, int max_count)
{
  if (obj && *count < max_count)
    objs[(*count)++] = obj;
}

static void screen_layout_collect_elem_objects(screen_element_id_t id, lv_obj_t **objs, int *count)
{
  switch (id)
  {
  case SCREEN_ELEM_GLUCOSE_LEVEL:
    screen_layout_push_obj(img_glucose, objs, count, 12);
    break;
  case SCREEN_ELEM_GLUCOSE_TREND:
    screen_layout_push_obj(img_glucose_trend, objs, count, 12);
    break;
  case SCREEN_ELEM_WIFI_OFF:
    screen_layout_push_obj(img_wifi, objs, count, 12);
    break;
  case SCREEN_ELEM_WEATHER:
    screen_layout_push_obj(img_weather, objs, count, 12);
    break;
  case SCREEN_ELEM_MOON:
    screen_layout_push_obj(img_moon, objs, count, 12);
    break;
  case SCREEN_ELEM_TIME:
    for (int i = 0; i < NUM_DIGITS; i++)
      screen_layout_push_obj(digit_objs[i], objs, count, 12);
    screen_layout_push_obj(dots[0], objs, count, 12);
    screen_layout_push_obj(dots[1], objs, count, 12);
    screen_layout_push_obj(img_mgdl, objs, count, 12);
    screen_layout_push_obj(label_degree, objs, count, 12);
    break;
  case SCREEN_ELEM_TIME_AUX:
    for (int i = 0; i < NUM_DIGITS; i++)
      screen_layout_push_obj(aux_digit_objs[i], objs, count, 12);
    screen_layout_push_obj(aux_dots[0], objs, count, 12);
    screen_layout_push_obj(aux_dots[1], objs, count, 12);
    screen_layout_push_obj(img_mgdl_aux, objs, count, 12);
    screen_layout_push_obj(label_degree_aux, objs, count, 12);
    break;
  case SCREEN_ELEM_AMPM:
    screen_layout_push_obj(img_ampm, objs, count, 12);
    break;
  case SCREEN_ELEM_MESSAGE:
    screen_layout_push_obj(label_msg, objs, count, 12);
    break;
  case SCREEN_ELEM_DIGIT_LABEL:
    screen_layout_push_obj(label_digit, objs, count, 12);
    break;
  case SCREEN_ELEM_DIGIT_LABEL_AUX:
    screen_layout_push_obj(label_digit_aux, objs, count, 12);
    break;
  case SCREEN_ELEM_GRAPH:
    screen_layout_push_obj(graph_canvas, objs, count, 12);
    break;
  default:
    if (screen_elem_is_static_text(id))
      screen_layout_push_obj(label_static[screen_static_text_index(id)], objs, count, 12);
    else if (screen_elem_is_icon(id))
      screen_layout_push_obj(img_icon[screen_icon_index(id)], objs, count, 12);
    break;
  }
}

static void apply_screen_layout_z_order(const screen_layout_profile_t *layout)
{
  int order[SCREEN_ELEM_COUNT];
  for (int i = 0; i < SCREEN_ELEM_COUNT; i++)
    order[i] = i;

  for (int i = 0; i < SCREEN_ELEM_COUNT - 1; i++)
  {
    for (int j = 0; j < SCREEN_ELEM_COUNT - i - 1; j++)
    {
      uint8_t zj = screen_widget_layer(&layout->widget[order[j]]);
      uint8_t zjp1 = screen_widget_layer(&layout->widget[order[j + 1]]);
      if (zj > zjp1 || (zj == zjp1 && order[j] > order[j + 1]))
      {
        int tmp = order[j];
        order[j] = order[j + 1];
        order[j + 1] = tmp;
      }
    }
  }

  for (int i = 0; i < SCREEN_ELEM_COUNT; i++)
  {
    lv_obj_t *objs[12];
    int count = 0;
    screen_layout_collect_elem_objects((screen_element_id_t)order[i], objs, &count);
    for (int j = 0; j < count; j++)
      lv_obj_move_foreground(objs[j]);
  }

  raise_grid_foreground();
}

static void apply_screen_layout_positions(void)
{
  if (!screen_layout_positions_live)
    return;

  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];
  const screen_widget_t *w_weather = &layout->widget[SCREEN_ELEM_WEATHER];
  const screen_widget_t *w_moon = &layout->widget[SCREEN_ELEM_MOON];
  const screen_widget_t *w_glucose_level = &layout->widget[SCREEN_ELEM_GLUCOSE_LEVEL];
  const screen_widget_t *w_wifi = &layout->widget[SCREEN_ELEM_WIFI_OFF];
  const screen_widget_t *w_glucose_trend = &layout->widget[SCREEN_ELEM_GLUCOSE_TREND];
  const screen_widget_t *w_time = &layout->widget[SCREEN_ELEM_TIME];
  const screen_widget_t *w_time_aux = &layout->widget[SCREEN_ELEM_TIME_AUX];
  const screen_widget_t *w_ampm = &layout->widget[SCREEN_ELEM_AMPM];
  const screen_widget_t *w_msg = &layout->widget[SCREEN_ELEM_MESSAGE];

  lvgl_port_lock(0);
  lv_obj_align(img_weather, LV_ALIGN_TOP_LEFT, layout_abs_x(w_weather), layout_abs_y(w_weather));
  lv_obj_align(img_moon, LV_ALIGN_TOP_LEFT, layout_abs_x(w_moon), layout_abs_y(w_moon));
  lv_obj_align(img_glucose, LV_ALIGN_TOP_LEFT, layout_abs_x(w_glucose_level), layout_abs_y(w_glucose_level));
  lv_obj_align(img_wifi, LV_ALIGN_TOP_LEFT, layout_abs_x(w_wifi), layout_abs_y(w_wifi));
  lv_obj_align(img_glucose_trend, LV_ALIGN_TOP_LEFT, layout_abs_x(w_glucose_trend), layout_abs_y(w_glucose_trend));
  lv_obj_align(img_mgdl, LV_ALIGN_TOP_LEFT, layout_abs_x(w_time) + 79, layout_abs_y(w_time) + 5);
  lv_obj_align(img_mgdl_aux, LV_ALIGN_TOP_LEFT, layout_abs_x(w_time_aux) + 79, layout_abs_y(w_time_aux) + 5);

  for (int i = 0; i < 4; i++)
    lv_obj_align(digit_objs[i], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time) + i * 18 + (i > 1 ? 6 : 0), layout_abs_y(w_time));
  for (int i = 0; i < 4; i++)
    lv_obj_align(aux_digit_objs[i], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time_aux) + i * 18 + (i > 1 ? 6 : 0), layout_abs_y(w_time_aux));
  lv_obj_align(img_ampm, LV_ALIGN_TOP_LEFT, layout_abs_x(w_ampm), layout_abs_y(w_ampm));
  lv_obj_align(dots[0], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time) + 2 * 18 + 1, layout_abs_y(w_time) + 10);
  lv_obj_align(dots[1], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time) + 2 * 18 + 1, layout_abs_y(w_time) + 26);
  lv_obj_align(aux_dots[0], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time_aux) + 2 * 18 + 1, layout_abs_y(w_time_aux) + 10);
  lv_obj_align(aux_dots[1], LV_ALIGN_TOP_LEFT, layout_abs_x(w_time_aux) + 2 * 18 + 1, layout_abs_y(w_time_aux) + 26);
  lv_obj_set_pos(label_msg, layout_abs_x(w_msg), layout_abs_y(w_msg));
  if (graph_canvas)
  {
    const screen_widget_t *w_graph = &layout->widget[SCREEN_ELEM_GRAPH];
    lv_obj_align(graph_canvas, LV_ALIGN_TOP_LEFT, layout_abs_x(w_graph), layout_abs_y(w_graph));
  }
  for (int i = 0; i < SCREEN_ICON_COUNT; i++)
  {
    const screen_widget_t *w_icon = &layout->widget[SCREEN_ELEM_ICON_1 + i];
    lv_obj_align(img_icon[i], LV_ALIGN_TOP_LEFT, layout_abs_x(w_icon), layout_abs_y(w_icon));
  }
  update_static_text_labels();
  update_digit_label_widgets();
  apply_screen_layout_z_order(layout);
  lvgl_port_unlock();
}

// Render the generic graph widget into its RGB565 canvas. Snapshots the ring
// under the graph mutex, then draws under the LVGL lock (the two locks are
// never held together). Skips the redraw when nothing changed to avoid
// per-minute flicker.
//
// The canvas drawing approach (band/axis markers) is derived from the CGM
// glucose graph contributed by Benoit Poirier in PR #180.

// Draw a crisp line into the canvas buffer with Bresenham + direct pixel
// writes. lv_draw_line does not render thin diagonal lines visibly on this
// RGB565 canvas (anti-aliased coverage fades out), so the trend line is drawn
// by hand. thick=true gives a 2px line, else 1px. Pixels clamped to bounds.
// Format a ring value (x10) for display. One decimal when the visible range is
// small (e.g. temperature), whole numbers otherwise (e.g. glucose/power).
static void graph_fmt(char *buf, size_t sz, float ring_val, bool dec)
{
  if (dec)
    snprintf(buf, sz, "%.1f", ring_val / 10.0f);
  else
    snprintf(buf, sz, "%d", (int)lroundf(ring_val / 10.0f));
}

static void graph_px_line(lv_obj_t *cv, int x0, int y0, int x1, int y1, int w, int h, lv_color_t c, bool thick)
{
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy;
  int omax = thick ? 1 : 0;
  for (;;)
  {
    for (int ox = 0; ox <= omax; ox++)
      for (int oy = 0; oy <= omax; oy++)
      {
        int px = x0 + ox, py = y0 + oy;
        if (px >= 0 && px < w && py >= 0 && py < h)
          lv_canvas_set_px(cv, px, py, c, LV_OPA_COVER);
      }
    if (x0 == x1 && y0 == y1)
      break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

void update_graph(void)
{
  if (!graph_canvas)
    return;

  // The graph is part of the layout: during the boot splash (no time yet) it
  // stays hidden, like everything except the logo and the boot message.
  if (!time_valid)
    return;

  const screen_layout_profile_t *layout_p = &eeprom_screen_layout.profile[font_index];
  const screen_widget_t *w = &layout_p->widget[SCREEN_ELEM_GRAPH];
  const screen_graph_cfg_t *g = &layout_p->graph;

  static time_t last_render_sample = -1;
  static bool last_render_shown = false;

  if (!w->enabled || g->token[0] == '\0')
  {
    if (last_render_shown || graph_canvas_buf)
    {
      lvgl_port_lock(0);
      lv_obj_add_flag(graph_canvas, LV_OBJ_FLAG_HIDDEN);
      if (graph_canvas_buf) // hidden -> not drawn, safe to release the buffer
      {
        free(graph_canvas_buf);
        graph_canvas_buf = NULL;
        graph_canvas_buf_size = 0;
      }
      lvgl_port_unlock();
      last_render_shown = false;
      last_render_sample = -1;
    }
    return;
  }

  int16_t samp[GRAPH_MAX_POINTS];
  time_t last_sample = 0;
  uint16_t interval_min = g->interval_min ? g->interval_min : 1;
  int n = graph_snapshot(samp, GRAPH_MAX_POINTS, &last_sample, &interval_min);

  // Nothing new since last render and already shown -> skip (no flicker),
  // unless a config edit asked for an immediate redraw.
  if (!graph_force_redraw && last_render_shown && last_sample == last_render_sample)
    return;
  graph_force_redraw = false;

  int gw = (int)g->width;
  if (gw < GRAPH_MIN_W) gw = GRAPH_MIN_W;
  if (gw > GRAPH_MAX_W) gw = GRAPH_MAX_W;
  int gh = (int)g->height;
  if (gh < GRAPH_MIN_H) gh = GRAPH_MIN_H;
  if (gh > GRAPH_MAX_H) gh = GRAPH_MAX_H;

  const bool show_yaxis = (g->flags & GRAPH_FLAG_SHOW_AXIS) != 0;  // value (Y) axis + labels
  const bool show_xaxis = (g->flags & GRAPH_FLAG_SHOW_XAXIS) != 0; // time (X) axis + labels
  const bool show_value = (g->flags & GRAPH_FLAG_SHOW_VALUE) != 0;
  const bool band_on = (g->flags & GRAPH_FLAG_BAND) != 0;
  const bool boolean = (g->flags & GRAPH_FLAG_BOOLEAN) != 0;
  const bool autoscale = (g->flags & GRAPH_FLAG_AUTOSCALE) != 0;
  const bool thick = (g->flags & GRAPH_FLAG_THICK) != 0;

  lv_color_t col_line = lv_color_make(w->color_r, w->color_g, w->color_b);
  lv_color_t col_bg = lv_color_make(w->bg_r, w->bg_g, w->bg_b);
  lv_color_t col_band = lv_color_make(g->band_r, g->band_g, g->band_b);
  lv_color_t col_warn = lv_color_make(g->warn_r, g->warn_g, g->warn_b);
  lv_color_t col_axis = lv_color_make(g->axis_r, g->axis_g, g->axis_b);

  int left_m = show_yaxis ? 17 : 1; // 16px fits 4 Tiny5 digits (4px adv) exactly
  int top_m = 1;
  int right_m = 1;
  int bot_m = show_xaxis ? 8 : 1;
  int px1 = left_m, px2 = gw - right_m - 1;
  int py1 = top_m, py2 = gh - bot_m - 1;
  int pw = px2 - px1 + 1, ph = py2 - py1 + 1;
  if (pw < 2 || ph < 2)
    return;

  // Data range over valid samples.
  float dmin = 1e9f, dmax = -1e9f;
  int valid = 0;
  int16_t last_valid = GRAPH_VAL_UNSET;
  for (int i = 0; i < n; i++)
  {
    if (samp[i] == GRAPH_VAL_UNSET)
      continue;
    valid++;
    last_valid = samp[i];
    float v = samp[i];
    if (v < dmin) dmin = v;
    if (v > dmax) dmax = v;
  }

  // ymin/ymax are in ring units (value x10), matching samp[]. Config values
  // (y_min/y_max, band) are real units, so they are x10 where used.
  float ymin, ymax;
  const bool manual_range = (!autoscale && g->y_min != GRAPH_VAL_UNSET &&
                             g->y_max != GRAPH_VAL_UNSET && g->y_max > g->y_min);
  if (boolean)
  {
    ymin = 0.0f;
    ymax = 10.0f; // 1 x10
  }
  else if (manual_range)
  {
    ymin = (float)g->y_min * 10.0f;
    ymax = (float)g->y_max * 10.0f;
  }
  else if (valid > 0)
  {
    ymin = dmin;
    ymax = dmax;
    if (ymax <= ymin) // single / flat value -> pad +-1 real unit so it's centred
    {
      ymin -= 10.0f;
      ymax += 10.0f;
    }
  }
  else
  {
    ymin = 0.0f;
    ymax = 10.0f;
  }
  // Keep the high/low bands on screen: when bands are enabled and the scale is
  // auto (not an explicit fixed y_min/y_max), widen the range to include both
  // thresholds so they never fall off the top or bottom of the plot.
  if (band_on && !boolean && !manual_range &&
      g->band_low != GRAPH_VAL_UNSET && g->band_high != GRAPH_VAL_UNSET &&
      g->band_high > g->band_low)
  {
    const float bl = (float)g->band_low * 10.0f;
    const float bh = (float)g->band_high * 10.0f;
    if (bl < ymin) ymin = bl;
    if (bh > ymax) ymax = bh;
  }

  // Snap the Y bounds to whole units: ceil the top, floor the bottom, so the
  // axis labels are clean integers and the scale matches them.
  ymax = ceilf(ymax / 10.0f) * 10.0f;
  ymin = floorf(ymin / 10.0f) * 10.0f;
  if (ymax <= ymin) ymax = ymin + 10.0f;
  float yrange = ymax - ymin;
  if (yrange < 1e-6f) yrange = 1.0f;

  // Fixed window: newest sample at px2, one slot left per older sample.
  int slots = (g->points > 1) ? (g->points - 1) : 1;
  float px_per_slot = (float)(pw - 1) / (float)slots;

  // Inset the curve/band a couple of px from the top/bottom plot edges so the
  // trend isn't drawn flush against (and partly clipped at) the boundary when a
  // data extreme lands exactly on the snapped ymin/ymax. (#188)
  int cmargin = (ph > 10) ? 2 : 0;
  int cy1 = py1 + cmargin, cy2 = py2 - cmargin;
  int cph = cy2 - cy1 + 1;

#define V2Y(v) (cy2 - (int)(((float)(v) - ymin) / yrange * (float)(cph - 1)))
#define CY(y) ((y) < py1 ? py1 : ((y) > py2 ? py2 : (y)))
#define I2X(i) (px2 - (int)((float)((n - 1) - (i)) * px_per_slot))

  lvgl_port_lock(0);
  lv_obj_add_flag(graph_canvas, LV_OBJ_FLAG_HIDDEN);
  // (Re)allocate the RGB565 buffer at the configured size (under the LVGL lock,
  // display task only — never races the renderer). Smaller than max => less heap.
  size_t need = (size_t)gw * gh * 2;
  if (graph_canvas_buf_size != need)
  {
    uint8_t *nb = realloc(graph_canvas_buf, need);
    if (!nb)
    {
      ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "graph_canvas_buf alloc %u failed", (unsigned)need);
      lvgl_port_unlock();
      return;
    }
    graph_canvas_buf = nb;
    graph_canvas_buf_size = need;
  }
  lv_canvas_set_buffer(graph_canvas, graph_canvas_buf, gw, gh, LV_COLOR_FORMAT_RGB565);
  lv_canvas_fill_bg(graph_canvas, col_bg, LV_OPA_COVER);
  lv_layer_t layer;
  lv_canvas_init_layer(graph_canvas, &layer);

  // 1. Low/high band: shade the in-range zone, then draw a crisp horizontal
  //    line at each threshold (axis colour) so the band levels are unmistakable.
  if (band_on && g->band_low != GRAPH_VAL_UNSET && g->band_high != GRAPH_VAL_UNSET && g->band_high > g->band_low)
  {
    int yh = CY(V2Y(g->band_high * 10));
    int yl = CY(V2Y(g->band_low * 10));
    if (yl > yh)
    {
      lv_draw_rect_dsc_t bd;
      lv_draw_rect_dsc_init(&bd);
      bd.bg_color = col_band;
      bd.bg_opa = LV_OPA_COVER;
      bd.border_width = 0;
      lv_area_t ba = {px1, yh, px2, yl};
      lv_draw_rect(&layer, &bd, &ba);
    }
    lv_draw_line_dsc_t bl;
    lv_draw_line_dsc_init(&bl);
    bl.width = 1;
    bl.color = col_axis;
    bl.opa = LV_OPA_COVER;
    bl.p1.x = px1; bl.p1.y = yh; bl.p2.x = px2; bl.p2.y = yh; // high threshold
    lv_draw_line(&layer, &bl);
    bl.p1.x = px1; bl.p1.y = yl; bl.p2.x = px2; bl.p2.y = yl; // low threshold
    lv_draw_line(&layer, &bl);
  }

  // (The trend polyline is drawn after finish_layer with direct pixels — see
  // graph_px_line — because lv_draw_line doesn't render thin diagonals here.)

  // 3. Axis + labels. The value (Y) axis and time (X) axis are independent so a
  //    user can hide either to reclaim space (#189): hiding Y frees the left
  //    margin, hiding X frees the bottom margin (handled via left_m/bot_m).
  if (show_yaxis || show_xaxis)
  {
    lv_draw_line_dsc_t al;
    lv_draw_line_dsc_init(&al);
    al.width = 1;
    al.color = col_axis;

    lv_draw_label_dsc_t lb;
    lv_draw_label_dsc_init(&lb);
    lb.font = &graph_tiny5; // compact 5px font for the x/y axis labels
    lb.color = col_axis;
    lb.opa = LV_OPA_COVER;
    lb.text_local = 1; // deferred draw: strdup the stack strings

    if (show_yaxis)
    {
      al.p1.x = px1 - 1; al.p1.y = py1; al.p2.x = px1 - 1; al.p2.y = py2 + 1;
      lv_draw_line(&layer, &al); // vertical axis line

      // Y bounds are whole units (ceil/floor) -> integer labels.
      char ymax_s[8], ymin_s[8];
      graph_fmt(ymax_s, sizeof(ymax_s), ymax, false);
      graph_fmt(ymin_s, sizeof(ymin_s), ymin, false);
      lb.align = LV_TEXT_ALIGN_RIGHT;
      lb.text = ymax_s;
      lv_area_t yat = {0, py1, px1 - 2, py1 + 8};
      lv_draw_label(&layer, &lb, &yat);
      lb.text = ymin_s;
      lv_area_t yab = {0, py2, px1 - 2, py2 + 8};
      lv_draw_label(&layer, &lb, &yab);
    }

    if (show_xaxis)
    {
      al.p1.x = px1 - 1; al.p1.y = py2 + 1; al.p2.x = px2; al.p2.y = py2 + 1;
      lv_draw_line(&layer, &al); // horizontal axis line

      char xnow[6] = "now";
      lb.align = LV_TEXT_ALIGN_RIGHT;
      lb.text = xnow;
      // Wide enough that "now" never overflows, right edge 6px in; raised 1px.
      lv_area_t xar = {px2 - 24, py2 + 1, px2 - 2, gh - 1};
      lv_draw_label(&layer, &lb, &xar);

      // Window duration label, e.g. 60 points x 1 min -> "-1h", x 5 min -> "-5h".
      long span_min = (long)g->points * (long)interval_min;
      char xleft[10];
      if (span_min >= 60)
        snprintf(xleft, sizeof(xleft), "-%ldh", span_min / 60);
      else
        snprintf(xleft, sizeof(xleft), "-%ldm", span_min);
      lb.align = LV_TEXT_ALIGN_LEFT;
      lb.text = xleft;
      lv_area_t xal = {px1, py2 + 2, px1 + 22, gh - 1};
      lv_draw_label(&layer, &lb, &xal);
    }
  }

  // 4. Current value readout, placed just above or below the latest point so
  //    the trend line doesn't run through the digits: below it when the line
  //    sits in the upper half of the plot, above it when it sits in the lower.
  if (show_value && last_valid != GRAPH_VAL_UNSET)
  {
    lv_draw_label_dsc_t vb;
    lv_draw_label_dsc_init(&vb);
    vb.font = &frixos_8;
    vb.color = col_line;
    vb.opa = LV_OPA_COVER;
    vb.text_local = 1;
    vb.align = LV_TEXT_ALIGN_RIGHT;
    char vt[12];
    graph_fmt(vt, sizeof(vt), (float)last_valid, (yrange < 200.0f));
    vb.text = vt;
    int vy = CY(V2Y((float)last_valid));
    int vtop = ((vy - py1) < (py2 - py1) / 2) ? (vy + 2) : (vy - 10);
    if (vtop < py1) vtop = py1;
    if (vtop + 8 > py2) vtop = py2 - 8;
    lv_area_t va = {px1, vtop, px2, vtop + 8};
    lv_draw_label(&layer, &vb, &va);
  }

  lv_canvas_finish_layer(graph_canvas, &layer);

  // 5. Trend polyline (direct pixels, on top of the band/axes). Connect each
  // valid sample to the PREVIOUS valid one, skipping empty bins between them
  // (CGM history is sparser than the bin interval); break only across a long
  // gap so a real outage isn't bridged with a straight line.
  if (valid >= 2)
  {
    int max_gap = g->points / 2;
    if (max_gap < 4)
      max_gap = 4;
    int prev = -1;
    for (int i = 0; i < n; i++)
    {
      if (samp[i] == GRAPH_VAL_UNSET)
        continue;
      if (prev >= 0 && (i - prev) <= max_gap)
      {
        float v1 = samp[prev], v2 = samp[i];
        int x1 = I2X(prev), x2 = I2X(i);
        int y1 = CY(V2Y(v1)), y2 = CY(V2Y(v2));
        const bool have_band = band_on && g->band_low != GRAPH_VAL_UNSET && g->band_high != GRAPH_VAL_UNSET;
        if (boolean)
        {
          lv_color_t c = col_line;
          if (have_band)
          {
            float avg = (v1 + v2) * 0.5f; // ring units (x10); band is real -> x10
            if (avg < (float)g->band_low * 10.0f || avg > (float)g->band_high * 10.0f)
              c = col_warn;
          }
          graph_px_line(graph_canvas, x1, y1, x2, y1, gw, gh, c, thick); // hold
          graph_px_line(graph_canvas, x2, y1, x2, y2, gw, gh, c, thick); // step
        }
        else if (have_band)
        {
          // Split the segment at each band-threshold crossing so the warn colour
          // covers exactly the out-of-band part: colouring by the segment average
          // flipped the whole segment up to half a step before the real crossing.
          const float lo10 = (float)g->band_low * 10.0f;
          const float hi10 = (float)g->band_high * 10.0f;
          const float dv = v2 - v1;
          float ts[2];
          int nts = 0;
          if (fabsf(dv) > 1e-6f)
          {
            const float cand[2] = {lo10, hi10};
            for (int k = 0; k < 2; k++)
            {
              float t = (cand[k] - v1) / dv;
              if (t > 0.0f && t < 1.0f) ts[nts++] = t;
            }
            if (nts == 2 && ts[0] > ts[1]) { float tmp = ts[0]; ts[0] = ts[1]; ts[1] = tmp; }
          }
          float prevT = 0.0f;
          int ax = x1, ay = y1;
          for (int s = 0; s <= nts; s++)
          {
            float curT = (s < nts) ? ts[s] : 1.0f;
            float midV = v1 + dv * ((prevT + curT) * 0.5f);
            lv_color_t c = (midV < lo10 || midV > hi10) ? col_warn : col_line;
            int bx = x1 + (int)lroundf((float)(x2 - x1) * curT);
            int by = y1 + (int)lroundf((float)(y2 - y1) * curT);
            graph_px_line(graph_canvas, ax, ay, bx, by, gw, gh, c, thick);
            ax = bx;
            ay = by;
            prevT = curT;
          }
        }
        else
        {
          graph_px_line(graph_canvas, x1, y1, x2, y2, gw, gh, col_line, thick);
        }
      }
      prev = i;
    }
  }
  else if (valid == 1)
  {
    // A single reading can't form a line -> draw a 3x3 marker so it's visible.
    for (int i = 0; i < n; i++)
    {
      if (samp[i] == GRAPH_VAL_UNSET)
        continue;
      int mx = I2X(i), my = CY(V2Y(samp[i]));
      for (int ox = -1; ox <= 1; ox++)
        for (int oy = -1; oy <= 1; oy++)
        {
          int px = mx + ox, py = my + oy;
          if (px >= 0 && px < gw && py >= 0 && py < gh)
            lv_canvas_set_px(graph_canvas, px, py, col_line, LV_OPA_COVER);
        }
      break;
    }
  }

  lv_obj_clear_flag(graph_canvas, LV_OBJ_FLAG_HIDDEN);
  lvgl_port_unlock();

  last_render_sample = last_sample;
  last_render_shown = true;

#undef V2Y
#undef CY
#undef I2X
}

static void handle_screen_layout_on_wifi(void)
{
  static bool latched = false;

  if (latched || !wifi_connected)
    return;

  latched = true;
  screen_layout_positions_live = true;
  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi connected, applying screen layout positions");
  display_changed();
}

// this is the initialization needed when stuff about the display has changed
void display_changed(void)
{
  lv_obj_t *scr = lv_scr_act();
  char buf[128];
  /* Task lock */

  sync_schedule_runners();
  primary_runner.slot_start = 0;
  primary_runner.current_idx = 0;
  primary_runner.showing_glucose = false;
  primary_runner.showing_weather = false;
  primary_runner.showing_ha = false;
  aux_runner.slot_start = 0;
  aux_runner.current_idx = 0;
  aux_runner.showing_glucose = false;
  aux_runner.showing_weather = false;
  aux_runner.showing_ha = false;

  lvgl_port_lock(0);
  st7735_set_rotation_and_mirror(eeprom_rotation, eeprom_mirroring);
  lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  show_grid(eeprom_show_grid);
  find_file(buf, sizeof(buf), eeprom_font[font_index], "font");
  lv_image_set_src(img_digits_sprite, buf);
  lv_obj_align(img_digits_sprite, LV_ALIGN_TOP_LEFT, 0, 0);
  find_file(buf, sizeof(buf), eeprom_aux_font[font_index], "font");
  lv_image_set_src(img_digits_sprite_aux, buf);
  lv_obj_align(img_digits_sprite_aux, LV_ALIGN_TOP_LEFT, 0, 0);
  lvgl_port_unlock();

  ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Display change: lx %.1f idx %i flt %i fnt %s",
              lux, font_index, eeprom_color_filter[font_index], buf);

  lvgl_port_lock(0);

  find_file(buf, sizeof(buf), eeprom_font[font_index], "weather");
  lv_image_set_src(img_weather, buf);
  lv_image_set_inner_align(img_weather, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_weather, 32, 22);
  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];

  find_file(buf, sizeof(buf), eeprom_font[font_index], "moon");
  lv_image_set_src(img_moon, buf);
  lv_image_set_inner_align(img_moon, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_moon, 14, 14);

  // Load glucose icon
  find_file(buf, sizeof(buf), eeprom_font[font_index], "glucose");
  lv_image_set_src(img_glucose, buf);
  lv_image_set_inner_align(img_glucose, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_glucose, 14, 20);

  // Load WiFi disabled icon
  find_file(buf, sizeof(buf), eeprom_font[font_index], "wifi-off");
  lv_image_set_src(img_wifi, buf);
  lv_image_set_inner_align(img_wifi, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_wifi, 20, 20);

  find_file(buf, sizeof(buf), eeprom_font[font_index], "trend");
  lv_image_set_src(img_glucose_trend, buf);
  lv_image_set_inner_align(img_glucose_trend, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_glucose_trend, 12, 14);

  // Load mgdl image
  find_file(buf, sizeof(buf), eeprom_font[font_index], "units");
  lv_image_set_src(img_mgdl, buf);
  lv_image_set_inner_align(img_mgdl, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_mgdl, 12, 24);
  lv_image_set_src(img_mgdl_aux, buf);
  lv_image_set_inner_align(img_mgdl_aux, LV_ALIGN_TOP_LEFT);
  lv_obj_set_size(img_mgdl_aux, 12, 24);

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
    lv_image_set_src(aux_digit_objs[i], lv_image_get_src(img_digits_sprite_aux));
    lv_image_set_inner_align(aux_digit_objs[i], LV_ALIGN_TOP_LEFT);
    lv_obj_set_size(aux_digit_objs[i], DIGIT_WIDTH, DIGIT_HEIGHT);
  }

  find_file(buf, sizeof(buf), eeprom_font[font_index], "dot");
  char aux_dot_buf[128];
  find_file(aux_dot_buf, sizeof(aux_dot_buf), eeprom_aux_font[font_index], "dot");
  for (int i = 0; i < 2; i++)
  {
    lv_image_set_src(dots[i], buf);
    lv_image_set_src(aux_dots[i], aux_dot_buf);
  }

  const screen_widget_t *w_msg = &layout->widget[SCREEN_ELEM_MESSAGE];
  lv_obj_set_style_text_font(label_msg, get_selected_font(active_message_font_index()), 0);
  apply_message_widget_styles(w_msg);
  // Invalidate character width cache when font changes
  invalidate_char_width_cache();

  update_icon_srcs(); // (re)point icon images at S:/icon<N>.jpg before visibility
  apply_widget_visibility(layout);
  if (!screen_layout_positions_live)
  {
    update_static_text_labels();
    update_digit_label_widgets();
  }
  lvgl_port_unlock(); // Unlock LVGL

  apply_screen_layout_positions();

  // Apply gamma curve based on eeprom_color_filter, after we get on wifi
  if (wifi_connected)
  {
    ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Color filter: %u (font %u, wifi %s)",
                eeprom_color_filter[font_index], font_index, wifi_connected ? "true" : "false");
    set_display_color_filter(eeprom_color_filter[font_index]);
  }

  ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Display changed");

  // Font or layout changed; remeasure message width for scroll vs centered mode.
  if (last_scroll_msg[0] != '\0')
  {
    set_scroll_message(last_scroll_msg);
  }

  // (Re)configure the generic graph sampler from the active profile. Keeps the
  // ring/history when token+interval+points are unchanged (e.g. day<->night).
  {
    const screen_layout_profile_t *gp = &eeprom_screen_layout.profile[font_index];
    graph_configure(&gp->graph, gp->widget[SCREEN_ELEM_GRAPH].enabled != 0);
  }
  graph_force_redraw = true; // a layout/config edit just landed -> redraw now
  update_graph();
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
  if (t < 0.5f) return 2.0f * t * t;
  float f = -2.0f * t + 2.0f;
  return 1.0f - (f * f) / 2.0f;
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
  const bool weather_enabled = eeprom_screen_layout.profile[font_index].widget[SCREEN_ELEM_WEATHER].enabled;
  show_object(img_weather, weather_enabled && weather_valid && time_valid);
  lvgl_port_unlock();
}

void show_qr_code(void)
{
  lvgl_port_lock(0);
  lv_image_set_src(img_logo, "S:/wifi-qr.jpg");
  lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, 37, 32);
  set_scroll_message(" Scan QR code to connect to your Frixos ");
  lvgl_port_unlock();
}

// Watchdog mechanism for display task hang detection
static volatile uint32_t display_task_heartbeat = 0;
static uint32_t last_display_task_heartbeat = 0xFFFFFFFF;
static uint8_t watchdog_missed_count = 0;

// Restart after this many consecutive 30s windows with a frozen heartbeat.
// 2 windows ≈ 60s, long enough to ride out legitimately slow operations
// without leaving the device permanently bricked when the display task
// deadlocks (e.g. on the LVGL port lock).
#define WATCHDOG_MISS_LIMIT 2

static void watchdog_callback(void *arg)
{
  if (display_task_heartbeat == last_display_task_heartbeat)
  {
    watchdog_missed_count++;
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Watchdog: display task hung (heartbeat %lu, miss %u/%u)",
                display_task_heartbeat, watchdog_missed_count, WATCHDOG_MISS_LIMIT);
    if (watchdog_missed_count >= WATCHDOG_MISS_LIMIT)
    {
      ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Restarting now (display hang)");
      esp_restart();
    }
  }
  else
  {
    last_display_task_heartbeat = display_task_heartbeat;
    watchdog_missed_count = 0;
  }
}

static int last_integration_update_hour = -1; // Track last hour we updated integration message
static time_t lastrun = 0;

static bool is_time_of_day_bright(void)
{
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);
  if (timeinfo.tm_year + 1900 <= 2025)
    return true;

  return is_time_in_range(eeprom_dim_start, eeprom_dim_end, timeinfo.tm_hour * 60 + timeinfo.tm_min);
}

static void handle_als_and_brightness(uint32_t loop_counter)
{
  (void)loop_counter; // cadence is now time-based, independent of loop rate
  static int64_t als_last_read_us = 0;
  uint8_t old_font_index = font_index;

  if (manufacturer_mode || eeprom_dim_mode == DIM_MODE_FULL)
  {
    font_index = 0;
  }
  else if (eeprom_dim_mode == DIM_MODE_TIME)
  {
    font_index = is_time_of_day_bright() ? 0 : 1;
  }
  else if ((esp_timer_get_time() - als_last_read_us) >= 5000000)
  {
    // read ALS sensor every 5 seconds (time-based, independent of loop period)
    als_last_read_us = esp_timer_get_time();
    lux = ltr303_get_frixos_lux();

    if (lux > eeprom_lux_threshold + eeprom_lux_sensitivity)
      font_index = 0;
    else if (lux < eeprom_lux_threshold - eeprom_lux_sensitivity)
      font_index = 1;
  }

  if (font_index != old_font_index)
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Font %u->%u (mode %u lux %.1f thres %.1f)",
                old_font_index, font_index, eeprom_dim_mode, lux, eeprom_lux_threshold);
    set_led_pwm_brightness(eeprom_brightness_LED[font_index]);
    display_changed();
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
      const bool wifi_enabled = eeprom_screen_layout.profile[font_index].widget[SCREEN_ELEM_WIFI_OFF].enabled;
      show_object(img_wifi, wifi_enabled);
      show_object(img_glucose, false);
      show_object(img_glucose_trend, false);
      ESP_LOG_WEB(ESP_LOG_INFO, TAG, "WiFi disabled (active hours)");
    }
    else
    {
      apply_widget_visibility(&eeprom_screen_layout.profile[font_index]);
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
      const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];
      // Gated on time_valid so the glucose icons stay hidden during the boot
      // splash (this block can run pre-sync via the boot-IP path).
      show_object(img_glucose, time_valid && layout->widget[SCREEN_ELEM_GLUCOSE_LEVEL].enabled && is_glucose_on());
      show_object(img_glucose_trend, time_valid && layout->widget[SCREEN_ELEM_GLUCOSE_TREND].enabled && is_glucose_fresh());
      lvgl_port_unlock();
    }
    else
    {
      lvgl_port_lock(0);
      show_object(img_glucose, false);
      show_object(img_glucose_trend, false);
      lvgl_port_unlock();
    }

    // IP address and OTA "updating..." / "reinstalling..." message display handling
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
      const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];
      replace_placeholders(layout->scroll_text, msg_scrolling, sizeof(msg_scrolling));
      lvgl_port_lock(0);
      set_scroll_message(msg_scrolling);
      update_static_text_labels();
      lvgl_port_unlock();
    }
    else if (ota_update_in_progress)
    {
      if (!ota_updating_message)
      {
        set_scroll_message(ota_reinstall_in_progress ? "Reinstalling..." : "Updating...");
        ota_updating_message = true;
      }
      if (ota_start_time == 0)
        ota_start_time = time(NULL);
      if (time(NULL) - ota_start_time > 300)
      {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "OTA timeout, restoring message");
        ota_update_in_progress = false;
        ota_reinstall_in_progress = false;
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

static void sync_schedule_runners(void)
{
  primary_runner.schedule = display_schedule;
  primary_runner.count = display_schedule_count;
  aux_runner.schedule = display_schedule_aux;
  aux_runner.count = display_schedule_aux_count;
}

static void advance_schedule_runner(schedule_runner_t *runner, time_t now, bool *mode_changed)
{
  runner->alternate_active = (runner->count > 1);

  if (!time_valid || runner->count == 0)
  {
    if (runner->showing_glucose || runner->showing_weather || runner->showing_ha)
      *mode_changed = true;
    runner->showing_glucose = false;
    runner->showing_weather = false;
    runner->showing_ha = false;
    runner->current_idx = 0;
    runner->slot_start = 0;
    return;
  }

  if (runner->slot_start == 0)
  {
    runner->current_idx = 0;
    runner->slot_start = now;
    *mode_changed = true;
  }
  else if (runner->count > 1)
  {
    const display_slot_t *cur = &runner->schedule[runner->current_idx];
    if ((now - runner->slot_start) >= cur->duration)
    {
      int next = (runner->current_idx + 1) % runner->count;
      int tries = runner->count;
      while (tries-- > 0)
      {
        if (slot_is_available(&runner->schedule[next]))
        {
          runner->current_idx = next;
          runner->slot_start = now;
          *mode_changed = true;
          break;
        }
        next = (next + 1) % runner->count;
      }
    }
  }

  slot_type_t st = runner->schedule[runner->current_idx].type;
  bool new_glucose = (st == SLOT_TYPE_CGM);
  bool new_weather = (st == SLOT_TYPE_WEATHER_TEMP);
  bool new_ha = (st == SLOT_TYPE_HA);
  if (new_glucose != runner->showing_glucose || new_weather != runner->showing_weather || new_ha != runner->showing_ha)
    *mode_changed = true;
  runner->showing_glucose = new_glucose;
  runner->showing_weather = new_weather;
  runner->showing_ha = new_ha;
}

static void handle_alternate_mode_switching(time_t now, uint32_t loop_counter, bool *should_update_display)
{
  (void)loop_counter; // cadence is now time-based, independent of loop rate
  static int64_t alt_last_us = 0;
  int64_t alt_now_us = esp_timer_get_time();
  if ((alt_now_us - alt_last_us) < 1000000) return; // ~1s cadence
  alt_last_us = alt_now_us;

  static bool last_primary_glucose = false;
  static bool last_primary_weather = false;
  static bool last_primary_ha = false;
  static time_t last_minute_slot = 0;
  time_t current_minute_slot = now / 60;
  bool minute_changed = (current_minute_slot != last_minute_slot);
  bool mode_changed = false;

  sync_schedule_runners();
  advance_schedule_runner(&primary_runner, now, &mode_changed);
  advance_schedule_runner(&aux_runner, now, &mode_changed);

  if (mode_changed
      || primary_runner.showing_glucose != last_primary_glucose
      || primary_runner.showing_weather != last_primary_weather
      || primary_runner.showing_ha != last_primary_ha)
  {
    *should_update_display = true;
    last_primary_glucose = primary_runner.showing_glucose;
    last_primary_weather = primary_runner.showing_weather;
    last_primary_ha = primary_runner.showing_ha;

    if (mode_changed)
      update_weather_msg();
  }
  else if (!primary_runner.showing_glucose && !primary_runner.showing_weather && !primary_runner.showing_ha)
  {
    *should_update_display = ((minute_changed || (now % 60 == 0) || (time_just_validated == 1) || weather_has_updated) && time_valid && (now - lastrun > 3));
    last_minute_slot = current_minute_slot;
  }
}

static void update_one_digit_display(const screen_layout_profile_t *layout,
                                     const schedule_runner_t *runner,
                                     screen_element_id_t elem,
                                     lv_obj_t *digits[NUM_DIGITS],
                                     lv_obj_t *dot_objs[2],
                                     lv_obj_t *mgdl_img,
                                     lv_obj_t *unit_label,
                                     bool *show_ampm_out)
{
  const bool show_time = layout_show_time_digits(layout, runner, elem);
  const bool show_glucose = layout_show_glucose_on_digits(layout, runner, elem);
  const bool show_weather = layout_show_weather_on_digits(layout, runner, elem);
  const bool show_ha = layout_show_ha_on_digits(layout, runner, elem);
  const bool show_slot_digits = show_time || show_glucose || show_weather || show_ha;

  digit_display_t digits_data = {0};
  digit_display_t value_digits = {0};
  const char *value_unit = "";

  if (show_glucose)
    fill_glucose_digits(&digits_data, true);
  else if (show_weather)
  {
    fill_weather_digits(&value_digits);
    value_unit = "\xc2\xb0";
    digits_data = value_digits;
  }
  else if (show_ha)
  {
    const char *ha_val = get_ha_slot_value(runner->schedule[runner->current_idx].entity);
    fill_ha_digits(&value_digits, ha_val);
    value_unit = runner->schedule[runner->current_idx].label;
    digits_data = value_digits;
  }
  else if (show_time)
  {
    struct tm local = timeinfo;
    bool is_pm = false;
    if (eeprom_12hour)
    {
      if (local.tm_hour == 0)
        local.tm_hour = 12;
      else if (local.tm_hour > 12)
      {
        local.tm_hour -= 12;
        is_pm = true;
      }
    }
    fill_time_digits(&digits_data, &local);
    if (show_ampm_out && elem == SCREEN_ELEM_TIME)
      *show_ampm_out = eeprom_12hour && is_pm;
  }

  if (show_glucose)
    render_digit_group(digits, &digits_data);
  else if (show_weather || show_ha)
    render_digit_group(digits, &value_digits);
  else if (show_time)
    render_digit_group(digits, &digits_data);

  if (screen_layout_positions_live)
  {
    const screen_widget_t *w = &layout->widget[elem];
    const int ox = layout_abs_x(w);
    const int y_digits = layout_abs_y(w);

    if (show_glucose)
      align_glucose_digits(digits, dot_objs, ox, y_digits, &digits_data);
    else if (show_weather || show_ha)
      align_value_with_label_digits(digits, dot_objs, ox, y_digits, &value_digits, value_unit, unit_label);
    else if (show_time)
      align_time_style_digits(digits, dot_objs, ox, y_digits, &digits_data, true);
  }

  for (int i = 0; i < 4; i++)
    show_object(digits[i], show_slot_digits);
  show_object(dot_objs[0], show_time || show_glucose);
  show_object(dot_objs[1], show_time || show_glucose);
  update_cgm_dots_visibility(dot_objs, show_glucose, show_glucose ? &digits_data : NULL);
  show_object(mgdl_img, show_glucose);
  lv_image_set_offset_x(mgdl_img, -eeprom_glucose_unit * 12);
  show_object(unit_label, show_weather || (show_ha && value_unit[0] != '\0'));
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

  sync_schedule_runners();
  const screen_layout_profile_t *layout = &eeprom_screen_layout.profile[font_index];
  if (weather_has_updated || (timeinfo.tm_min == 1))
  {
    update_weather_msg();
    weather_has_updated = false;
  }

  bool show_ampm = false;

  lvgl_port_lock(0);
  update_one_digit_display(layout, &primary_runner, SCREEN_ELEM_TIME,
                           digit_objs, dots, img_mgdl, label_degree, &show_ampm);
  update_one_digit_display(layout, &aux_runner, SCREEN_ELEM_TIME_AUX,
                           aux_digit_objs, aux_dots, img_mgdl_aux, label_degree_aux, NULL);
  update_digit_label_widgets();
  show_object(img_ampm, layout_show_time_digits(layout, &primary_runner, SCREEN_ELEM_TIME) && eeprom_12hour);

  lv_image_set_offset_x(img_weather, -weather_icon_index * 32);
  lv_image_set_offset_x(img_moon, -moon_icon_index * 14);
  lv_image_set_offset_x(img_ampm, show_ampm ? -10 : 0);
  lvgl_port_unlock();

  last_minute = timeinfo.tm_min;

  if (time_just_validated == 1)
  {
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "Time validated, showing clock");
    time_just_validated = 0;
    lvgl_port_lock(0);
    if (img_logo) lv_obj_add_flag(img_logo, LV_OBJ_FLAG_HIDDEN);
    apply_widget_visibility(layout);
    update_static_text_labels();
    update_digit_label_widgets();
    lvgl_port_unlock();
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
  // Start the watchdog timer periodically (every 30 seconds).
  // The display_task will increment a heartbeat counter, and this timer's callback
  // will verify that the counter is still advancing.
  esp_timer_start_periodic(watchdog_timer, 30000000);

  TickType_t lastrun_tick = xTaskGetTickCount();
  time_t now;
  static uint32_t loop_counter = 0;

  while (1)
  {
    loop_counter++;
    if (loop_counter % 1000 == 1)
    {
      size_t stack_free_bytes = uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t);
      ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Display loop %lu, stack %u bytes", loop_counter, (unsigned)stack_free_bytes);
      if (stack_free_bytes < 1024) ESP_LOG_WEB(ESP_LOG_WARN, TAG, "Low stack: %u bytes", (unsigned)stack_free_bytes);
    }

    time(&now);

    if (settings_updated)
    {
      ESP_LOG_WEB(ESP_LOG_VERBOSE, TAG, "Settings updated");
      display_changed();
      update_display_content(now);
      settings_updated = false;
    }

    handle_screen_layout_on_wifi();
    handle_wifi_status_icon();
    handle_integration_and_messages();
    handle_als_and_brightness(loop_counter);
    update_graph(); // self-guards: only redraws when a new sample arrived

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

      // Bolt Optimization: Guard against redundant LVGL style updates and port locks.
      // Breathing is often disabled, yet the fade timer continues to trigger every 200ms.
      static int last_op = -1;
      static bool last_disabled = false;
      bool currently_disabled = (eeprom_dots_breathe == 1 || primary_runner.showing_glucose);

      if (currently_disabled)
      {
        if (!last_disabled)
        {
          // Breathing newly disabled - show dots at full brightness once
          lvgl_port_lock(0);
          lv_obj_set_style_opa(dots[0], 255, LV_PART_MAIN);
          lv_obj_set_style_opa(dots[1], 255, LV_PART_MAIN);
          lvgl_port_unlock();
          last_disabled = true;
          last_op = 255;
        }
      }
      else
      {
        // Breathing enabled - update only if opacity changed or if we were previously disabled
        if (opacity != last_op || last_disabled)
        {
          lvgl_port_lock(0);
          lv_obj_set_style_opa(dots[0], opacity, LV_PART_MAIN);
          lv_obj_set_style_opa(dots[1], 255 - opacity, LV_PART_MAIN);
          lvgl_port_unlock();
          last_op = opacity;
          last_disabled = false;
        }
      }
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

    if (label_size > MSG_CENTER_WIDTH)
    { // scrolling — time-based seamless belt.
      // Both labels carry the same text and sit scroll_period apart, so the
      // leftmost copy can move continuously left and the wrap is invisible
      // (no role swap needed).
      int scroll_period = label_max_pos + 20;
      if (scroll_period < 1) scroll_period = 1;
      uint32_t delay_ms = screen_scroll_delay_ms(); // milliseconds per pixel
      if (delay_ms < 1) delay_ms = 1;

      // #1 time-based: the offset is a pure function of elapsed wall-clock time,
      //    so loop-timing jitter and tick quantization never reach the motion.
      // #2 high-rate sampling: this runs every SCROLL_LOOP_MS pass (well above the
      //    panel refresh), so a 1px step lands the instant elapsed time crosses
      //    delay_ms. We only touch LVGL when the integer pixel actually changes.
      int64_t traveled = (esp_timer_get_time() - scroll_t0_us) / ((int64_t)delay_ms * 1000);
      int64_t pos = (int64_t)scroll_seed_px - traveled;
      int ipos = (int)(pos % scroll_period);
      if (ipos > 0) ipos -= scroll_period; // normalize into (-period, 0]

      if (!scroll_last_valid || ipos != scroll_last_ipos)
      {
        scroll_last_ipos = ipos;
        scroll_last_valid = true;
        const screen_widget_t *w_msg = active_screen_widget(SCREEN_ELEM_MESSAGE);
        const int msg_x = screen_layout_positions_live ? layout_abs_x(w_msg) : BOOT_MSG_X;
        const int msg_y = screen_layout_positions_live ? layout_abs_y(w_msg) : BOOT_MSG_Y;
        lvgl_port_lock(0);
        lv_obj_set_pos(label_msg,      msg_x + ipos,                msg_y);
        lv_obj_set_pos(label_msg_loop, msg_x + ipos + scroll_period, msg_y);
        lvgl_port_unlock();
      }
    }

    // NOTE: Do NOT call lv_task_handler() here! The esp_lvgl_port creates its own
    // task that runs lv_timer_handler. Calling it from display_task causes two tasks
    // to run the LVGL timer handler, corrupting the event list and crashing in
    // lv_event_mark_deleted when objects are deleted. See LVGL issue #6677.

    // Bolt Optimization: Feed the watchdog using a lightweight heartbeat counter.
    // This replaces the overhead-heavy esp_timer_stop/start_periodic calls
    // which were executing every ~65ms in the high-frequency display loop.
    display_task_heartbeat++;

    // Fixed, fast loop period (decoupled from scroll speed). Scroll speed is now
    // expressed purely as time in the belt math above, so the loop runs at a
    // steady high rate and only the per-pixel duration changes with the setting.
    xTaskDelayUntil(&lastrun_tick, pdMS_TO_TICKS(SCROLL_LOOP_MS));
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
  size_t len; // Optimization: store token length to avoid repeated strlen calls
} token_t;

// Define base tokens array
static const token_t base_tokens[] = {
    {"[device]", NULL, 1, TOKEN_TYPE_BASE, 8},
    {"[greeting]", NULL, 2, TOKEN_TYPE_BASE, 10},
    {"[day]", NULL, 3, TOKEN_TYPE_TIME, 5},
    {"[date]", NULL, 4, TOKEN_TYPE_TIME, 6},
    {"[mon]", NULL, 5, TOKEN_TYPE_TIME, 5},
    {"[temp]", NULL, 6, TOKEN_TYPE_WEATHER, 6},
    {"[hum]", NULL, 7, TOKEN_TYPE_WEATHER, 5},
    {"[high]", NULL, 8, TOKEN_TYPE_WEATHER, 6},
    {"[low]", NULL, 9, TOKEN_TYPE_WEATHER, 5},
    {"[rise]", NULL, 10, TOKEN_TYPE_WEATHER, 6},
    {"[set]", NULL, 11, TOKEN_TYPE_WEATHER, 5},
    {"[wind]", NULL, 12, TOKEN_TYPE_WEATHER, 6},
    {"[gust]", NULL, 13, TOKEN_TYPE_WEATHER, 6},
    {"[precip]", NULL, 14, TOKEN_TYPE_WEATHER, 8},
    {"[uv]", NULL, 15, TOKEN_TYPE_WEATHER, 4},
    {"[pressure]", NULL, 16, TOKEN_TYPE_WEATHER, 10},
    {"[3high]", NULL, 17, TOKEN_TYPE_WEATHER, 7},
    {"[3low]", NULL, 18, TOKEN_TYPE_WEATHER, 6},
    {"[time]", NULL, 19, TOKEN_TYPE_TIME, 6},
    {"[hour12]", NULL, 20, TOKEN_TYPE_TIME, 8},
    {"[hour24]", NULL, 21, TOKEN_TYPE_TIME, 8},
    {"[min]", NULL, 22, TOKEN_TYPE_TIME, 5},
    {"[ampm]", NULL, 23, TOKEN_TYPE_TIME, 6},
    {NULL, NULL, 0, TOKEN_TYPE_BASE, 0} // End marker
};

// Global token list that persists between calls
static const token_t *prepared_tokens = NULL;
static int prepared_tokens_count = 0;

// Case-insensitive equality for short literals (boolean token mapping).
static bool ci_eq(const char *a, const char *b)
{
  for (; *a && *b; a++, b++)
  {
    char ca = (*a >= 'A' && *a <= 'Z') ? (char)(*a + 32) : *a;
    char cb = (*b >= 'A' && *b <= 'Z') ? (char)(*b + 32) : *b;
    if (ca != cb)
      return false;
  }
  return *a == '\0' && *b == '\0';
}

// Parse an HA/stock string value to a number. Booleans -> 1/0. Numeric strings
// (optionally with a trailing unit, e.g. "21.3 °C") parse via strtof.
static bool parse_string_value(const char *s, float *out)
{
  if (!s || !s[0])
    return false;
  if (ci_eq(s, "on") || ci_eq(s, "true") || ci_eq(s, "open") || ci_eq(s, "yes") || ci_eq(s, "home"))
  {
    *out = 1.0f;
    return true;
  }
  if (ci_eq(s, "off") || ci_eq(s, "false") || ci_eq(s, "closed") || ci_eq(s, "no") || ci_eq(s, "away"))
  {
    *out = 0.0f;
    return true;
  }
  char *end = NULL;
  float v = strtof(s, &end);
  if (end == s)
    return false; // no leading number
  *out = v;
  return true;
}

// Resolve a token string to its current numeric value, for the graph widget.
// Returns false if the token is unknown or has no numeric value right now.
bool token_numeric_value(const char *tok, float *out)
{
  if (!tok || !out || !prepared_tokens)
    return false;
  for (int i = 0; i < prepared_tokens_count; i++)
  {
    const token_t *t = &prepared_tokens[i];
    if (!t->token || strcmp(t->token, tok) != 0)
      continue;
    switch (t->type)
    {
    case TOKEN_TYPE_WEATHER:
    {
      double v;
      switch (t->id)
      {
      case 6: v = eeprom_fahrenheit ? (weather_temp * 9.0 / 5.0) + 32 : weather_temp; break;       // [temp]
      case 7: v = weather_humidity; break;                                                          // [hum]
      case 8: v = eeprom_fahrenheit ? (weather_high * 9.0 / 5.0) + 32 : weather_high; break;        // [high]
      case 9: v = eeprom_fahrenheit ? (weather_low * 9.0 / 5.0) + 32 : weather_low; break;          // [low]
      case 12: v = weather_wind_speed_mps; break;                                                   // [wind]
      case 13: v = weather_gust_mps; break;                                                         // [gust]
      case 14: v = weather_precip_mm; break;                                                        // [precip]
      case 15: v = weather_uv; break;                                                               // [uv]
      case 16: v = weather_pressure_hpa; break;                                                     // [pressure]
      case 17: v = eeprom_fahrenheit ? (weather_3day_high * 9.0 / 5.0) + 32 : weather_3day_high; break; // [3high]
      case 18: v = eeprom_fahrenheit ? (weather_3day_low * 9.0 / 5.0) + 32 : weather_3day_low; break;   // [3low]
      default: return false;                                                                        // rise/set are times
      }
      *out = (float)v;
      return true;
    }
    case TOKEN_TYPE_GLUCOSE:
      if (glucose_data.current_gl_mgdl > 0)
      {
        *out = glucose_data.current_gl_mgdl;
        return true;
      }
      return false;
    case TOKEN_TYPE_HA:
    case TOKEN_TYPE_STOCK:
      return parse_string_value(t->value, out);
    default:
      return false; // BASE / TIME tokens are not graphable
    }
  }
  return false;
}

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
  prepared_tokens_count = 23; // Base tokens count (incl. time tokens and met.no extended weather)

  // Allocate space for all tokens
  int tokencount = 0;
  for (int i = 0; i < AVAILABLE_INTEGRATIONS; i++)
  {
    if (integration_active_tokens[i] != NULL && integration_active_tokens_count[i] > 0)
      tokencount += integration_active_tokens_count[i];
  }

  // Add space for CGM tokens (3 per active CGM integration: [CGM:glucose], [CGM:reading], [CGM:time])
  // These tokens are always added when integration is active, regardless of token array
  if (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT])
  {
    tokencount += 3;
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
    ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "Token alloc failed, free heap %u",
                (unsigned)esp_get_free_heap_size());
    prepared_tokens = base_tokens;
    prepared_tokens_count = 23;
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
      all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
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
      all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
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
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;

    // Add [CGM:time] token
    all_tokens[prepared_tokens_count].token = "[CGM:time]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL;
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
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
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;

    // Add [CGM:time] token
    all_tokens[prepared_tokens_count].token = "[CGM:time]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL;
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;
  }
  else if (integration_active[INTEGRATION_NIGHTSCOUT])
  {
    // Add [CGM:glucose] token
    all_tokens[prepared_tokens_count].token = "[CGM:glucose]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;
    
    // Add [CGM:reading] token (will be formatted differently in replacement)
    all_tokens[prepared_tokens_count].token = "[CGM:reading]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL; // Will be formatted on-demand in replace_placeholders
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;

    // Add [CGM:time] token
    all_tokens[prepared_tokens_count].token = "[CGM:time]";
    all_tokens[prepared_tokens_count].id = prepared_tokens_count + 1;
    all_tokens[prepared_tokens_count].type = TOKEN_TYPE_GLUCOSE;
    all_tokens[prepared_tokens_count].value = NULL;
    all_tokens[prepared_tokens_count].len = strlen(all_tokens[prepared_tokens_count].token);
    prepared_tokens_count++;
  }

  // Add end marker
  all_tokens[prepared_tokens_count].token = NULL;
  all_tokens[prepared_tokens_count].id = 0;
  all_tokens[prepared_tokens_count].type = TOKEN_TYPE_BASE;
  all_tokens[prepared_tokens_count].len = 0;

  // Update prepared tokens
  prepared_tokens = (const token_t *)all_tokens;
}

// Function to replace tokens in a string
// Optimization: Direct iteration over input, pre-calculated token lengths, and O(N) lookup for dynamic values.
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

  const char *src = input;
  char *dst = output;
  char *end = output + output_size - 1;

  // Get current local time for greeting calculation
  time_t now_time;
  struct tm current_timeinfo;
  time(&now_time);
  localtime_r(&now_time, &current_timeinfo);

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

  // Initialize output buffer
  output[0] = '\0';

  while (*src && dst < end)
  {
    if (*src == '[')
    {
      // Check if we have a matching token
      const token_t *token = prepared_tokens;
      while (token->token)
      {
        if (strncmp(src, token->token, token->len) == 0)
        {
          // Found a match, handle it in switch
          char replacement[64];
          replacement[0] = '-';
          replacement[1] = '\0';

          switch (token->type)
          {
          case TOKEN_TYPE_BASE:
            switch (token->id)
            {
            case 1: // [device]
              strlcpy(replacement, eeprom_hostname, sizeof(replacement));
              break;
            case 2: // [greeting]
              strlcpy(replacement, greeting, sizeof(replacement));
              break;
            }
            break;

          case TOKEN_TYPE_TIME:
            switch (token->id)
            {
            case 3: // [day]
              strlcpy(replacement, day_names[lang_index][timeinfo.tm_wday], sizeof(replacement));
              break;
            case 4: // [date]
              snprintf(replacement, sizeof(replacement), "%d", timeinfo.tm_mday);
              break;
            case 5: // [mon]
              strlcpy(replacement, month_names[lang_index][timeinfo.tm_mon], sizeof(replacement));
              break;
            case 19: // [time]
              format_local_time(replacement, sizeof(replacement), &timeinfo);
              break;
            case 20: // [hour12]
              snprintf(replacement, sizeof(replacement), "%d", time_hour12(&timeinfo));
              break;
            case 21: // [hour24]
              snprintf(replacement, sizeof(replacement), "%02d", timeinfo.tm_hour);
              break;
            case 22: // [min]
              snprintf(replacement, sizeof(replacement), "%02d", timeinfo.tm_min);
              break;
            case 23: // [ampm]
              strlcpy(replacement, time_localized_ampm(&timeinfo), sizeof(replacement));
              break;
            }
            break;

          case TOKEN_TYPE_WEATHER:
            if (!weather_valid)
            {
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
            case 12: // [wind] — speed + cardinal direction
            {
              static const char *cardinals[] = {"N","NE","E","SE","S","SW","W","NW"};
              const char *dir = cardinals[((weather_wind_dir_deg + 22) / 45) & 7];
              if (eeprom_fahrenheit)
              {
                // m/s -> mph (1 m/s = 2.236936 mph)
                snprintf(replacement, sizeof(replacement), "%.1f mph %s",
                         weather_wind_speed_mps * 2.236936, dir);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.1f m/s %s",
                         weather_wind_speed_mps, dir);
              }
            }
            break;
            case 13: // [gust] — gust speed + same direction
            {
              static const char *cardinals[] = {"N","NE","E","SE","S","SW","W","NW"};
              const char *dir = cardinals[((weather_wind_dir_deg + 22) / 45) & 7];
              if (eeprom_fahrenheit)
              {
                snprintf(replacement, sizeof(replacement), "%.1f mph %s",
                         weather_gust_mps * 2.236936, dir);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.1f m/s %s",
                         weather_gust_mps, dir);
              }
            }
            break;
            case 14: // [precip] — next-hour amount + probability
              if (eeprom_fahrenheit)
              {
                // mm -> inches (1 mm = 0.0393701 in)
                snprintf(replacement, sizeof(replacement), "%.2f in. (%.0f%%)",
                         weather_precip_mm * 0.0393701, weather_precip_prob);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.1f mm (%.0f%%)",
                         weather_precip_mm, weather_precip_prob);
              }
              break;
            case 15: // [uv]
              snprintf(replacement, sizeof(replacement), "%.1f", weather_uv);
              break;
            case 16: // [pressure] — integer hPa or inHg + trend arrow
            {
              const char *arrow = (weather_pressure_trend > 0) ? " ↑" :
                                  (weather_pressure_trend < 0) ? " ↓" : " →";
              if (eeprom_fahrenheit)
              {
                // hPa -> inHg (1 hPa = 0.02953 inHg). Two decimals are conventional.
                snprintf(replacement, sizeof(replacement), "%.2f inHg%s",
                         weather_pressure_hpa * 0.02953, arrow);
              }
              else
              {
                snprintf(replacement, sizeof(replacement), "%.0f hPa%s",
                         weather_pressure_hpa, arrow);
              }
            }
            break;
            case 17: // [3high] — max temp across today + 2 days
              if (eeprom_fahrenheit)
                snprintf(replacement, sizeof(replacement), "%.0f°F", (weather_3day_high * 9.0 / 5.0) + 32);
              else
                snprintf(replacement, sizeof(replacement), "%.0f°C", weather_3day_high);
              break;
            case 18: // [3low] — min temp across today + 2 days
              if (eeprom_fahrenheit)
                snprintf(replacement, sizeof(replacement), "%.0f°F", (weather_3day_low * 9.0 / 5.0) + 32);
              else
                snprintf(replacement, sizeof(replacement), "%.0f°C", weather_3day_low);
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
          case TOKEN_TYPE_STOCK:
            // Optimization: dynamic values are already linked in prepare_tokens
            if (token->value && token->value[0] != '\0')
            {
              strlcpy(replacement, token->value, sizeof(replacement));
            }
            break;

          case TOKEN_TYPE_GLUCOSE:
            // [CGM:reading] (plain), [CGM:time], or [CGM:glucose] (formatted)
            // Format directly from glucose_data (no token arrays for Dexcom/Freestyle/Nightscout)
            if (integration_active[INTEGRATION_DEXCOM] || integration_active[INTEGRATION_FREESTYLE] || integration_active[INTEGRATION_NIGHTSCOUT])
            {
              if (glucose_data.current_gl_mgdl > 0)
              {
                if (strcmp(token->token, "[CGM:reading]") == 0)
                {
                  get_glucose_reading_plain(replacement, sizeof(replacement));
                }
                else if (strcmp(token->token, "[CGM:time]") == 0)
                {
                  format_glucose_time_token(replacement, sizeof(replacement));
                }
                else
                {
                  format_glucose_token(replacement, sizeof(replacement));
                }
              }
            }
            break;
          }

          // Copy the replacement to output with bounds checking
          size_t rem = end - dst + 1;
          size_t rlen = strlen(replacement);
          if (rlen < rem)
          {
            memcpy(dst, replacement, rlen);
            dst += rlen;
          }
          else if (rem > 1)
          {
            memcpy(dst, replacement, rem - 1);
            dst += rem - 1;
          }
          src += token->len;
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
#define CACHE_SIZE 128
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
  // Guard against start_pixel beyond text end (gap phase of infinite loop)
  int32_t substring_len = end_char_index - start_char_index;
  if (substring_len <= 0 || !start_found)
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

  // Set only the substring text (this is a key optimization!)
  // Bolt Optimization: Only call lv_label_set_text if the content actually changed.
  // This avoids expensive internal LVGL re-parsing and layout during scrolling.
  if (strcmp(lv_label_get_text(label_obj), substring_buffer) != 0)
  {
    lv_label_set_text(label_obj, substring_buffer);
  }

  // Position changes every frame during scrolling; set directly
  int32_t new_x = x - char_offset + padding_offset;
  lv_obj_set_pos(label_obj, new_x, y);
  lv_obj_set_width(label_obj, width_pixels);

  lvgl_port_unlock();
}
