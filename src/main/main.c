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
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/apps/sntp.h"

#include "board.h"
#include "ble_scan.h"
#include "wifi.h"
#include "measure_task.h"

static void configure_power_management(void);
static void initialize_nvs(void);
static void initialize_sntp(void);

static void print_wakeup_reason(esp_sleep_wakeup_cause_t);

static void print_time(time_t now, const char* message);
static void set_base_time(int year, int month, int day, int hour, int minute, int second);
static uint64_t determine_sleep_time(time_t now);

static time_t base_time;

void app_main()
{
    time_t now = 0;
    uint64_t sleep_us;
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    set_base_time(2019, 6, 10, 9, 36, 27);
    setup_board();

    uint32_t battery_milliVolts = get_battery_millivolts();
    if (battery_milliVolts > 11700)
    {
        initialize_nvs();
        configure_power_management();

        print_wakeup_reason(wakeup_reason);

        wifi_event_group = xEventGroupCreate();

        setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/2", 1);
        tzset();

        switch_router(1);

        xTaskCreate(&measure_task, "measure_task", 4096, NULL, 5, NULL);
        
        ble_start();
        SleepFor(CONFIG_ROUTER_WAIT);
        ble_stop();

        wifi_start();

        BEGIN_WAIT_SEQUENCE

            WAIT_AND_BAIL(CONNECTED_BIT, CONFIG_MAX_CONNECT_WAIT, "Timed out waiting for connection")

            initialize_sntp();

            if (!WaitFor(TIME_SYNCD_BIT, CONFIG_MAX_SNTP_WAIT))
            {
                struct tm timeinfo;

                time(&now);
                gmtime_r(&now, &timeinfo);
                if (timeinfo.tm_year < (2019 - 1900))
                {
                    ESP_LOGE(TAG, "Timed out waiting for obtaining valid time");
                    break;
                }
                ESP_LOGI(TAG, "Continuing using RTC time");
                SetEvent(TIME_SYNCD_BIT);
            }

            // wait until all is done
            WAIT_AND_BAIL(DATA_SENT_BIT, CONFIG_MAX_SEND_WAIT, "Timed out waiting to send data")

        END_WAIT_SEQUENCE

        // shut down

        wifi_stop();

        switch_router(0);
    }

    time(&now);
    sleep_us = determine_sleep_time(now);

    print_time(now + (sleep_us / 1000000), "next wakeup at");

    esp_sleep_enable_timer_wakeup(sleep_us);
    esp_deep_sleep_start();
}

static uint64_t determine_sleep_time(time_t now)
{
    uint64_t usec = CONFIG_SLEEP_INTERVAL;
    if (now >= base_time)
    {
        usec = base_time - now + ((now - base_time) / CONFIG_SLEEP_INTERVAL + 1) * CONFIG_SLEEP_INTERVAL; 
    }
    return usec * 1000000L;
}

static void set_base_time(int year, int month, int day, int hour, int minute, int second)
{
    struct tm base = {
        .tm_year = year - 1900,
        .tm_mon = month - 1,
        .tm_mday = day,
        .tm_hour = hour,
        .tm_min = minute,
        .tm_sec = second
    };
    base_time = mktime(&base);
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

static void initialize_nvs(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (   ret == ESP_ERR_NVS_NO_FREE_PAGES
        || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

static void configure_power_management(void)
{
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
}

static void time_sync_notification_cb(struct timeval *tv)
{
    print_time(tv->tv_sec, "system time has been set to");
    xEventGroupSetBits(wifi_event_group, TIME_SYNCD_BIT);
}

static void initialize_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}