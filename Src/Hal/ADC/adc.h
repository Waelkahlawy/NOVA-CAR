#ifndef ADC_H
#define ADC_H


#if ADC_ENABLED == STD_ON

#include "config_file.h"
#include "driver/adc.h"


typedef struct {
    adc_channel_t channel;    
    adc_bits_width_t resolution; 
    adc_atten_t attenuation;    
} Adc_ConfigType;


void Adc_Init(Adc_ConfigType *configurations);
uint32_t Adc_ReadValue(adc_channel_t channel);

#endif 

#endif 