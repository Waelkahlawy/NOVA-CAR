#ifndef CFG_H
#define CFG_H


// Standard definitions
#define STD_ON      1 // Enabled
#define STD_OFF     0   // Disabled

// Tags for logging
#define TAG_GPIO        "GPIO_DRIVER"
#define TAG_ADC         "ADC_DRIVER"
#define TAG_I2C         "I2C_DRIVER"
#define TAG_UART        "UART_DRIVER"
#define TAG_WIFI        "WIFI_DRIVER"

// Enable or disable drivers
#define GPIO_ENABLED            STD_ON
#define ADC_ENABLED             STD_ON
#define I2C_ENABLED             STD_ON
#define UART_ENABLED            STD_ON
#define WIFI_ENABLED            STD_ON


// Enable or disable debug logs for drivers
// GPIO Debugging
#if GPIO_ENABLED == STD_ON
#define GPIO_DEBUG_ENABLED      STD_ON
#endif
// ADC Debugging
#if ADC_ENABLED == STD_ON
#define ADC_DEBUG_ENABLED       STD_ON
#endif 
// I2C Debugging
#if I2C_ENABLED == STD_ON

#define I2C_DEBUG_ENABLED       STD_ON

#endif
// UART Debugging
#if UART_ENABLED == STD_ON
#define UART_DEBUG_ENABLED      STD_ON
#endif

#if WIFI_ENABLED == STD_ON
#define WIFI_SSID               "WAEL"
#define WIFI_PASSWORD           "Null987897"
#define WIFI_DEBUG_ENABLED      STD_ON
#endif





























#endif 
