#include "frixos.h"
#include "f-pwm.h"

static const char *TAG = "f-pwm";
static bool led_pwm_configured = false;

static uint32_t scale_10bit_to_8bit(uint32_t value)
{
    if (value > PWM_SETTINGS_MAX_POWER)
        value = PWM_SETTINGS_MAX_POWER;
    return (value * PWM_8BIT_MAX_DUTY + (PWM_SETTINGS_MAX_POWER / 2)) / PWM_SETTINGS_MAX_POWER;
}

static uint32_t normalize_pwm_frequency(uint32_t freq)
{
    if (freq < 10)
        freq = 10;
    if (freq > PWM_MAX_FREQUENCY_HZ)
        freq = PWM_MAX_FREQUENCY_HZ;
    if (freq == 133)
        freq = 200; // replace 133 with 200 for backwards compatibility
    return freq;
}


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
    uint32_t freq = normalize_pwm_frequency(eeprom_pwm_frequency);

    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                   // Low-speed mode (ESP32 supports both high and low)
        .timer_num = timer,                   // Use timer 0
        .duty_resolution = LEDC_TIMER_8_BIT,  // 8-bit resolution
        .freq_hz = freq,                      // Use frequency from NVS (default 200Hz)
        // APB at 200 Hz needs div ~400000, above ESP32 LEDC max (0x3FFFF). AUTO_CLK falls back to RC_FAST.
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "PWM timer config failed: %s", esp_err_to_name(err));
        led_pwm_configured = false;
        return;
    }

    // Configure the LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode = mode,
        .channel = channel, // Use channel 0
        .timer_sel = timer, // Use timer 0
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = 32, // LED GPIO pin is pin32
        .duty = scale_10bit_to_8bit(eeprom_max_power), // Start with LED at max configured power
        .hpoint = 0};
    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "PWM channel config failed: %s", esp_err_to_name(err));
        led_pwm_configured = false;
        return;
    }
    led_pwm_configured = true;

    // set_led_pwm_brightness(eeprom_brightness_LED[0]); // Set initial brightness

    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "PWM LED GPIO32 %lu Hz 8bit", (unsigned long)freq);
}

// Reconfigure PWM frequency (called when settings are updated)
esp_err_t reconfigure_led_pwm_frequency(void)
{
    ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    ledc_timer_t timer = LEDC_TIMER_0;
    
    uint32_t freq = normalize_pwm_frequency(eeprom_pwm_frequency);
    
    // Configure the LEDC timer with new frequency
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                   // Low-speed mode
        .timer_num = timer,                   // Use timer 0
        .duty_resolution = LEDC_TIMER_8_BIT,  // 8-bit resolution
        .freq_hz = freq,                      // New frequency from settings
        .clk_cfg = LEDC_AUTO_CLK,
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
    return err;
}



// accepts brightness percentage (0-100)
void set_led_pwm_brightness(uint8_t duty)
{

    (duty > 100) ? (duty = 100) : (duty = duty);  // Ensure duty cycle is between 0 and 100
    uint32_t capped_duty_10bit = ((uint32_t)duty * eeprom_max_power) / 100;
    int pwm_duty = (int)scale_10bit_to_8bit(capped_duty_10bit);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "LED brightness %i%% (actual %i)", duty, pwm_duty);
    if (!led_pwm_configured)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "LED brightness skipped: PWM not configured");
        return;
    }

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pwm_duty);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LED duty set failed: %s", esp_err_to_name(err));
        return;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LED duty update failed: %s", esp_err_to_name(err));
    }
}
