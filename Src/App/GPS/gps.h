/* gps.h FILE GAURD */
#ifndef GPS_H_
#define GPS_H_

#if GPS_ENABLED == STD_ON

    /* LIBs INCLUSION */
    #include <stdio.h>
    #include <sys/_intsup.h>
    #include "driver/ledc.h"
    #include "esp_err.h"
    #include "freertos/FreeRTOS.h"
    #include "hal/ledc_types.h"
    #include "portmacro.h"
    #include "esp_log.h"
    #include "../../Hal/UART/uart.h"


    /* GPS LOCATION DATA TYPE */
    typedef struct 
    {
        float Latitude;
        float Longitude;
        float Altitude;
        float Speed;
        float Course;
        float Timestamp;
    }Gps_Parameters_t;

    typedef struct
    {
        uint32_t Hour;
        uint32_t Min;
        uint32_t Sec;
    }GPS_Time_t;

    /* GPS RECIVED BUFFER SIZE Based On Full NMEA Sentences*/
    #define BUFFER_SIZE     128


    #define CAIRO_TIMEZONE       2

    #define KnotsToKmh_ConversionFactor 1.852



    void GPS_Init   (const Uart_ConfigType *UART_Config);
    void GPS_GetData(Gps_Parameters_t *GPS_Parametars);

#endif /* GPS_ON */


#endif /* GPS_H_ */