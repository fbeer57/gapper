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
#include "esp_sleep.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include "wifi.h"

static void print_wakeup_reason(esp_sleep_wakeup_cause_t);
static uint64_t determine_sleep_time(void);
static void print_time(time_t now, const char* message);

static void time_sync_notification_cb(struct timeval *tv)
{
        print_time(tv->tv_sec, "system time has been set to");
        xEventGroupSetBits(wifi_event_group, TIME_SYNCD_BIT);
}

void app_main()
{
    time_t now = 0;
    int bits;
    uint64_t sleep_us = 30000000;           // sleep 30s by default
    esp_sleep_wakeup_cause_t wakeup_reason;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (   ret == ESP_ERR_NVS_NO_FREE_PAGES
        || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

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

    wakeup_reason = esp_sleep_get_wakeup_cause();
    print_wakeup_reason(wakeup_reason);

    setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/2", 1);
    tzset();

    wifi_start();

    do
    {
        bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, MAX_CONNECT_WAIT);
        if ((bits & CONNECTED_BIT) != CONNECTED_BIT)
        {
            ESP_LOGE(TAG, "Timed out waiting for connection");
            break;
        }

        time(&now);
        print_time(now, "time before sync is");

        ESP_LOGI(TAG, "Initializing SNTP");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
        sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
        sntp_init();
        

        bits = xEventGroupWaitBits(wifi_event_group, TIME_SYNCD_BIT, false, true, MAX_SNTP_WAIT);
        if ((bits & TIME_SYNCD_BIT) != TIME_SYNCD_BIT)
        {
            // if (timeinfo.tm_year < (2016 - 1900))
            // {
            //     ESP_LOGE(TAG, "Timed out waiting for obtaining valid time");
            //     break;
            // }
            ESP_LOGI(TAG, "Continuing using RTC time");
            xEventGroupSetBits(wifi_event_group, TIME_SYNCD_BIT);
        }

        time(&now);
        print_time(now, "time after sync is");

        sleep_us = determine_sleep_time();

    } while(false);

    // shut down

    // esp_bluedroid_disable();
    // esp_bt_controller_disable();
    wifi_stop();

    time(&now);
    print_time(now +(sleep_us / 1000000), "next wakeup at");

    esp_sleep_enable_timer_wakeup(sleep_us);
    esp_deep_sleep_start();
}

static uint64_t determine_sleep_time()
{
    return 15 * 1000000;
}

static void print_time(time_t now, const char* message)
{
    struct tm timeinfo = { 0 };
    char current_time[48];

    localtime_r(&now, &timeinfo);
    asctime_r(&timeinfo, current_time);
    ESP_LOGI(TAG, "%s %s", message, current_time);
}

static void print_wakeup_reason(esp_sleep_wakeup_cause_t wakeup_reason)
{
    char* reason = "Wakeup reason unknown";
    switch(wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_UNDEFINED: reason = "Reset was not caused by exit from deep sleep"; break;
    case ESP_SLEEP_WAKEUP_ALL:       reason = "Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source"; break;
    case ESP_SLEEP_WAKEUP_EXT0:      reason = "Wakeup caused by external signal using RTC_IO"; break;
    case ESP_SLEEP_WAKEUP_EXT1:      reason = "Wakeup caused by external signal using RTC_CNTL"; break;
    case ESP_SLEEP_WAKEUP_TIMER:     reason = "Wakeup caused by timer"; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:  reason = "Wakeup caused by touchpad"; break;
    case ESP_SLEEP_WAKEUP_ULP:       reason = "Wakeup caused by ULP program"; break;
    case ESP_SLEEP_WAKEUP_GPIO:      reason = "Wakeup caused by GPIO (light sleep only)"; break;
    case ESP_SLEEP_WAKEUP_UART:      reason = "Wakeup caused by UART (light sleep only)"; break;
    }
    ESP_LOGI(TAG, "%s", reason);
}

