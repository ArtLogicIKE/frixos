#include "ltr303.h"
#include <string.h>
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "LTR303 driver";

// Forward declarations for internal helpers
static bool ltr303_read_byte(ltr303_dev_t *dev, uint8_t reg, uint8_t *data);
static bool ltr303_write_byte(ltr303_dev_t *dev, uint8_t reg, uint8_t data);
static bool ltr303_read_uint16(ltr303_dev_t *dev, uint8_t reg, uint16_t *value);
static bool ltr303_write_uint16(ltr303_dev_t *dev, uint8_t reg, uint16_t value);

/**
 * @brief Update dev->last_error from an esp_err_t
 */
static bool ltr303_update_error(ltr303_dev_t *dev, esp_err_t err)
{
    dev->last_error = ltr303_map_esp_err(err); // 0 if OK, 4 otherwise
    return (err == ESP_OK);
}

/********************************************************************
 *                 PUBLIC API IMPLEMENTATION                        *
 *******************************************************************/

void ltr303_init(ltr303_dev_t *dev, i2c_master_bus_handle_t bus, uint8_t i2c_addr)
{
    ESP_LOGI(TAG, "LTR303 init");
    dev->last_error = 0;

    // 1) Setup bus config
    i2c_master_bus_config_t bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1, // auto select port
        .scl_io_num = 22,
        .sda_io_num = 21,
        .flags.enable_internal_pullup = true,
        .glitch_ignore_cnt = 7,
    };

    // 2) Create the bus
    esp_err_t ret = i2c_new_master_bus(&bus_cfg, &bus);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "I2C bus could not be created");
    }

    // Optionally set bus clock to 100kHz
    i2c_device_config_t clk_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = i2c_addr,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t i2c_dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &clk_cfg, &i2c_dev));
    dev->i2c_dev = i2c_dev;
}

bool ltr303_begin(ltr303_dev_t *dev)
{
    ESP_LOGI(TAG, "LTR303 begin");
    // Try reading PART ID
    uint8_t part_id = 0;
    if (!ltr303_get_part_id(dev, &part_id))
    {
        ESP_LOGE(TAG, "Failed reading Part ID. Error code=%d", dev->last_error);
        return false;
    }
    ESP_LOGI(TAG, "LTR303 PART ID = 0x%02X", part_id);
    return true;
}

bool ltr303_set_power_up(ltr303_dev_t *dev)
{
    uint8_t regVal;
    if (!ltr303_read_byte(dev, LTR303_CONTR, &regVal))
    {
        return false;
    }
    // bit0 => active mode
    regVal |= 0x01;
    return ltr303_write_byte(dev, LTR303_CONTR, regVal);
}

bool ltr303_set_power_down(ltr303_dev_t *dev)
{
    uint8_t regVal;
    if (!ltr303_read_byte(dev, LTR303_CONTR, &regVal))
    {
        return false;
    }
    regVal &= ~(0x01);
    return ltr303_write_byte(dev, LTR303_CONTR, regVal);
}

bool ltr303_set_control(ltr303_dev_t *dev, uint8_t gain, bool reset, bool mode)
{
    // bits [7:5] => gain, bit3 => reset, bit0 => mode
    uint8_t val = 0;
    val |= ((gain & 0x07) << 5);
    if (reset)
        val |= (1 << 3);
    if (mode)
        val |= (1 << 0);

    return ltr303_write_byte(dev, LTR303_CONTR, val);
}

bool ltr303_get_control(ltr303_dev_t *dev, uint8_t *gain, bool *reset, bool *mode)
{
    uint8_t val;
    if (!ltr303_read_byte(dev, LTR303_CONTR, &val))
        return false;

    *gain = (val >> 5) & 0x07;
    *reset = (val & (1 << 3)) != 0;
    *mode = (val & (1 << 0)) != 0;
    return true;
}

bool ltr303_set_measurement_rate(ltr303_dev_t *dev, uint8_t integrationTime, uint8_t measurementRate)
{
    // bits [7:3] => integrationTime, [2:0] => measurementRate
    uint8_t val = ((integrationTime & 0x07) << 3) | (measurementRate & 0x07);
    return ltr303_write_byte(dev, LTR303_MEAS_RATE, val);
}

