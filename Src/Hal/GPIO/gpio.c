#include "gpio.h"  // Include GPIO driver header

#if GPIO_ENABLED == STD_ON // GPIO_ENABLED


static const char *g_TAG = TAG_GPIO ; // Debug TAG
// Initialize GPIO pin
void Gpio_InitPin(Gpio_ConfigType *pinConfig)
{

    
    gpio_set_direction(pinConfig->pin_num, pinConfig->pin_mode); // Set pin direction

    // If output mode, set initial value
    if (pinConfig->pin_mode == GPIO_MODE_OUTPUT) {
        gpio_set_level(pinConfig->pin_num, pinConfig->pin_value);
    }

#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "Initialized pin: %d with mode: %d", pinConfig->pin_num, pinConfig->pin_mode);
#endif

}
// Read value from GPIO pin
uint8_t Gpio_ReadPinValue(gpio_num_t pin_num)
{

    uint8_t pin_value = gpio_get_level(pin_num);
    
#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "Read pin value: %d from GPIO pin: %d", pin_value, pin_num);
#endif

    return pin_value;

}
// Write value to GPIO pin
void Gpio_WritePinValue(Gpio_ConfigType *pinConfig)
{

    gpio_set_level(pinConfig->pin_num, pinConfig->pin_value);
    
#if GPIO_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "Wrote pin value: %d to GPIO pin: %d", pinConfig->pin_value, pinConfig->pin_num);
#endif

}

#else 
 ESP_LOGI(g_TAG, " GPIO feature is disabled "); 
return -1;


#endif /* GPIO_ENABLED */