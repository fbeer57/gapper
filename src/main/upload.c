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
#include "upload.h"
#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_system.h>

void upload_file(const char* type, const uint8_t* udata, size_t length, const char* suffix)
{
    char query[64];

    snprintf(query, sizeof(query), "type=%s&suffix=%s", type, suffix);

    esp_http_client_config_t config = {
        .host = CONFIG_UPLOAD_HOST,
        .port = CONFIG_UPLOAD_PORT,
        .path = CONFIG_UPLOAD_PATH,
        .query = query,
        .username = CONFIG_UPLOAD_USER,
        .password = CONFIG_UPLOAD_PASSWORD,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .method = HTTP_METHOD_POST,
    };
    int http_status_code;

    esp_http_client_handle_t http_client = esp_http_client_init(&config);
    esp_http_client_open(http_client, length);

    esp_http_client_write(http_client, (const char*)udata, length);

    esp_http_client_fetch_headers(http_client);
    http_status_code = esp_http_client_get_status_code(http_client);
    ESP_LOGI(TAG, "Server responded with %d status", http_status_code);
    esp_http_client_close(http_client);

    if (esp_http_client_cleanup(http_client) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_http_client_cleanup() failed");
    }
}

void upload_file_scattered(const char* type, gapper_fragment_head_t* head, const char* suffix)
{
    char query[64];

    snprintf(query, sizeof(query), "type=%s&suffix=%s", type, suffix);

    esp_http_client_config_t config = {
        .host = CONFIG_UPLOAD_HOST,
        .port = CONFIG_UPLOAD_PORT,
        .path = CONFIG_UPLOAD_PATH,
        .query = query,
        .username = CONFIG_UPLOAD_USER,
        .password = CONFIG_UPLOAD_PASSWORD,
        .auth_type = HTTP_AUTH_TYPE_BASIC,
        .method = HTTP_METHOD_POST,
    };
    int http_status_code;

    esp_http_client_handle_t http_client = esp_http_client_init(&config);

    size_t length = 0;
    gapper_fragment_t *node;
    STAILQ_FOREACH(node, head, nodes){
        length += node->length;
    }
    esp_http_client_open(http_client, length);

    STAILQ_FOREACH(node, head, nodes){
        esp_http_client_write(http_client, node->payload, node->length);
    }

    esp_http_client_fetch_headers(http_client);
    http_status_code = esp_http_client_get_status_code(http_client);
    ESP_LOGI(TAG, "Server responded with %d status", http_status_code);
    esp_http_client_close(http_client);

    if (esp_http_client_cleanup(http_client) != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_http_client_cleanup() failed");
    }
}
