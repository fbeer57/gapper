/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "main.h"
#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_system.h>

void upload_file(const char* type, const uint8_t* data, size_t length, const char* suffix)
{
    time_t now;
    struct tm timeinfo;

    esp_http_client_config_t config = {
        .url = CONFIG_UPLOAD_BASE_URL "/upload.php",
        .username = CONFIG_UPLOAD_USER,
        .password = CONFIG_UPLOAD_PASSWORD,
        .auth_type = HTTP_AUTH_TYPE_BASIC    
    };

    char dirname[12];
    char filename[18];

    time(&now);
    gmtime_r(&now, &timeinfo);

    esp_http_client_handle_t http_client = esp_http_client_init(&config);

    strftime(dirname, sizeof(dirname), "%Y%m%d", &timeinfo);
    strftime(filename, sizeof(filename), "%Y%m%d%H%M%S", &timeinfo);

    ESP_LOGI(TAG, "esp_http_client fun...");
    ESP_LOGI(TAG, "free heap is %d KBytes", esp_get_free_heap_size() / 1024);

    if (esp_http_client_cleanup(http_client) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_http_client_cleanup() failed");
    }
}
