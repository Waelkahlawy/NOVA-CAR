#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "esp_http_server.h"
#include "../Config.h"

class AppWebServer {
public:
    static void init();

private:
    static esp_err_t index_handler(httpd_req_t *req);
    static esp_err_t css_handler(httpd_req_t *req);
    static esp_err_t js_handler(httpd_req_t *req);
    static esp_err_t stream_handler(httpd_req_t *req);
    
    static void startStreamServer();
};

#endif