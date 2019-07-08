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

void upload_file(const char* type, const uint8_t* udata, size_t length, const char* suffix)
{
    time_t now;
    struct tm timeinfo;

    esp_http_client_config_t config = {
        .host = "www.blaubeer.eu",
        .path = "/garden-test/raw-upload.php",
        .port = 80,
        .username = CONFIG_UPLOAD_USER,
        .password = CONFIG_UPLOAD_PASSWORD,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .method = HTTP_METHOD_POST,
    };
    int http_status_code;

    char dirname[12];
    char filename[18];

    time(&now);
    gmtime_r(&now, &timeinfo);


    strftime(dirname, sizeof(dirname), "%Y%m%d", &timeinfo);
    strftime(filename, sizeof(filename), "%Y%m%d%H%M%S", &timeinfo);

    ESP_LOGI(TAG, "free heap is %d KBytes", esp_get_free_heap_size() / 1024);   // 141 K free heap on ESP32 WROOM

    esp_http_client_handle_t http_client = esp_http_client_init(&config);
    esp_http_client_open(http_client, length);

    size_t written = esp_http_client_write(http_client, (const char*)udata, length);
    ESP_LOGI(TAG, "Wrote %d/%d bytes", written, length);

    esp_http_client_fetch_headers(http_client);
    http_status_code = esp_http_client_get_status_code(http_client);
    ESP_LOGI(TAG, "Server responded with %d status", http_status_code);
    esp_http_client_close(http_client);

    if (esp_http_client_cleanup(http_client) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_http_client_cleanup() failed");
    }
}
