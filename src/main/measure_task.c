/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include "main.h"
#include "board.h"
#include "measure_task.h"
#include "upload.h"
#include "ble_scan.h"

#include <esp_log.h>
#include <sys/queue.h>

static gapper_fragment_head_t gapper_fragments = STAILQ_HEAD_INITIALIZER(gapper_fragments);

static void add_fragment(const char* format, ...)
{
    va_list valist;
    va_start(valist, format);

    gapper_fragment_t* new_node = malloc(sizeof(gapper_fragment_t)); 
    new_node->length = vasprintf(&new_node->payload, format, valist);
    STAILQ_INSERT_TAIL(&gapper_fragments, new_node, nodes);
}

static void add_scan_result(esp_bd_addr_t bda, const beacon_info_t* sr)
{
    add_fragment("\"%02x%02x%02x%02x%02x%02x\": {", bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
    add_fragment("\"DieTemperature\": {\"Min\":%d, \"Avg\":%d, \"Max\":%d},", sr->info.die_temperature_min,
                                                                                 sr->info.die_temperature_avg,
                                                                                 sr->info.die_temperature_max);

    add_fragment("\"Temperature\": {\"Min\":%d, \"Avg\":%d, \"Max\":%d},", sr->info.temperature_min,
                                                                              sr->info.temperature_avg,
                                                                              sr->info.temperature_max);
    add_fragment("\"Humidity\": {\"Min\":%d, \"Avg\":%d, \"Max\":%d},", sr->info.humidity_min,
                                                                           sr->info.humidity_avg,
                                                                           sr->info.humidity_max);
    add_fragment("\"Moisture\": {\"Min\":%d, \"Avg\":%d, \"Max\":%d},", sr->info.moisture_min,
                                                                           sr->info.moisture_avg,
                                                                           sr->info.moisture_max);
    add_fragment("\"Solar\": {\"Min\":%d, \"Avg\":%d, \"Max\":%d},", sr->info.solar_lsb_min + (sr->info.solar_msb_min << 8),
                                                                        sr->info.solar_lsb_avg + (sr->info.solar_msb_avg << 8),
                                                                        sr->info.solar_lsb_max + (sr->info.solar_msb_max << 8));
    add_fragment("\"VccAvg\":%d,", sr->info.vcc_lsb_avg + (sr->info.vcc_msb_avg << 8));
    add_fragment("\"Slugs\":%d}", sr->info.slugs_cnt);
}

void measure_task(void* context)
{
    STAILQ_INIT(&gapper_fragments);
    BEGIN_WAIT_SEQUENCE
        WAIT_AND_BAIL(TIME_SYNCD_BIT, (CONFIG_ROUTER_WAIT + CONFIG_MAX_CONNECT_WAIT), "Measure: Timed out waiting for connection")

        add_fragment("{\"BatteryMillivolts\":%u,\"BLEs\":{", get_battery_millivolts());

        gapper_scan_result_t* node = NULL;
        gapper_scan_result_t* next_node = NULL;
        SLIST_FOREACH_SAFE(node, &gapper_scan_results, nodes, next_node) {
            add_scan_result(node->bda, &node->info);
            if (next_node) {
                add_fragment(",");
            }

            free(node);
        }
        SLIST_INIT(&gapper_scan_results);

        add_fragment("}}");
        upload_file_scattered("central", &gapper_fragments, "json");

        SetEvent(DATA_SENT_BIT);
    END_WAIT_SEQUENCE

    // clean up payload buffers
    gapper_fragment_t *node;
    gapper_fragment_t *next_node;
    STAILQ_FOREACH_SAFE(node, &gapper_fragments, nodes, next_node){
        free(node);
    }
    STAILQ_INIT(&gapper_fragments);

    // done
    vTaskDelete(NULL);
}

