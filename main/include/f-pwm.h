#ifndef PWM_H
#define PWM_H

#include "esp_err.h"

#define PWM_MIN_FREQUENCY_HZ 60
#define PWM_MAX_FREQUENCY_HZ 50000
#define PWM_SETTINGS_MAX_POWER 1023 /* 10-bit PWM on GPIO32; DAC on GPIO25 is 8-bit */

uint16_t pwm_get_safe_maximum_power(void);
uint16_t pwm_get_effective_max_power(void);
uint8_t pwm_get_current_brightness(void);

void startup_led_pwm();
void set_led_pwm_brightness(uint8_t duty);
esp_err_t reconfigure_led_pwm_frequency(void);


#endif // PWM_H
