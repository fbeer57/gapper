/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#ifndef MAIN_H
#define MAIN_H 1

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "curl/curl.h"
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

inline void SleepFor(long ms) { vTaskDelay(ms / portTICK_PERIOD_MS); }
inline uint32_t ElapsedMilliseconds(uint64_t start, uint64_t end) { return (uint32_t)((end - start) / 1000); }
inline uint32_t KiloBytes(uint32_t bytes) { return bytes / 1024; }

#define TAG "gapper-central"

extern EventGroupHandle_t wifi_event_group;

#define CONNECTED_BIT  BIT0
#define TIME_SYNCD_BIT BIT1
#define DATA_SENT_BIT  BIT2
#define PHOTO_SENT_BIT BIT3

inline bool WaitFor(const EventBits_t mask, uint32_t timeoutMs)
{
    int bits = xEventGroupWaitBits(wifi_event_group, mask, false, true, timeoutMs / portTICK_PERIOD_MS);
    return ((bits & mask) == mask);
}

inline void SetEvent(const EventBits_t mask)
{
    xEventGroupSetBits(wifi_event_group, mask);
}

#define BEGIN_WAIT_SEQUENCE do {
#define WAIT_AND_BAIL(mask, timeoutMs, message) if (!WaitFor(mask, timeoutMs)) { ESP_LOGI(TAG, message); break; }
#define END_WAIT_SEQUENCE } while(false);

extern CURL* s_curl;

#ifdef __cplusplus
}
#endif

#endif