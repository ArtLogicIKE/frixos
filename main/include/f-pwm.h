#ifndef PWM_H
#define PWM_H

#include "esp_err.h"

#define MAX_DUTY 750 // a bit less than 75% duty cycle
#define PWM_MAX_FREQUENCY_HZ 300000
#define PWM_SETTINGS_MAX_POWER 1023
#define PWM_8BIT_MAX_DUTY 255

void startup_led_pwm();
void set_led_pwm_brightness(uint8_t duty);
esp_err_t reconfigure_led_pwm_frequency(void);


#endif // PWM_H