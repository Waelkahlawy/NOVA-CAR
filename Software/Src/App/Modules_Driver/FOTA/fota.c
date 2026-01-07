#include "fota.h"

#if FOTA_ENABLED == STD_ON

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "cJSON.h"
#include <string.h>

// Debug TAG
static const char *TAG = TAG_FOTA ;

void Fota_Init(void)
{
    // Get current firmware info
    const esp_app_desc_t *app_desc = esp_app_get_description();
    
#if FOTA_DEBUG_ENABLED == STD_ON
    ESP_LOGI(TAG, "FOTA initialized");
    ESP_LOGI(TAG, "Current version: %s", FOTA_CURRENT_VERSION);
    ESP_LOGI(TAG, "App name: %s", app_desc->project_name);
    ESP_LOGI(TAG, "IDF version: %s", app_desc->idf_ver);
#endif
}

const char* Fota_GetCurrentVersion(void)
{
    return FOTA_CURRENT_VERSION;
}

int Fota_CheckUpdate(void)
{

    char buffer[1024] = {0};
    int content_length = 0;

    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = FOTA_VERSION_URL,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // Perform GET request
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
#if FOTA_DEBUG_ENABLED == STD_ON
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
#endif
        esp_http_client_cleanup(client);
        return -1;
    }

    content_length = esp_http_client_fetch_headers(client);
    
    if (content_length > 0 && content_length < sizeof(buffer)) {
        int read_len = esp_http_client_read(client, buffer, content_length);
        buffer[read_len] = '\0';

#if FOTA_DEBUG_ENABLED == STD_ON
        ESP_LOGI(TAG, "Received version info: %s", buffer);
#endif

        // Parse JSON
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
#if FOTA_DEBUG_ENABLED == STD_ON
            ESP_LOGE(TAG, "Failed to parse JSON");
#endif
            esp_http_client_cleanup(client);
            return -1;
        }

        cJSON *version = cJSON_GetObjectItem(json, "version");
        cJSON *url = cJSON_GetObjectItem(json, "url");

        if (cJSON_IsString(version) && cJSON_IsString(url)) {
            const char *new_version = version->valuestring;
            const char *firmware_url = url->valuestring;

#if FOTA_DEBUG_ENABLED == STD_ON
            ESP_LOGI(TAG, "Current: %s | Available: %s", FOTA_CURRENT_VERSION, new_version);
#endif

            // Compare versions
            if (strcmp(FOTA_CURRENT_VERSION, new_version) != 0) {
#if FOTA_DEBUG_ENABLED == STD_ON
                ESP_LOGI(TAG, "New version available: %s", new_version);
                ESP_LOGI(TAG, "Firmware URL: %s", firmware_url);
#endif
                
                // Perform update
                int result = Fota_PerformUpdate(firmware_url);
                
                cJSON_Delete(json);
                esp_http_client_cleanup(client);
                return result;
            } else {
#if FOTA_DEBUG_ENABLED == STD_ON
                ESP_LOGI(TAG, "Already on latest version");
#endif
            }
        }

        cJSON_Delete(json);
    }

    esp_http_client_cleanup(client);
    return 0;

}

int Fota_PerformUpdate(const char *firmware_url)
{
#if FOTA_DEBUG_ENABLED == STD_ON
    ESP_LOGI(TAG, "Starting OTA update...");
    ESP_LOGI(TAG, "Downloading from: %s", firmware_url);
#endif

    esp_http_client_config_t config = {
        .url = firmware_url,
        .timeout_ms = 30000,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_err_t err = esp_https_ota(&ota_config);

    if (err == ESP_OK) {
#if FOTA_DEBUG_ENABLED == STD_ON
        ESP_LOGI(TAG, "OTA update successful!");
        ESP_LOGI(TAG, "Rebooting in 3 seconds...");
#endif
        vTaskDelay(pdMS_TO_TICKS(3000));
        esp_restart();
    } else {
#if FOTA_DEBUG_ENABLED == STD_ON
        ESP_LOGE(TAG, "OTA update failed: %s", esp_err_to_name(err));
#endif
        return -1;
    }

    return 0;

}

#else

// Stub functions when FOTA is disabled
void Fota_Init(void)
{
    // Do nothing
}

int Fota_CheckUpdate(void)
{
    return -1;
}

int Fota_PerformUpdate(const char *firmware_url)
{
    return -1;
}

const char* Fota_GetCurrentVersion(void)
{
    return "disabled";
}

#endif // FOTA_ENABLED