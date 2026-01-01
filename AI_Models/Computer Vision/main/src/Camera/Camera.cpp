#include "Camera.h"
#include <Arduino.h>
#include "../../board_config.h" 

bool CameraModule::init() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = CFG_CAM_XCLK_FREQ;
    config.pixel_format = CFG_CAM_PIXFORMAT;
    config.frame_size = CFG_CAM_FRAMESIZE;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    if (esp_camera_init(&config) != ESP_OK) {
        return false;
    }
    
    sensor_t *s = esp_camera_sensor_get();
    // if (s) {
    //     s->set_vflip(s, 1); 
    //     s->set_hmirror(s, 1); 
    // }
    return true;
}

camera_fb_t* CameraModule::takePicture() {
    return esp_camera_fb_get();
}

void CameraModule::freePicture(camera_fb_t* fb) {
    esp_camera_fb_return(fb);
}