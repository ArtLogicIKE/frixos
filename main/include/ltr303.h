#ifndef LTR303_H
#define LTR303_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

/*
 * Default LTR303 addresses and register definitions:
 */
#define LTR303_DEFAULT_I2C_ADDR 0x29  // 7-bit address

#define LTR303_CONTR         0x80
#define LTR303_MEAS_RATE     0x85
#define LTR303_PART_ID       0x86
#define LTR303_MANUFAC_ID    0x87
#define LTR303_DATA_CH1_0    0x88
#define LTR303_DATA_CH1_1    0x89
#define LTR303_DATA_CH0_0    0x8A
#define LTR303_DATA_CH0_1    0x8B
#define LTR303_STATUS        0x8C
#define LTR303_INTERRUPT     0x8F
#define LTR303_THRES_UP_0    0x97
#define LTR303_THRES_UP_1    0x98
#define LTR303_THRES_LOW_0   0x99
#define LTR303_THRES_LOW_1   0x9A
#define LTR303_INTR_PERS     0x9E

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Map ESP-IDF I2C errors to simple "Wire-like" error codes:
 *   0 = Success
 *   4 = Other error
 */
static inline uint8_t ltr303_map_esp_err(esp_err_t err) {
    return (err == ESP_OK) ? 0 : 4;
}

/**
 * LTR303 device context. 
 *   - Replaces the old approach of storing (i2c_port, i2c_addr).
 *   - We now store an I2C device handle and track the last error code.
 */
typedef struct {
    i2c_master_dev_handle_t i2c_dev; ///< Handle to the LTR303 device
    uint8_t last_error;             ///< 0 = success, 4 = other error
} ltr303_dev_t;

/**
 * Initialize the LTR303 device handle on the *new* I2C master driver.
 *
 * @param dev         Pointer to ltr303_dev_t allocated by caller.
 * @param bus         A valid I2C bus handle (from i2c_new_bus()).
 * @param i2c_addr    7-bit address of the LTR303 (usually 0x29).
 */
void ltr303_init(ltr303_dev_t *dev, i2c_master_bus_handle_t bus, uint8_t i2c_addr);

/**
 * Check if sensor is responding by reading PART ID.
 * Logs an error if it fails.
 *
 * @return true if success, false if I2C error.
 */
bool ltr303_begin(ltr303_dev_t *dev);

/* Power on / off */
bool ltr303_set_power_up(ltr303_dev_t *dev);
bool ltr303_set_power_down(ltr303_dev_t *dev);

/* Control register access */
bool ltr303_set_control(ltr303_dev_t *dev, uint8_t gain, bool reset, bool mode);
bool ltr303_get_control(ltr303_dev_t *dev, uint8_t *gain, bool *reset, bool *mode);

/* Measurement rate register */
bool ltr303_set_measurement_rate(ltr303_dev_t *dev, uint8_t integrationTime, uint8_t measurementRate);
bool ltr303_get_measurement_rate(ltr303_dev_t *dev, uint8_t *integrationTime, uint8_t *measurementRate);

/* ID / data read */
bool ltr303_get_part_id(ltr303_dev_t *dev, uint8_t *part_id);
bool ltr303_get_manufac_id(ltr303_dev_t *dev, uint8_t *manuf_id);
bool ltr303_get_data(ltr303_dev_t *dev, uint16_t *CH0, uint16_t *CH1);

/* Status / interrupt config */
bool ltr303_get_status(ltr303_dev_t *dev, bool *valid, uint8_t *gain, bool *intrStatus, bool *dataStatus);
bool ltr303_set_interrupt_control(ltr303_dev_t *dev, bool intrMode, bool polarity);
bool ltr303_get_interrupt_control(ltr303_dev_t *dev, bool *intrMode, bool *polarity);

/* Threshold / persist */
bool ltr303_set_threshold(ltr303_dev_t *dev, uint16_t upper, uint16_t lower);
bool ltr303_get_threshold(ltr303_dev_t *dev, uint16_t *upper, uint16_t *lower);
bool ltr303_set_intr_persist(ltr303_dev_t *dev, uint8_t persist);
bool ltr303_get_intr_persist(ltr303_dev_t *dev, uint8_t *persist);

/* Lux calculation */
bool ltr303_get_lux(uint8_t gain, uint8_t integrationTime, uint16_t CH0, uint16_t CH1, double *lux);

/* Get the last error code (0=OK, 4=Other) */
uint8_t ltr303_get_error(ltr303_dev_t *dev);

#ifdef __cplusplus
}
#endif

#endif // LTR303_H

/**
 * Set measurement rate register:
 *   bits [7:3] => integrationTime, bits [2:0] => measurementRate.
 */
			// Sets the integration time and measurement rate of the sensor
			// integrationTime is the measurement time for each ALs cycle
			// measurementRate is the interval between DATA_REGISTERS update
			// measurementRate must be set to be equal or greater than integrationTime
			// Default value is 0x03
			// If integrationTime = 0, integrationTime will be 100ms (default)
			// If integrationTime = 1, integrationTime will be 50ms
			// If integrationTime = 2, integrationTime will be 200ms
			// If integrationTime = 3, integrationTime will be 400ms
			// If integrationTime = 4, integrationTime will be 150ms
			// If integrationTime = 5, integrationTime will be 250ms
			// If integrationTime = 6, integrationTime will be 300ms
			// If integrationTime = 7, integrationTime will be 350ms
			//------------------------------------------------------
			// If measurementRate = 0, measurementRate will be 50ms
			// If measurementRate = 1, measurementRate will be 100ms
			// If measurementRate = 2, measurementRate will be 200ms
			// If measurementRate = 3, measurementRate will be 500ms (default)
			// If measurementRate = 4, measurementRate will be 1000ms
			// If measurementRate = 5, measurementRate will be 2000ms
			// If measurementRate = 6, measurementRate will be 2000ms
			// If measurementRate = 7, measurementRate will be 2000ms
			// Returns true (1) if successful, false (0) if there was an I2C error