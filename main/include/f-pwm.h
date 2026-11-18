#ifndef PWM_H
#define PWM_H

#include "esp_err.h"

#define PWM_MIN_FREQUENCY_HZ 60
#define PWM_MAX_FREQUENCY_HZ 50000
#define PWM_SETTINGS_MAX_POWER 1023 /* 10-bit PWM on GPIO32; DAC on GPIO25 is 8-bit */

uint16_t pwm_get_safe_maximum_power(void);
uint16_t pwm_get_effective_max_power(void);
uint8_t pwm_get_current_brightness(void);
uint16_t pwm_get_current_duty(void);

void startup_led_pwm();
void set_led_brightness_profile(uint8_t profile); // 0 = day, 1 = night
uint16_t pwm_duty_from_level(uint8_t level);       // p68 gamma 2.2 slider level (0-100) to duty
uint8_t pwm_percent_from_duty(uint16_t duty);      // duty to whole percent of the maximum
esp_err_t reconfigure_led_pwm_frequency(void);


#endif // PWM_H