bool ltr303_get_measurement_rate(ltr303_dev_t *dev, uint8_t *integrationTime, uint8_t *measurementRate)
{
    uint8_t val;
    if (!ltr303_read_byte(dev, LTR303_MEAS_RATE, &val))
        return false;

    *integrationTime = (val >> 3) & 0x07;
    *measurementRate = val & 0x07;
    return true;
}

bool ltr303_get_part_id(ltr303_dev_t *dev, uint8_t *part_id)
{
    return ltr303_read_byte(dev, LTR303_PART_ID, part_id);
}

bool ltr303_get_manufac_id(ltr303_dev_t *dev, uint8_t *manuf_id)
{
    return ltr303_read_byte(dev, LTR303_MANUFAC_ID, manuf_id);
}

bool ltr303_get_data(ltr303_dev_t *dev, uint16_t *CH0, uint16_t *CH1)
{
    // CH0 => 0x8A..8B, CH1 => 0x88..89
    if (!ltr303_read_uint16(dev, LTR303_DATA_CH0_0, CH0))
        return false;
    if (!ltr303_read_uint16(dev, LTR303_DATA_CH1_0, CH1))
        return false;
    return true;
}

bool ltr303_get_status(ltr303_dev_t *dev, bool *valid, uint8_t *gain, bool *intrStatus, bool *dataStatus)
{
    uint8_t stat;
    if (!ltr303_read_byte(dev, LTR303_STATUS, &stat))
        return false;
    // bit7 => data invalid, [6:4] => gain, bit2 => intr, bit0 => new data
    *valid = (stat & (1 << 7)) != 0;
    *gain = (stat >> 4) & 0x07;
    *intrStatus = (stat & (1 << 2)) != 0;
    *dataStatus = (stat & (1 << 0)) != 0;
    return true;
}

bool ltr303_set_interrupt_control(ltr303_dev_t *dev, bool intrMode, bool polarity)
{
    // bits: [2] => intrMode, [1] => polarity
    uint8_t val = 0;
    if (intrMode)
        val |= (1 << 2);
    if (polarity)
        val |= (1 << 1);

    return ltr303_write_byte(dev, LTR303_INTERRUPT, val);
}

bool ltr303_get_interrupt_control(ltr303_dev_t *dev, bool *intrMode, bool *polarity)
{
    uint8_t val;
    if (!ltr303_read_byte(dev, LTR303_INTERRUPT, &val))
        return false;
    *intrMode = (val & (1 << 2)) != 0;
    *polarity = (val & (1 << 1)) != 0;
    return true;
}

bool ltr303_set_threshold(ltr303_dev_t *dev, uint16_t upper, uint16_t lower)
{
    if (!ltr303_write_uint16(dev, LTR303_THRES_UP_0, upper))
        return false;
    if (!ltr303_write_uint16(dev, LTR303_THRES_LOW_0, lower))
        return false;
    return true;
}

bool ltr303_get_threshold(ltr303_dev_t *dev, uint16_t *upper, uint16_t *lower)
{
    if (!ltr303_read_uint16(dev, LTR303_THRES_UP_0, upper))
        return false;
    if (!ltr303_read_uint16(dev, LTR303_THRES_LOW_0, lower))
        return false;
    return true;
}

bool ltr303_set_intr_persist(ltr303_dev_t *dev, uint8_t persist)
{
    return ltr303_write_byte(dev, LTR303_INTR_PERS, persist);
}

bool ltr303_get_intr_persist(ltr303_dev_t *dev, uint8_t *persist)
{
    return ltr303_read_byte(dev, LTR303_INTR_PERS, persist);
}
// Function to get gain value from gain setting
float ltr303_get_gain(uint8_t gain_setting)
{
    switch (gain_setting)
    {
    case 0:
        return 1.0;
    case 1:
        return 2.0;
    case 2:
        return 4.0;
    case 3:
        return 8.0;
    case 6:
        return 48.0;
    case 7:
        return 96.0;
    default:
        return 1.0; // Default gain if invalid setting
    }
}

