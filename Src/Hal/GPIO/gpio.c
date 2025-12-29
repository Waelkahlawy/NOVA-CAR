#include "gpio.h"

#if GPIO_ENABLED == STD_ON

#include "esp_log.h"

static const char *g_TAG = TAG_GPIO;

void Gpio_InitPin(Gpio_ConfigType *pinConfig)
{
    // Configure pull-up/pull-down mode
    gpio_set_pull_mode(pinConfig->pin_num, pinConfig->pin_pull);

    // Set pin direction (input/output/open-drain)
    gpio_set_direction(pinConfig->pin_num, pinConfig->pin_mode);

    // If pin is configured as output, set initial value
    if (pinConfig->pin_mode == GPIO_MODE_OUTPUT ||
        pinConfig->pin_mode == GPIO_MODE_OUTPUT_OD) {
        gpio_set_level(pinConfig->pin_num, pinConfig->pin_value);
    }

#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "GPIO %d initialized - Mode: %d, Initial Value: %d, Pull: %d",
             pinConfig->pin_num, pinConfig->pin_mode, pinConfig->pin_value, pinConfig->pin_pull);
#endif
}


uint8_t Gpio_ReadPinValue(gpio_num_t pin_num)
{
    uint8_t value = gpio_get_level(pin_num);

#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "GPIO %d read - Value: %d", pin_num, value);
#endif

    return value;
}


void Gpio_WritePinValue(Gpio_ConfigType *pinConfig)
{
    gpio_set_level(pinConfig->pin_num, pinConfig->pin_value);

#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "GPIO %d write - Value: %d", pinConfig->pin_num, pinConfig->pin_value);
#endif
}

#else
// Stub implementations when GPIO driver is disabled

void Gpio_InitPin(Gpio_ConfigType *pinConfig)
{
    // Do nothing
}

uint8_t Gpio_ReadPinValue(gpio_num_t pin_num)
{
    return 0;
}

void Gpio_WritePinValue(Gpio_ConfigType *pinConfig)
{
    // Do nothing
}

#endif // GPIO_ENABLED