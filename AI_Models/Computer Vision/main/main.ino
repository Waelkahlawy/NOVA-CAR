#include <Arduino.h>
#include "src/Config.h"
#include "src/Camera/Camera.h"
#include "src/AI_Engine/AI_Engine.h"
#include "src/Network_Manager/http.h"
#include "src/WebServer/WebServer.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

AIEngine aiEngine;
String currentLabel = "Waiting...";
float currentScore = 0.0;

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(SYS_BAUD_RATE);

    if (!CameraModule::init()) {
        Serial.println("Camera Init Failed");
        while(1) delay(100);
    }
    
    if (!aiEngine.init()) {
        Serial.println("AI Engine Init Failed");
        while(1) delay(100);
    }

    HttpManager::init(); // Connect WiFi & Start API
    AppWebServer::init(); // Start Web & Video Stream

    Serial.println("System Ready!");
}

void loop() {
    // 1. Handle API requests
    HttpManager::run(currentLabel.c_str(), currentScore);

    // 2. AI Inference
    camera_fb_t* fb = CameraModule::takePicture();
    if (fb) {
        AIResult res = aiEngine.runModel(fb);
        currentLabel = res.label;
        currentScore = res.score;
        
        CameraModule::freePicture(fb);
    }
    
    delay(10);
}