#ifndef PWM_H
#define PWM_H

#define MAX_DUTY 750 // a bit less than 75% duty cycle

void startup_led_pwm();
void set_led_pwm_brightness(uint8_t duty);
void reconfigure_led_pwm_frequency(void);


#endif // PWM_H