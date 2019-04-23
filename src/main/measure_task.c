/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include "main.h"
#include "esp_log.h"
#include "board.h"
#include "measure_task.h"
#include "upload.h"

static uint8_t results[48];

void measure_task(void* context)
{
    ESP_LOGI(TAG, "Measure Task");
    int chars_written = snprintf((char*)results, sizeof(results), "{\n\t\"BatteryMilliVolts\": %u\n}\n", get_battery_millivolts());
    BEGIN_WAIT_SEQUENCE
        WAIT_AND_BAIL(TIME_SYNCD_BIT, (CONFIG_ROUTER_WAIT + CONFIG_MAX_CONNECT_WAIT), "Measure: Timed out waiting for connection")

        upload_file("central", results, chars_written, "json");

        SetEvent(DATA_SENT_BIT);
    END_WAIT_SEQUENCE
    ESP_LOGI(TAG, "Destroy Measure Task");
    vTaskDelete(NULL);
}

