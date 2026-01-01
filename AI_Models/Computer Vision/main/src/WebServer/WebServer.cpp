#include "WebServer.h"
#include "../Camera/Camera.h"
#include <Arduino.h>
#include <SD_MMC.h>  
#include <FS.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"

httpd_handle_t web_httpd = NULL;
httpd_handle_t stream_httpd = NULL;


esp_err_t sendFile(httpd_req_t *req, String filename, String mimeType) {
    
    File file = SD_MMC.open(filename, "r");
    if (!file) {
        Serial.printf("File %s not found on SD Card!\n", filename.c_str());
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, mimeType.c_str());

    char chunk[1024];
    size_t chunksize;
    while (file.available()) {
        chunksize = file.read((uint8_t *)chunk, 1024);
        if (chunksize > 0) {
            httpd_resp_send_chunk(req, chunk, chunksize);
        }
    }
    file.close();
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// ---------------------------------------------------------

void AppWebServer::init() {
    
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("WebServer: No SD Card attached");
        return;
    }
    Serial.println("WebServer: Using SD Card for HTML files");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = CFG_WEB_PORT;

    httpd_uri_t index_uri = { "/", HTTP_GET, index_handler, NULL };
    httpd_uri_t css_uri = { "/main.css", HTTP_GET, css_handler, NULL };
    httpd_uri_t js_uri = { "/main.js", HTTP_GET, js_handler, NULL };

    if (httpd_start(&web_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(web_httpd, &index_uri);
        httpd_register_uri_handler(web_httpd, &css_uri);
        httpd_register_uri_handler(web_httpd, &js_uri);
    }
    
    startStreamServer();
}

void AppWebServer::startStreamServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = CFG_STREAM_PORT;
    config.ctrl_port = CFG_STREAM_PORT;
    
    httpd_uri_t stream_uri = { "/stream", HTTP_GET, stream_handler, NULL };
    
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}



esp_err_t AppWebServer::index_handler(httpd_req_t *req) {
    return sendFile(req, "/data/main.html", "text/html");
}

esp_err_t AppWebServer::css_handler(httpd_req_t *req) {
    return sendFile(req, "/data/main.css", "text/css");
}

esp_err_t AppWebServer::js_handler(httpd_req_t *req) {
    return sendFile(req, "/data/main.js", "application/javascript");
}


esp_err_t AppWebServer::stream_handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    char *part_buf[64];
    static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=123456789000000000000987654321";
    static const char* _STREAM_BOUNDARY = "\r\n--123456789000000000000987654321\r\n";
    static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;

    while(true) {
        fb = CameraModule::takePicture();
        if (!fb) { res = ESP_FAIL; break; }

        uint8_t * buf = fb->buf;
        size_t len = fb->len;
        uint8_t * jpg_buf = NULL;
        size_t jpg_len = 0;
        bool converted = false;

        if(fb->format != PIXFORMAT_JPEG){
            converted = frame2jpg(fb, 80, &jpg_buf, &jpg_len);
            CameraModule::freePicture(fb);
            if(!converted) { res = ESP_FAIL; break; }
            buf = jpg_buf;
            len = jpg_len;
        }

        if(res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if(res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)buf, len);

        if(converted) free(jpg_buf);
        else CameraModule::freePicture(fb);

        if(res != ESP_OK) break;
    }
    return res;
}