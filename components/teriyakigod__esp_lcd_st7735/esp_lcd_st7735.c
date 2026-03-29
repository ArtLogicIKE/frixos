#include <stdlib.h>
#include <sys/cdefs.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"

#include "esp_lcd_st7735.h"

static const char *TAG = "st7735";

static esp_err_t panel_st7735_del(esp_lcd_panel_t *panel);
static esp_err_t panel_st7735_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_st7735_init(esp_lcd_panel_t *panel);
static esp_err_t panel_st7735_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data);
static esp_err_t panel_st7735_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_st7735_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_st7735_swap_xy(esp_lcd_panel_t *panel, bool swap_axes);
static esp_err_t panel_st7735_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap);
static esp_err_t panel_st7735_disp_on_off(esp_lcd_panel_t *panel, bool off);

typedef struct
{
    esp_lcd_panel_t base;
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    bool reset_level;
    int x_gap;
    int y_gap;
    uint8_t fb_bits_per_pixel;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save current value of LCD_CMD_COLMOD register
    const st7735_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t color_channel;
} st7735_panel_t;

esp_err_t esp_lcd_new_panel_st7735(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel)
{
    esp_err_t ret = ESP_OK;
    st7735_panel_t *st7735 = NULL;
    gpio_config_t io_conf = {0};

    ESP_GOTO_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    st7735 = (st7735_panel_t *)calloc(1, sizeof(st7735_panel_t));
    ESP_GOTO_ON_FALSE(st7735, ESP_ERR_NO_MEM, err, TAG, "no mem for st7735 panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num;
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->rgb_endian)
    {
    case LCD_RGB_ENDIAN_RGB:
        st7735->madctl_val = 0;
        break;
    case LCD_RGB_ENDIAN_BGR:
        st7735->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported rgb endian");
        break;
    }

    switch (panel_dev_config->bits_per_pixel)
    {
    case 16: // RGB565
        st7735->colmod_val = 0x55;
        st7735->fb_bits_per_pixel = 16;
        break;
    case 18: // RGB666
        st7735->colmod_val = 0x66;
        // each color component (R/G/B) should occupy the 6 high bits of a byte, which means 3 full bytes are required for a pixel
        st7735->fb_bits_per_pixel = 24;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    st7735->io = io;
    st7735->reset_gpio_num = panel_dev_config->reset_gpio_num;
    st7735->reset_level = panel_dev_config->flags.reset_active_high;
    if (panel_dev_config->vendor_config)
    {
        st7735->init_cmds = ((st7735_vendor_config_t *)panel_dev_config->vendor_config)->init_cmds;
        st7735->init_cmds_size = ((st7735_vendor_config_t *)panel_dev_config->vendor_config)->init_cmds_size;
    }
    st7735->base.del = panel_st7735_del;
    st7735->base.reset = panel_st7735_reset;
    st7735->base.init = panel_st7735_init;
    st7735->base.draw_bitmap = panel_st7735_draw_bitmap;
    st7735->base.invert_color = panel_st7735_invert_color;
    st7735->base.set_gap = panel_st7735_set_gap;
    st7735->base.mirror = panel_st7735_mirror;
    st7735->base.swap_xy = panel_st7735_swap_xy;
    // #if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    //     st7735->base.disp_off = panel_st7735_disp_on_off;
    // #else
    //     st7735->base.disp_on_off = panel_st7735_disp_on_off;
    // #endif
    st7735->base.disp_on_off = panel_st7735_disp_on_off;
    st7735->color_channel = 0; // no color channel, show it all
    *ret_panel = &(st7735->base);
    ESP_LOGD(TAG, "new st7735 panel @%p", st7735);
    ESP_LOGI(TAG, "LCD panel create success");

    return ESP_OK;

err:
    if (st7735)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(st7735);
    }
    return ret;
}

static esp_err_t panel_st7735_del(esp_lcd_panel_t *panel)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);

    if (st7735->reset_gpio_num >= 0)
    {
        gpio_reset_pin(st7735->reset_gpio_num);
    }
    ESP_LOGD(TAG, "del st7735 panel @%p", st7735);
    free(st7735);
    return ESP_OK;
}

