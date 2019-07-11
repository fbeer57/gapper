/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/
#ifndef BLE_SCAN_H
#define BLE_SCAN_H 1

#include <esp_bt_defs.h>
#include <sys/queue.h>

typedef struct {
    uint8_t     device_type;        //  1
    uint8_t     data_length;        //  1

    uint8_t     die_temperature_min;//  1
    uint8_t     die_temperature_max;//  1
    uint8_t     die_temperature_avg;//  1

    uint8_t     temperature_min;    //  1
    uint8_t     temperature_max;    //  1
    uint8_t     temperature_avg;    //  1

    uint8_t     humidity_min;       //  1
    uint8_t     humidity_max;       //  1
    uint8_t     humidity_avg;       //  1

    uint8_t     moisture_min;       //  1
    uint8_t     moisture_max;       //  1
    uint8_t     moisture_avg;       //  1

    uint8_t     vcc_msb_avg;        //  1
    uint8_t     vcc_lsb_avg;        //  1

    uint8_t     solar_msb_min;      //  1
    uint8_t     solar_lsb_min;      //  1
    uint8_t     solar_msb_max;      //  1
    uint8_t     solar_lsb_max;      //  1
    uint8_t     solar_msb_avg;      //  1
    uint8_t     solar_lsb_avg;      //  1

    uint8_t     slugs_cnt;          //  1
} __attribute__ ((packed)) _beacon_info_t;

typedef union {
    _beacon_info_t info;
    uint8_t info_raw[sizeof(_beacon_info_t)];
} beacon_info_t;

typedef struct gapper_scan_result {
    SLIST_ENTRY(gapper_scan_result) nodes;
    esp_bd_addr_t bda;
    beacon_info_t info;
} gapper_scan_result_t;

typedef SLIST_HEAD(head_s, gapper_scan_result) head_t;

extern head_t gapper_scan_results;

void ble_start();
void ble_stop();

#endif
