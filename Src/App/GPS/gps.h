#ifndef GPS_H_
#define GPS_H_

#include <stdio.h>
#include <sys/_intsup.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "hal/ledc_types.h"
#include "portmacro.h"
#include "esp_log.h"
#include "../../Hal/UART/uart.h"



typedef struct 
{
    float Latitude;
    float Longitude;
    float Altitude;
}Gps_Location_t;



void GPS_Init   (void);
void GPS_RawNmea(void);




#endif /* GPS_H_ */