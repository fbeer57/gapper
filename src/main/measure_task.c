
#include "main.h"
#include "esp_log.h"
#include "measure_task.h"

void measure_task(void* context)
{
    EventBits_t bits;
    ESP_LOGI(TAG, "Measure Task");
    BEGIN_WAIT_SEQUENCE
        WAIT_AND_BAIL(TIME_SYNCD_BIT, CONFIG_MAX_CONNECT_WAIT, "Measure: Timed out waiting for connection")
        SleepFor(2000);
        SetEvent(DATA_SENT_BIT);
    END_WAIT_SEQUENCE
    ESP_LOGI(TAG, "Destroy Measure Task");
    vTaskDelete(NULL);
}