static esp_err_t panel_st7735_reset(esp_lcd_panel_t *panel)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    ESP_LOGD(TAG, "reset st7735 panel @%p", st7735);
    // perform hardware reset
    if (st7735->reset_gpio_num >= 0)
    {
        gpio_set_level(st7735->reset_gpio_num, st7735->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(st7735->reset_gpio_num, !st7735->reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    else
    { // perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20)); // spec, wait at least 5ms before sending new command
    }

    return ESP_OK;
}

typedef struct
{
    uint8_t cmd;
    uint8_t data[16];
    uint8_t data_bytes; // Length of data in above data array; 0xFF = end of cmds.
} lcd_init_cmd_t;

const uint8_t TFT_INIT_DELAY = 0;

st7735_lcd_init_cmd_t st7735_init_cmds[] = {
    // Rcmd1
    {ST7735_SWRESET, (uint8_t[]){0x00}, 1, 150},                                                                                          // Software reset, 150 ms delay
    {ST7735_SLPOUT, (uint8_t[]){0x00}, 1, 255},                                                                                           // Out of sleep mode, 255 ms delay
    {ST7735_FRMCTR1, (uint8_t[]){0x01, 0x2C, 0x2D}, 3, 0},                                                                                // Frame rate ctrl - normal mode
    {ST7735_FRMCTR2, (uint8_t[]){0x01, 0x2C, 0x2D}, 3, 0},                                                                                // Frame rate control - idle mode
    {ST7735_FRMCTR3, (uint8_t[]){0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D}, 6, 0},                                                              // Frame rate ctrl - partial mode
    {ST7735_INVCTR, (uint8_t[]){0x07}, 1, 0},                                                                                             // Display inversion ctrl
    {ST7735_PWCTR1, (uint8_t[]){0xA2, 0x02, 0x84}, 3, 0},                                                                                 // Power control
    {ST7735_PWCTR2, (uint8_t[]){0xC5}, 1, 0},                                                                                             // Power control
    {ST7735_PWCTR3, (uint8_t[]){0x0A, 0x00}, 2, 0},                                                                                       // Power control
    {ST7735_PWCTR4, (uint8_t[]){0x8A, 0x2A}, 2, 0},                                                                                       // Power control
    {ST7735_PWCTR5, (uint8_t[]){0x8A, 0xEE}, 2, 0},                                                                                       // Power control
    {ST7735_VMCTR1, (uint8_t[]){0x0E}, 1, 0},                                                                                             // Power control
    {ST7735_INVOFF, (uint8_t[]){0x00}, 1, 0},                                                                                             // Don't invert display
    {ST7735_COLMOD, (uint8_t[]){0x05}, 1, 0},                                                                                             // Set color mode (16-bit)
    {ST7735_GMCTRP1, (uint8_t[]){0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d, 0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10}, 16, 0}, // Positive Gamma
    {ST7735_GMCTRN1, (uint8_t[]){0x03, 0x1d, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D, 0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10}, 16, 0}, // Negative Gamma
    {ST7735_NORON, (uint8_t[]){0x00}, 1, TFT_INIT_DELAY},                                                                                 // Normal display on, no args, w/delay 10 ms delay
    {ST7735_DISPON, (uint8_t[]){0x00}, 1, TFT_INIT_DELAY},
};

static esp_err_t panel_st7735_init(esp_lcd_panel_t *panel)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    ESP_LOGI(TAG, "init st7735 panel @%p", st7735);
    // LCD goes into sleep mode and display will be turned off after power on reset, exit sleep mode first
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SLPOUT, NULL, 0), TAG, "send command failed");
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){
                                                                          st7735->madctl_val,
                                                                      },
                                                  1),
                        TAG, "send command failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_COLMOD, (uint8_t[]){
                                                                          st7735->colmod_val,
                                                                      },
                                                  1),
                        TAG, "send command failed");

    const st7735_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    if (st7735->init_cmds)
    {
        init_cmds = st7735->init_cmds;
        init_cmds_size = st7735->init_cmds_size;
    }
    else
    {
        init_cmds = st7735_init_cmds;
        init_cmds_size = sizeof(st7735_init_cmds) / sizeof(st7735_lcd_init_cmd_t);
    }

    bool is_cmd_overwritten = false;
    ESP_LOGV(TAG, "Command Count == %d", init_cmds_size);
    for (int i = 0; i < init_cmds_size; i++)
    {
        ESP_LOGV(TAG, "cmd %02x", init_cmds[i].cmd);
        // Check if the command has been used or conflicts with the internal
        switch (init_cmds[i].cmd)
        {
        case LCD_CMD_MADCTL:
            is_cmd_overwritten = true;
            st7735->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
            break;
        case LCD_CMD_COLMOD:
            is_cmd_overwritten = true;
            st7735->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
            break;
        default:
            is_cmd_overwritten = false;
            break;
        }

        if (is_cmd_overwritten)
        {
            ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence", init_cmds[i].cmd);
        }

        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
    }
    ESP_LOGV(TAG, "send init commands success");

    return ESP_OK;
}

