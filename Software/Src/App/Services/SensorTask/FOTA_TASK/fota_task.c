#include "fota_task.h"
#include "../../Modules_Driver/FOTA/fota.h"
#include "../../Modules_Driver/MQTT/mqtt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = TAG_FOTA_TASK ;

// MQTT Callback for FOTA
void Fota_MqttCallback(const char *topic, const char *data)
{
    ESP_LOGI(TAG, " %s => %s", topic, data);

    // FOTA trigger
    if (strcmp(topic, MQTT_TOPIC_ESP1_FOTA_UPDATE_CHECK) == 0 && strcmp(data, "check") == 0) {
        ESP_LOGI(TAG, "Checking for updates...");
        Mqtt_Publish(MQTT_TOPIC_ESP1_FOTA_UPDATE_STATUS, "checking", 1, 0);
        
        int result = Fota_CheckUpdate();
        
        if (result == 0) {
            Mqtt_Publish(MQTT_TOPIC_ESP1_FOTA_UPDATE_STATUS, "up_to_date", 1, 0);
        } else {
            Mqtt_Publish(MQTT_TOPIC_ESP1_FOTA_UPDATE_STATUS, "failed", 1, 0);
        }
    }
}

// FOTA Task - Auto-check every 24 hours
void Fota_Task(void *pvParameters)
{
     // Setup MQTT
    Mqtt_SetDataCallback(Fota_MqttCallback);
    Mqtt_Subscribe(MQTT_TOPIC_ESP1_FOTA_UPDATE_CHECK, 1);

    // Publish status
    char status[64];
    snprintf(status, sizeof(status), "online_v%s", Fota_GetCurrentVersion());
    Mqtt_Publish(MQTT_TOPIC_ESP1_FOTA_UPDATE_STATUS, status, 1, 1);
    
    uint32_t check_interval = 24 * 60 * 60 * 1000;  // 24 hours in ms
    
    while (1) {
        ESP_LOGI(TAG, " Daily FOTA check...");
        Fota_CheckUpdate();
        
        vTaskDelay(pdMS_TO_TICKS(check_interval));
    }
}

