#ifndef CONFIG_H_
#define CONFIG_H_

// --- System ---
#define SYS_SERIAL_DEBUG        1
#define SYS_BAUD_RATE           115200

// --- WiFi ---
#define CFG_WIFI_SSID           "DESKTOP-UC4MCI1 1119"
#define CFG_WIFI_PASS           "L279<10g"
#define CFG_API_PORT            82
#define CFG_WEB_PORT            80
#define CFG_STREAM_PORT         81

// --- Camera ---
#define CFG_CAM_FRAMESIZE       FRAMESIZE_QVGA
#define CFG_CAM_PIXFORMAT       PIXFORMAT_RGB565
#define CFG_CAM_XCLK_FREQ       20000000

// --- AI Model ---
#define CFG_MODEL_USE_SD        1   // 1 for SD Card, 0 for Flash
#define CFG_SD_MOUNT_POINT      "/sdcard"
#define CFG_MODEL_FILENAME      "/model.tflite"
#define CFG_AI_ARENA_SIZE       (800 * 1024)
#define CFG_CONFIDENCE_THR      0.40f

#endif