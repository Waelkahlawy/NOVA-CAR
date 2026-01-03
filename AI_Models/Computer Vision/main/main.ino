#include <Arduino.h>
#include "src/Config.h"

// Include Modules
#include "src/hal/Camera/Camera_HAL.h"
#include "src/hal/Network/WiFi_Manager.h"
#include "src/app/AI/AI_Engine.h"
#include "src/app/Comm/MQTT_Manager.h"
#include "src/app/Web/WebServer.h"
#include "src/app/Telegram_bot/Telegram_Manager.h"
#include "src/mcal/uart/uart_driver.h" 
#include "img_converters.h" 

// --- Global Shared Data ---
SemaphoreHandle_t xDataMutex;
String g_lastLabel = "Init...";
float g_lastScore = 0.0;
int g_lastIndex = 0;

// --- Global Variables for Image Sharing ---
uint8_t* g_alertJpgBuf = NULL; // Pointer to image buffer
size_t g_alertJpgLen = 0;      // Image size
bool g_readyToSend = false;    // Flag to signal Task_Comm

// --- Task Handles ---
TaskHandle_t hTaskAI = NULL;
TaskHandle_t hTaskComm = NULL;

#if CFG_MODULE_AI == ENABLED
AIEngine aiEngine;
#endif

// =============================================================
//  TASK 1: AI Processing (Capture & Convert)
// =============================================================
void Task_AI(void *pvParameters) {
    unsigned long lastSnapTime = 0;
    
    while(1) {
        camera_fb_t* fb = CameraHAL::capture();
        
        if (fb) {
            #if CFG_MODULE_AI == ENABLED
            AIResult res = aiEngine.run(fb);
            
            if (res.valid) {
                // Update shared data
                if (xSemaphoreTake(xDataMutex, portMAX_DELAY) == pdTRUE) {
                    g_lastLabel = res.label;
                    g_lastScore = res.score;
                    g_lastIndex = res.index;
                    xSemaphoreGive(xDataMutex);
                }

                // === Logic for Telegram Photo ===
                // Conditions: Danger detected (index != 0), No photo currently sending, Cooldown passed
                if (res.index != 0 && !g_readyToSend && (millis() - lastSnapTime > 5000)) {
                    
                    // Convert current frame (fb) to JPEG
                    uint8_t* tempBuf = NULL;
                    size_t tempLen = 0;
                    bool converted = frame2jpg(fb, 20, &tempBuf, &tempLen);

                    if (converted) {
                        Serial.println("AI: Danger Detected! Photo Converted.");
                        g_alertJpgBuf = tempBuf;
                        g_alertJpgLen = tempLen;
                        g_readyToSend = true; // Raise flag
                        lastSnapTime = millis();
                    }
                }
                
                // Send via UART (Fast)
                UARTDriver::sendResult(res.index, res.score);
            }
            #endif
            CameraHAL::release(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// =============================================================
//  TASK 2: Communication (Send Only)
// =============================================================
void Task_Comm(void *pvParameters) {
    #if CFG_MODULE_TELEGRAM == ENABLED
    TelegramManager::init();
    #endif

    vTaskDelay(pdMS_TO_TICKS(2000));

    while(1) {
        if (!WiFiManager::isConnected()) {
            WiFiManager::connect();
            vTaskDelay(pdMS_TO_TICKS(500));
            continue;
        }

        // --- Check if photo is ready to send ---
        if (g_readyToSend && g_alertJpgBuf != NULL) {
            
            int idx; String lbl; float scr;
            if (xSemaphoreTake(xDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
                idx = g_lastIndex;
                lbl = g_lastLabel;
                scr = g_lastScore;
                xSemaphoreGive(xDataMutex);
            }

            #if CFG_MODULE_TELEGRAM == ENABLED
            TelegramManager::sendIPG(idx, lbl, scr, g_alertJpgBuf, g_alertJpgLen);
            #endif

            // *** IMPORTANT: Free memory after sending ***
            free(g_alertJpgBuf);
            g_alertJpgBuf = NULL;
            g_alertJpgLen = 0;
            g_readyToSend = false; 
            
            Serial.println("Comm: Photo Memory Freed.");
        }

        #if CFG_MODULE_MQTT == ENABLED
        MQTTManager::loop();
        #endif

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// =============================================================
//  SETUP
// =============================================================
void setup() {

    
    Serial.begin(115200);
    UARTDriver::init();
    
    Serial.println("\n\n--- SYSTEM STARTING ---");


    WiFi.setTxPower(WIFI_POWER_19dBm); 

    xDataMutex = xSemaphoreCreateMutex();

    Serial.println("1. Init Camera...");
    if (!CameraHAL::init()) {
        Serial.println("ERR: Camera Init Failed");
        while(1) delay(100);
    }
    
    #if CFG_MODULE_AI == ENABLED
    Serial.println("2. Init AI Engine...");
    if (!aiEngine.init()) {
         Serial.println("ERR: AI Init Failed");
    }
    #endif

    Serial.println("3. Connecting WiFi...");
    WiFiManager::connect();
    
    #if CFG_MODULE_WEBSERVER == ENABLED
    WebServer::init();
    #endif

    Serial.println("5. Starting Tasks...");
    xTaskCreatePinnedToCore(Task_AI, "AI_Task", 10 * 1024, NULL, 2, &hTaskAI, 1);
    xTaskCreatePinnedToCore(Task_Comm, "Comm_Task", 8 * 1024, NULL, 1, &hTaskComm, 0);
    
    Serial.println("--- SYSTEM READY ---");
}

void loop() { vTaskDelete(NULL); }