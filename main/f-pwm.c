#include "frixos.h"
#include "f-pwm.h"

static const char *TAG = "f-pwm";


void startup_led_pwm()
{

    // Let's isolate IO0 before is causes havoc
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_0, 1); // Force HIGH after boot

    // and prepare IO32
    gpio_reset_pin(GPIO_NUM_32);
    gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(GPIO_NUM_32, GPIO_FLOATING); // Ensure no pull-ups or pull-downs;

    // Configure the LEDC peripheral
    ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    ledc_timer_t timer = LEDC_TIMER_0;
    ledc_channel_t channel = LEDC_CHANNEL_0;

    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                   // Low-speed mode (ESP32 supports both high and low)
        .timer_num = timer,                   // Use timer 0
        .duty_resolution = LEDC_TIMER_10_BIT, // 10-bit resolution
        .freq_hz = eeprom_pwm_frequency,      // Use frequency from NVS (default 200Hz)
        .clk_cfg = LEDC_APB_CLK,              // Use the fast clock;
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = mode,
        .channel = channel, // Use channel 0
        .timer_sel = timer, // Use timer 0
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = 32, // LED GPIO pin is pin32
        .duty = eeprom_max_power,   // Start with LED at high
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    // set_led_pwm_brightness(eeprom_brightness_LED[0]); // Set initial brightness

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "PWM LED GPIO32 %lu Hz 10bit", (unsigned long)eeprom_pwm_frequency);
}

// Reconfigure PWM frequency (called when settings are updated)
void reconfigure_led_pwm_frequency(void)
{
    ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    ledc_timer_t timer = LEDC_TIMER_0;
    
    // Validate frequency range
    uint32_t freq = eeprom_pwm_frequency;
    if (freq < 10) freq = 10;
    if (freq > 78000) freq = 78000;
    if (freq == 133) freq = 200; // replace 133 with 200 for backwards compatibility
    
    // Configure the LEDC timer with new frequency
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                   // Low-speed mode
        .timer_num = timer,                   // Use timer 0
        .duty_resolution = LEDC_TIMER_10_BIT, // 10-bit resolution
        .freq_hz = freq,                      // New frequency from settings
        .clk_cfg = LEDC_APB_CLK,              // Use the fast clock
    };
    
    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "PWM freq reconfigure failed: %s", esp_err_to_name(err));
    }
    else
    {
        ESP_LOG_WEB(ESP_LOG_INFO, TAG, "PWM frequency reconfigured to %lu Hz", (unsigned long)freq);
    }
}



// accepts brightness percentage (0-100)
void set_led_pwm_brightness(uint8_t duty)
{

    (duty > 100) ? (duty = 100) : (duty = duty);  // Ensure duty cycle is between 0 and 100
    int pwm_duty = (int)(duty * 10.23);           // 1023 is the max duty cycle for 10-bit resolution
    // scale based on max_power
    pwm_duty = (pwm_duty * eeprom_max_power) / 1023;    
    if (duty == 99) {
        pwm_duty = 1023;
    }
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "LED brightness %i%% (actual %i)", duty, pwm_duty);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pwm_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}
