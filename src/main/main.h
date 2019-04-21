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

#define MAX_CONNECT_WAIT (30000 / portTICK_PERIOD_MS)
#define MAX_SEND_WAIT    (20000 / portTICK_PERIOD_MS)
#define MAX_SNTP_WAIT    (10000 / portTICK_PERIOD_MS)

#ifdef __cplusplus
}
#endif

#endif