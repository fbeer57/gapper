/*
** This code is in the Public Domain.
**
** Unless required by applicable law or agreed to in writing, this
** software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied.
**
*/
#ifndef BOARD_H
#define BOARD_H 1

#include "driver/adc.h"
#include "driver/gpio.h"

// this is the input connected to the R1/R2 13:1 (1.1V) voltage divider for the 12V battery voltage
#define BATTERY_IN GPIO_NUM_36
#define BATTERY_IN_CH ADC1_GPIO36_CHANNEL
#define BATTERY_IN_MULTIPLIER 14

// (unused) input pins
#define IN_1 BATTERY_IN
#define IN_2 GPIO_NUM_39
#define IN_3 GPIO_NUM_34
#define IN_4 GPIO_NUM_35

// this is the GPIO output that drives the 12V/2A MOSFET
#define SWITCH_VSS GPIO_NUM_32

// these are the GPIO outputs that drive the Valve (0.5A) MOSFETs
#define VALVE_1 GPIO_NUM_33
#define VALVE_2 GPIO_NUM_25
#define VALVE_3 GPIO_NUM_26
#define VALVE_4 GPIO_NUM_27

// pins used for I2C
#define I2C_2_SCL GPIO_NUM_22
#define I2C_3_SDA GPIO_NUM_21

// pins used for SPI
#define SPI_1_MISO GPIO_NUM_19
#define SPI_2_MOSI GPIO_NUM_18
#define SPI_3_SCK GPIO_NUM_5
#define SPI_4_CS GPIO_NUM_16

// pins used for JTAG
#define JTAG_1 GPIO_NUM_14
#define JTAG_2 GPIO_NUM_12
#define JTAG_3 GPIO_NUM_13
#define JTAG_4 GPIO_NUM_15

// unused GPIOs 
#define J6_1 GPIO_NUM_23
#define J6_2 GPIO_NUM_17
#define J6_3 GPIO_NUM_4
#define J6_4 GPIO_NUM_2

#define BATTERY_MULTISAMPLING_LOG2 2

#ifdef __cplusplus
extern "C" {
#endif

extern void setup_board();
extern uint32_t get_battery_millivolts();
extern void switch_valve(unsigned num, unsigned level);
extern void switch_router(unsigned level);

#ifdef __cplusplus
}
#endif

#endif
