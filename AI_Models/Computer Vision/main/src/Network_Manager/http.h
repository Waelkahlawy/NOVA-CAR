#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <WiFi.h>
#include "../Config.h"

class HttpManager {
public:
    static void init();
    static void run(const char* label, float score);

private:
    static WiFiServer apiServer;
};

#endif