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

#include "esp_system.h"
#include "esp_pm.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include "wifi.h"

EventGroupHandle_t wifi_event_group;

void app_shutdown(void);

void app_main()
{
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    int bits;

    ESP_LOGI(TAG, "Starting app");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_esp32_t pm_config = {
            .max_freq_mhz = CONFIG_DYNAMIC_MAX_CPU_FREQ_MHZ,
            .min_freq_mhz = CONFIG_DYNAMIC_MIN_CPU_FREQ_MHZ,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
            .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
#endif // CONFIG_PM_ENABLE

    wifi_event_group = xEventGroupCreate();

    wifi_start();

    bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, MAX_CONNECT_WAIT);
    if ((bits & CONNECTED_BIT) != CONNECTED_BIT)
    {
        ESP_LOGE(TAG, "Timed out waiting for connection");
        app_shutdown();
    }

    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time(&now);
    localtime_r(&now, &timeinfo);
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count);
    {
        // wait for time to be set
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        SleepFor(2000);
        time(&now);
        localtime_r(&now, &timeinfo);
    } 
    ESP_LOGI(TAG, "system time has been set.");
    xEventGroupSetBits(wifi_event_group, TIME_SYNCD_BIT);

    app_shutdown();
}

void app_shutdown()
{
    // esp_bluedroid_disable();
    // esp_bt_controller_disable();
    wifi_stop();

    esp_deep_sleep(15 /* * 60 */ * 1000000);  // ms
}
