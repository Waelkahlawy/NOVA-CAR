
#if ADC_ENABLED == STD_ON

#include "adc_driver.h"

static const char *g_TAG = "ADC_DRIVER";


void Adc_Init(Adc_ConfigType *configurations)
{

#if ADC_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "ADC Initialized");
    ESP_LOGI(g_TAG, "Sensor Channel is: %d", configurations->channel);
    ESP_LOGI(g_TAG, "Sensor ADC Resolution is: %d", configurations->resolution);
#endif
    
    adc1_config_width(configurations->resolution);
    
#if ADC_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "Sensor ADC Attenuation is: %d", configurations->attenuation);
#endif
    
    adc1_config_channel_atten(configurations->channel, configurations->attenuation);

}

uint32_t Adc_ReadValue(adc_channel_t channel)
{

    uint32_t sensorValue = adc1_get_raw(channel);
    
#if ADC_DEBUG_ENABLED == STD_ON
    ESP_LOGI(g_TAG, "Read sensor value from channel %d and the Value is: %lu", channel, sensorValue);
#endif
    
    return sensorValue;

}

#endif 