static uint16_t rgb565_to_brightness_channel(uint16_t color16, uint8_t channel)
{
    uint16_t luma, result;
    uint8_t rc, gc, bc;

    // rc = (us >> 10) & 0x3E;                     /* 6-bit Red Component. */
    // gc = (us >> 5) & 0x3F;                      /* 6-bit Green Component. */
    // bc = (us << 1) & 0x3E;                      /* 6-bit Blue Component. */

     uint16_t color16_swapped = __builtin_bswap16(color16); // swap bytes
  // uint16_t color16_swapped = color16;
    rc = ((color16_swapped >> 10) & 0x3E);                     /* 6-bit Red Component. */
    gc = ((color16_swapped >> 5) & 0x3F);                      /* 6-bit Green Component. */
    bc = ((color16_swapped << 1) & 0x3E);                      /* 6-bit Blue Component. */


/*
    // ESP32 is little endian, so we need to swap the bytes
    uint8_t highByte = color16 & 0xFF;
    uint8_t lowByte = (color16 >> 8) & 0xFF;

    // Extract components directly from little-endian layout
    rc = ((highByte >> 3) & 0x1F) << 1;             // 5+1 bits for red (top 5 bits of high byte)
    gc = ((highByte & 0x07) << 3) | (lowByte >> 5); // 3 bits from high byte + 3 bits from low byte
    bc = (lowByte & 0x1F) << 1; // 5+1 bits for blue (low byte)
*/

     luma = ((rc * 19) + (gc * 37) + (bc * 7)); /* Wx*1024/10000. */
     luma = (luma >> 6);
    //luma = (rc + gc + bc) / 3;

    switch (channel)
    {
    case 1: // R channel (5 bits)        
        rc = luma;
        gc = 0;
        bc = 0;
        break;
    case 2: // G channel (6 bits)
        rc = 0;
        gc = luma;
        bc = 0;
        break;
    case 3: // B channel (5 bits)        
        rc = 0;
        gc = 0;
        bc = luma;
        break;
    default: // monochrome
        rc = luma;
        gc = luma;
        bc = luma;
        break;
    }

    uint16_t x = ((rc >> 1) << 11) | (gc << 5) | (bc >> 1); // lose 1 bit for R and B

    // result = ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);

    return __builtin_bswap16(x);
}

static esp_err_t panel_st7735_draw_bitmap(esp_lcd_panel_t *panel, int x_start, int y_start, int x_end, int y_end, const void *color_data)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    assert((x_start < x_end) && (y_start < y_end) && "start position must be smaller than end position");
    esp_lcd_panel_io_handle_t io = st7735->io;

    x_start += st7735->x_gap;
    x_end += st7735->x_gap;
    y_start += st7735->y_gap;
    y_end += st7735->y_gap;

    // define an area of frame memory where MCU can access
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_CASET, (uint8_t[]){
                                                                         (x_start >> 8) & 0xFF,
                                                                         x_start & 0xFF,
                                                                         ((x_end - 1) >> 8) & 0xFF,
                                                                         (x_end - 1) & 0xFF,
                                                                     },
                                                  4),
                        TAG, "send command failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_RASET, (uint8_t[]){
                                                                         (y_start >> 8) & 0xFF,
                                                                         y_start & 0xFF,
                                                                         ((y_end - 1) >> 8) & 0xFF,
                                                                         (y_end - 1) & 0xFF,
                                                                     },
                                                  4),
                        TAG, "send command failed");
    // transfer frame buffer
    size_t len = (x_end - x_start) * (y_end - y_start) * st7735->fb_bits_per_pixel / 8;
    // ESP_LOGI(TAG, "x,y = %d,%d -- %d,%d, len == %d", x_start, y_start, x_end, y_end, len);

    // apply color filter, if required
    if (st7735->color_channel) // 0 = none, 1 = R, 2 = G, 3 = B
    {
        uint16_t *color_data_16 = (uint16_t *)color_data;
        for (int i = 0; i < len / 2; i++)
            color_data_16[i] = rgb565_to_brightness_channel(color_data_16[i], st7735->color_channel);
    }

    esp_lcd_panel_io_tx_color(io, LCD_CMD_RAMWR, color_data, len);

    return ESP_OK;
}

esp_err_t panel_st7735_set_color_channel(esp_lcd_panel_t *panel, uint8_t color_channel)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    st7735->color_channel = color_channel;
    return ESP_OK;
}

static esp_err_t panel_st7735_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    int command = 0;
    if (invert_color_data)
    {
        command = LCD_CMD_INVON;
    }
    else
    {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_st7735_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    if (mirror_x)
    {
        st7735->madctl_val |= LCD_CMD_MX_BIT;
    }
    else
    {
        st7735->madctl_val &= ~LCD_CMD_MX_BIT;
    }
    if (mirror_y)
    {
        st7735->madctl_val |= LCD_CMD_MY_BIT;
    }
    else
    {
        st7735->madctl_val &= ~LCD_CMD_MY_BIT;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){st7735->madctl_val}, 1), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_st7735_swap_xy(esp_lcd_panel_t *panel, bool swap_axes)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    if (swap_axes)
    {
        st7735->madctl_val |= LCD_CMD_MV_BIT;
    }
    else
    {
        st7735->madctl_val &= ~LCD_CMD_MV_BIT;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){st7735->madctl_val}, 1), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_st7735_set_gap(esp_lcd_panel_t *panel, int x_gap, int y_gap)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    st7735->x_gap = x_gap;
    st7735->y_gap = y_gap;
    return ESP_OK;
}

static esp_err_t panel_st7735_disp_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    st7735_panel_t *st7735 = __containerof(panel, st7735_panel_t, base);
    esp_lcd_panel_io_handle_t io = st7735->io;
    int command = 0;

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    on_off = !on_off;
#endif

    if (on_off)
    {
        command = LCD_CMD_DISPON;
    }
    else
    {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}
