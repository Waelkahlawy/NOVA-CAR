#ifndef GPIO_H   // GPIO_H
#define GPIO_H // GPIO_H

#include "../../Cfg.h" // Include project configuration

#if GPIO_ENABLED == STD_ON // Enable GPIO driver
#include "esp_log.h" // ESP32 logging
#include "driver/gpio.h"   // GPIO driver header

// GPIO Configuration Structure
typedef struct {
    gpio_num_t pin_num;         // GPIO pin number
    gpio_mode_t pin_mode;       // Pin mode (input/output)
    uint8_t pin_value;          // Pin value (0 or 1)
} Gpio_ConfigType;



void Gpio_InitPin(Gpio_ConfigType *pinConfig); // Initialize GPIO pin


uint8_t Gpio_ReadPinValue(gpio_num_t pin_num); // Read value from GPIO pin


void Gpio_WritePinValue(Gpio_ConfigType *pinConfig); // Write value to GPIO pin

#else 
 ESP_LOGI(g_TAG, " GPIO feature is disabled "); 
return -1;


#endif // GPIO_ENABLED



#endif //GPIO_H