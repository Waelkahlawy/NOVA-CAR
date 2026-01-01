#ifndef CAMERA_MODULE_H
#define CAMERA_MODULE_H

#include "esp_camera.h"
#include "../Config.h"

class CameraModule {
public:
    
    static bool init();
    
    
    static camera_fb_t* takePicture();
    
    
    static void freePicture(camera_fb_t* fb);

    
};

#endif