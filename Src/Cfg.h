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
#define TAG_IMU         "IMU_DRIVER"
#define TAG_MQTT        "MQTT_DRIVER"
#define TAG_LDR         "LDR_SENSOR"




// Enable or disable drivers
#define GPIO_ENABLED            STD_ON
#define ADC_ENABLED             STD_ON
#define I2C_ENABLED             STD_ON
#define UART_ENABLED            STD_ON
#define WIFI_ENABLED            STD_ON
#define IMU_ENABLED             STD_ON
#define MQTT_ENABLED            STD_ON
#define LDR_ENABLED             STD_ON

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
#define WIFI_PASSWORD           "Null@987897"
#define WIFI_DEBUG_ENABLED      STD_ON
#endif

#if IMU_ENABLED == STD_ON
#define IMU_DEBUG_ENABLED       STD_ON
// MPU6050 I2C Address
#define MPU6050_ADDR            0x68

// MPU6050 Registers
#define MPU6050_PWR_MGMT_1      0x6B
#define MPU6050_WHO_AM_I        0x75
#define MPU6050_ACCEL_XOUT_H    0x3B
#define MPU6050_GYRO_XOUT_H     0x43
#define MPU6050_TEMP_OUT_H      0x41

#define ACCEL_SENS_2G      16384.0f     // LSB/g
#define GYRO_SENS_250DPS   131.0f       // LSB/(deg/s)

#endif // IMU_ENABLED


// MQTT Configuration & Debugging
#if MQTT_ENABLED == STD_ON
#define MQTT_BROKER_URI         "mqtt://192.168.1.7:1883"
#define MQTT_CLIENT_ID          "esp32_client"
#define MQTT_USERNAME            "WaelKahlawy"
#define MQTT_PASSWORD            "123456789"
#define MQTT_DEBUG_ENABLED      STD_ON
#endif


// LDR Configuration & Debugging
#if LDR_ENABLED == STD_ON
#define LDR_ADC_CHANNEL         ADC_CHANNEL_6       // GPIO34
#define LDR_MIN_VALUE           0                   // Dark (raw ADC)
#define LDR_MAX_VALUE           4095                // Bright (raw ADC)
#define LDR_MIN_LIGHT           0                   // Mapped: Dark = 0%
#define LDR_MAX_LIGHT           100                 // Mapped: Bright = 100%
#define LDR_DEBUG_ENABLED       STD_ON
#endif

























#endif 
