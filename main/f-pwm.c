#include <string.h>
#include "frixos.h"
#include "f-pwm.h"
#include "driver/dac_oneshot.h"
#include "driver/dac_continuous.h"

static const char *TAG = "f-pwm";
static bool led_pwm_configured = false;
static bool led_dac_configured = false;
static dac_oneshot_handle_t led_dac_handle = NULL;
static dac_continuous_handle_t led_dac_chop_handle = NULL;
// Last brightness percent applied via set_led_pwm_brightness.
// Until startup_led_pwm() runs, the hardware is not driven; 100 is the
// pre-init placeholder only.
static uint8_t current_brightness = 100;

#define LED_PWM_GPIO GPIO_NUM_32
#define LED_DAC_GPIO GPIO_NUM_25 /* DAC_CHAN_0 */
#define DAC_MAX_VALUE 255

// Rev I feeds GPIO25 straight into the PAM2863 VSET pin. VSET shuts the output off at 0.2 V and DC
// dimming is only specified from 0.3 V, so levels below that chop a steady 0.3 V on and off with DMA.
#define LED_DAC_CHOP_BOARD_REV 2
#define LED_DAC_CHOP_LEVEL 24 /* about 0.31 V */
#define LED_DAC_CHOP_SAMPLES 200
#define LED_DAC_CHOP_MIN_HZ 100 /* the DMA clock needs at least 19.6 kHz */
#define LED_DAC_CHOP_MAX_HZ 2000

uint8_t pwm_get_current_brightness(void)
{
    return current_brightness;
}

uint16_t pwm_get_safe_maximum_power(void)
{
    switch (eeprom_board_rev)
    {
    case 1:
        return 850;
    case 2:
        return 600;
    default:
        return 750;
    }
}

uint16_t pwm_get_effective_max_power(void)
{
    uint32_t safe = pwm_get_safe_maximum_power();
    return (uint16_t)((safe * eeprom_max_power) / PWM_SETTINGS_MAX_POWER);
}

static uint8_t active_brightness_percent(void)
{
    uint8_t profile = (font_index <= 1) ? font_index : 0;
    uint8_t pct = eeprom_brightness_LED[profile];
    if (pct > 100)
        pct = 100;
    return pct;
}

static uint16_t duty_from_percent(uint8_t pct)
{
    if (pct > 100)
        pct = 100;
    return (uint16_t)(((uint32_t)pct * pwm_get_effective_max_power()) / 100);
}

static uint32_t normalize_pwm_frequency(uint32_t freq)
{
    if (freq < PWM_MIN_FREQUENCY_HZ)
        freq = PWM_MIN_FREQUENCY_HZ;
    if (freq > PWM_MAX_FREQUENCY_HZ)
        freq = PWM_MAX_FREQUENCY_HZ;
    if (freq == 133)
        freq = 200; // replace 133 with 200 for backwards compatibility
    return freq;
}

// Map 10-bit PWM duty (0..1023) onto the 8-bit DAC (0..255).
static uint8_t scale_10bit_to_dac8(uint16_t value_10bit)
{
    if (value_10bit > PWM_SETTINGS_MAX_POWER)
        value_10bit = PWM_SETTINGS_MAX_POWER;
    return (uint8_t)(((uint32_t)value_10bit * DAC_MAX_VALUE + (PWM_SETTINGS_MAX_POWER / 2))
                     / PWM_SETTINGS_MAX_POWER);
}

