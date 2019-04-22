/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/

#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "main.h"
#include "board.h"

#include "esp_adc_cal.h"
#include "esp_log.h"

#define DEFAULT_VREF 1200

static esp_adc_cal_characteristics_t ADC1_CHARACTERISTICS;

static gpio_num_t VALVE2GPIO[] = { VALVE_1, VALVE_2, VALVE_3, VALVE_4 };
static gpio_num_t MOSFETS[] = { SWITCH_VSS, VALVE_1, VALVE_2, VALVE_3, VALVE_4 };

static void setup_mosfet_switch(gpio_num_t gpio_num) {
    // uses external 1M pull-down
    gpio_set_pull_mode(gpio_num, GPIO_FLOATING);
    gpio_set_direction(gpio_num, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_num, 0);
}

void setup_board() {

    for(int i = 0; i < sizeof(MOSFETS) / sizeof(gpio_num_t); ++i) {
        setup_mosfet_switch(MOSFETS[i]);
    }

    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_12, DEFAULT_VREF, &ADC1_CHARACTERISTICS);

    char* val_type_str = "";
    switch(val_type) {
        case ESP_ADC_CAL_VAL_DEFAULT_VREF:  val_type_str = "DEFAULT_VREF";  break;
        case ESP_ADC_CAL_VAL_EFUSE_VREF:    val_type_str = "EFUSE_VREF";  break;
        case ESP_ADC_CAL_VAL_EFUSE_TP:      val_type_str = "EFUSE_TP";  break;
        default:                            val_type_str = "UNKNOWN"; break;
    }
    ESP_LOGI(TAG, "ADC1 is characterized by %s", val_type_str);
}

uint32_t get_battery_millivolts() {

    // adc_power_on();
    // configure ADC 1, channel 0 (BATTERY Voltage)
    adc_gpio_init(ADC_UNIT_1, BATTERY_IN_CH);
    adc1_config_width(ADC1_CHARACTERISTICS.bit_width);
    adc1_config_channel_atten(BATTERY_IN_CH, ADC1_CHARACTERISTICS.atten);
    uint32_t average_reading = 0;
    // take voltage samples
    for(int i = 0, n = (1 << BATTERY_MULTISAMPLING_LOG2); i < n; ++i) {
        average_reading += adc1_get_raw(BATTERY_IN_CH);
    }
    // adc_power_off();
    average_reading >>= BATTERY_MULTISAMPLING_LOG2;
    // convert to voltage in millivolts
    uint32_t val = esp_adc_cal_raw_to_voltage(average_reading, &ADC1_CHARACTERISTICS);
    return val * BATTERY_IN_MULTIPLIER;
}

void switch_valve(unsigned num, unsigned level) {
    if (num < sizeof(VALVE2GPIO) / sizeof(gpio_num_t)) {
        gpio_set_level(VALVE2GPIO[num], level);
    }
    else {
        ESP_LOGE(TAG, "switch_valve: num is out of range");
    }
}

void switch_router(unsigned level) {
    gpio_set_level(SWITCH_VSS, level);
}