// Function to get integration time in milliseconds from integration setting
float ltr303_get_integration_time(uint8_t integration_setting)
{
    switch (integration_setting)
    {
    case 0:
        return 100.0;
    case 1:
        return 50.0;
    case 2:
        return 200.0;
    case 3:
        return 400.0;
    case 4:
        return 150.0;
    case 5:
        return 250.0;
    case 6:
        return 300.0;
    case 7:
        return 350.0;
    default:
        return 100.0; // Default integration time if invalid setting
    }
}

// Function to calculate lux from raw CH0 and CH1 values
float ltr303_calculate_lux(uint16_t ch0, uint16_t ch1, uint8_t gain_setting, uint8_t integration_setting)
{
    float ratio, lux;
    float gain = ltr303_get_gain(gain_setting);
    float int_time = ltr303_get_integration_time(integration_setting);

    if (ch0 + ch1 == 0)
        return 0.0;

    ratio = (float)ch1 / (ch0 + ch1);

    if (ratio <= 0.45)
    {
        lux = (1.7743 * ch0 + 1.1059 * ch1);
    }
    else if (ratio <= 0.64)
    {
        lux = (4.2785 * ch0 - 1.9548 * ch1);
    }
    else if (ratio <= 0.85)
    {
        lux = (0.5926 * ch0 + 0.1185 * ch1);
    }
    else
    {
        lux = 0.0;
    }
    ESP_LOGV(TAG, "LTR303 lux: %.2f = %d, %d, ratio %f, gain %f, int_time %f", lux, ch0, ch1, ratio, gain, int_time);

/*
    lux *= 100.0; // as per datasheet recommendation
    lux /= (gain * int_time);
    */

    return lux;
}
bool ltr303_get_lux(uint8_t gain, uint8_t integrationTime, uint16_t CH0, uint16_t CH1, double *lux)
{
    if (!lux)
        return false;
    // If either channel is saturated, return 999.0
    if (CH0 == 0xFFFF || CH1 == 0xFFFF)
    {
        *lux = 999.0;
        return false;
    }
    *lux = ltr303_calculate_lux(CH0, CH1, gain, integrationTime);
    return true;
}

uint8_t ltr303_get_error(ltr303_dev_t *dev)
{
    return dev->last_error;
}

/********************************************************************
 *                         INTERNAL HELPERS                          *
 ********************************************************************/

static bool ltr303_read_byte(ltr303_dev_t *dev, uint8_t reg, uint8_t *data)
{
    // 1) Write the register address, then read 1 byte from that register.
    // We can do this in *one* call with i2c_master_write_read_device().
    esp_err_t ret = i2c_master_transmit_receive(
        dev->i2c_dev,
        &reg, 1, // first, send 'reg'
        data, 1, // then read 1 byte
        1000);

    return ltr303_update_error(dev, ret);
}

static bool ltr303_write_byte(ltr303_dev_t *dev, uint8_t reg, uint8_t data)
{
    // We can do a small buffer: [reg, data]
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;

    // i2c_master_write_to_device() => write N bytes
    esp_err_t ret = i2c_master_transmit(
        dev->i2c_dev,
        buf, 2, // write reg + data
        1000    // timeout
    );

    return ltr303_update_error(dev, ret);
}

static bool ltr303_read_uint16(ltr303_dev_t *dev, uint8_t reg, uint16_t *value)
{
    // read 2 bytes (low, high) from 'reg'
    esp_err_t ret = i2c_master_transmit_receive(
        dev->i2c_dev,
        &reg, 1,
        (uint8_t *)value, 2,
        1000);
    if (!ltr303_update_error(dev, ret))
    {
        return false;
    }

    // By default, *value = [low, high] in little-endian.
    // But LTR303: first byte => low, second => high
    // The read above will store them in little-endian memory, which is correct for CH0/CH1.
    // If needed, you can reorder bytes:
    uint8_t *b = (uint8_t *)value;
    uint16_t combined = ((uint16_t)b[1] << 8) | b[0];
    *value = combined;

    return true;
}

static bool ltr303_write_uint16(ltr303_dev_t *dev, uint8_t reg, uint16_t val)
{
    // We want [reg, low, high]
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (val & 0xFF);        // low
    buf[2] = ((val >> 8) & 0xFF); // high

    esp_err_t ret = i2c_master_transmit(
        dev->i2c_dev,
        buf, 3,
        1000 / portTICK_PERIOD_MS);
    return ltr303_update_error(dev, ret);
}