// Above the chop level the table holds one steady value; below it, the level for part of each cycle.
static void apply_led_dac_chop(uint16_t duty_10bit)
{
    static uint8_t wave[LED_DAC_CHOP_SAMPLES];
    static int last_level = -1;
    static int last_on = -1;

    int level = scale_10bit_to_dac8(duty_10bit);
    int on = LED_DAC_CHOP_SAMPLES;
    if (level < LED_DAC_CHOP_LEVEL)
    {
        const uint32_t full = (uint32_t)LED_DAC_CHOP_LEVEL * PWM_SETTINGS_MAX_POWER;
        on = (int)(((uint32_t)duty_10bit * DAC_MAX_VALUE * LED_DAC_CHOP_SAMPLES + full / 2) / full);
        if (on == 0 && duty_10bit > 0)
            on = 1;
        level = LED_DAC_CHOP_LEVEL;
    }
    // Each new table restarts the DMA, so leave it running when nothing changed.
    if (level == last_level && on == last_on)
        return;

    memset(wave, 0, sizeof(wave));
    memset(wave, level, on);
    esp_err_t err = dac_continuous_write_cyclically(led_dac_chop_handle, wave, sizeof(wave), NULL);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DAC chop write failed: %s", esp_err_to_name(err));
        return;
    }
    last_level = level;
    last_on = on;
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "DAC GPIO25 level %d on %d/%d", level, on, LED_DAC_CHOP_SAMPLES);
}

static void apply_led_dac(uint16_t duty_10bit)
{
    if (led_dac_chop_handle != NULL)
    {
        apply_led_dac_chop(duty_10bit);
        return;
    }
    if (!led_dac_configured || led_dac_handle == NULL)
        return;
    uint8_t dac_val = scale_10bit_to_dac8(duty_10bit);
    esp_err_t err = dac_oneshot_output_voltage(led_dac_handle, dac_val);
    if (err != ESP_OK)
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DAC GPIO25 set failed: %s", esp_err_to_name(err));
}

// The chop frequency only changes at boot: deleting the channel would float VSET, which is full current.
static void startup_led_dac_chop(uint16_t duty_10bit)
{
    uint32_t chop_hz = normalize_pwm_frequency(eeprom_pwm_frequency);
    if (chop_hz < LED_DAC_CHOP_MIN_HZ)
        chop_hz = LED_DAC_CHOP_MIN_HZ;
    if (chop_hz > LED_DAC_CHOP_MAX_HZ)
        chop_hz = LED_DAC_CHOP_MAX_HZ;

    dac_continuous_config_t cfg = {
        .chan_mask = DAC_CHANNEL_MASK_CH0, // GPIO25
        .desc_num = 4,
        .buf_size = LED_DAC_CHOP_SAMPLES,
        .freq_hz = chop_hz * LED_DAC_CHOP_SAMPLES,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
    };
    esp_err_t err = dac_continuous_new_channels(&cfg, &led_dac_chop_handle);
    if (err == ESP_OK)
        err = dac_continuous_enable(led_dac_chop_handle);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DAC chop config failed: %s", esp_err_to_name(err));
        led_dac_chop_handle = NULL;
        led_dac_configured = false;
        return;
    }
    led_dac_configured = true;
    apply_led_dac_chop(duty_10bit);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "DAC LED GPIO25 chop %lu Hz", (unsigned long)chop_hz);
}

static void startup_led_dac(uint16_t duty_10bit)
{
    gpio_reset_pin(LED_DAC_GPIO);

    if (eeprom_board_rev == LED_DAC_CHOP_BOARD_REV)
    {
        startup_led_dac_chop(duty_10bit);
        return;
    }

    dac_oneshot_config_t dac_cfg = {
        .chan_id = DAC_CHAN_0, // GPIO25
    };
    esp_err_t err = dac_oneshot_new_channel(&dac_cfg, &led_dac_handle);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "DAC channel config failed: %s", esp_err_to_name(err));
        led_dac_configured = false;
        led_dac_handle = NULL;
        return;
    }
    led_dac_configured = true;
    apply_led_dac(duty_10bit);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "DAC LED GPIO25 8bit value=%u (from 10bit %u)",
                scale_10bit_to_dac8(duty_10bit), duty_10bit);
}


