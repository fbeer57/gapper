/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include "main.h"
#include "wifi.h"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"

/*set the ssid and password via "make menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PWD CONFIG_WIFI_PASSWORD

#define DEFAULT_LISTEN_INTERVAL CONFIG_WIFI_LISTEN_INTERVAL

#if CONFIG_POWER_SAVE_MIN_MODEM
#   define DEFAULT_PS_MODE WIFI_PS_MIN_MODEM
#elif CONFIG_POWER_SAVE_MAX_MODEM
#   define DEFAULT_PS_MODE WIFI_PS_MAX_MODEM
#elif CONFIG_POWER_SAVE_NONE
#   define DEFAULT_PS_MODE WIFI_PS_NONE
#else
#   define DEFAULT_PS_MODE WIFI_PS_NONE
#endif /*CONFIG_POWER_SAVE_MODEM*/

static bool s_reconnect = true;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
	    ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
    	ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "got ip:%s\n", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        if (s_reconnect)
        {
            ESP_ERROR_CHECK(esp_wifi_connect());
        }
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/*init wifi as sta and set power save mode*/
void wifi_start(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    s_reconnect = true;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
            .listen_interval = DEFAULT_LISTEN_INTERVAL,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_ps(DEFAULT_PS_MODE);
}

void wifi_stop(void)
{
    s_reconnect = false;
    esp_wifi_stop();
}

