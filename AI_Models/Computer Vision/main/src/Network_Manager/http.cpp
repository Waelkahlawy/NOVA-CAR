#include "http.h"
#include <Arduino.h>

WiFiServer HttpManager::apiServer(CFG_API_PORT);

void HttpManager::init() {
    WiFi.begin(CFG_WIFI_SSID, CFG_WIFI_PASS);
    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");
    Serial.println(WiFi.localIP());
    
    apiServer.begin();
}

void HttpManager::run(const char* label, float score) {
    WiFiClient client = apiServer.available();
    if (client) {
        while (client.connected() && client.available()) client.read();
        
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Connection: close");
        client.println();
        
        client.printf("{\"label\":\"%s\",\"score\":%.1f}", label, score * 100);
        client.stop();
    }
}