void startup_led_pwm()
{

    // Let's isolate IO0 before is causes havoc
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_0, 1); // Force HIGH after boot

    // and prepare IO32
    gpio_reset_pin(LED_PWM_GPIO);
    gpio_set_direction(LED_PWM_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LED_PWM_GPIO, GPIO_FLOATING); // Ensure no pull-ups or pull-downs;

    // Configure the LEDC peripheral
    ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    ledc_timer_t timer = LEDC_TIMER_0;
    ledc_channel_t channel = LEDC_CHANNEL_0;
    uint32_t freq = normalize_pwm_frequency(eeprom_pwm_frequency);
    uint8_t initial_pct = active_brightness_percent();
    uint16_t initial_duty = duty_from_percent(initial_pct);

    // Configure the LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                    // Low-speed mode (ESP32 supports both high and low)
        .timer_num = timer,                    // Use timer 0
        .duty_resolution = LEDC_TIMER_10_BIT,  // 10-bit resolution
        .freq_hz = freq,                       // Use frequency from NVS (default 200Hz)
        // APB at 200 Hz needs div ~400000, above ESP32 LEDC max (0x3FFFF). AUTO_CLK falls back to RC_FAST.
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK)
    {
        ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "PWM timer config failed: %s", esp_err_to_name(err));
        led_pwm_configured = false;
    }
    else
    {
        // Configure the LEDC channel
        ledc_channel_config_t ledc_channel = {
            .speed_mode = mode,
            .channel = channel, // Use channel 0
            .timer_sel = timer, // Use timer 0
            .intr_type = LEDC_INTR_DISABLE,
            .gpio_num = LED_PWM_GPIO, // LED PWM GPIO pin is pin32
            .duty = initial_duty, // Start at the active day/night percent
            .hpoint = 0};
        err = ledc_channel_config(&ledc_channel);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "PWM channel config failed: %s", esp_err_to_name(err));
            led_pwm_configured = false;
        }
        else
        {
            led_pwm_configured = true;
            ESP_LOG_WEB(ESP_LOG_INFO, TAG, "PWM LED GPIO32 %lu Hz 10bit safe_max=%u effective_max=%u start=%u%% duty=%u",
                        (unsigned long)freq, pwm_get_safe_maximum_power(),
                        pwm_get_effective_max_power(), initial_pct, initial_duty);
        }
    }

    // Analog dimming on IO25 (DAC1). PWM boards leave this pin unused;
    // analog-driver boards ignore IO32.
    startup_led_dac(initial_duty);

    // Display only reapplies brightness when font_index changes, so boot must
    // set the saved day/night level here or the LED stays at the init duty.
    set_led_pwm_brightness(active_brightness_percent());
}

// Reconfigure PWM frequency (called when settings are updated)
esp_err_t reconfigure_led_pwm_frequency(void)
{
    ledc_mode_t mode = LEDC_LOW_SPEED_MODE;
    ledc_timer_t timer = LEDC_TIMER_0;
    
    uint32_t freq = normalize_pwm_frequency(eeprom_pwm_frequency);
    
    // Configure the LEDC timer with new frequency
    ledc_timer_config_t ledc_timer = {
        .speed_mode = mode,                    // Low-speed mode
        .timer_num = timer,                    // Use timer 0
        .duty_resolution = LEDC_TIMER_10_BIT,  // 10-bit resolution
        .freq_hz = freq,                       // New frequency from settings
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
    int pwm_duty = (int)duty_from_percent(duty);
    uint8_t dac_val = scale_10bit_to_dac8((uint16_t)pwm_duty);
    ESP_LOG_WEB(ESP_LOG_INFO, TAG, "LED brightness %i%% (pwm duty %i, dac %u)", duty, pwm_duty, dac_val);
    if (!led_pwm_configured && !led_dac_configured)
    {
        ESP_LOG_WEB(ESP_LOG_WARN, TAG, "LED brightness skipped: PWM/DAC not configured");
        return;
    }
    current_brightness = duty;

    if (led_pwm_configured)
    {
        esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, pwm_duty);
        if (err != ESP_OK)
        {
            ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LED duty set failed: %s", esp_err_to_name(err));
        }
        else
        {
            err = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
            if (err != ESP_OK)
            {
                ESP_LOG_WEB(ESP_LOG_ERROR, TAG, "LED duty update failed: %s", esp_err_to_name(err));
            }
        }
    }

    apply_led_dac((uint16_t)pwm_duty);
}
