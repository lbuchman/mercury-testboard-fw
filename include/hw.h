#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#define SENSORS_PULL_INTERVAL (TASK_SECOND * 3)

#define WATCHDOG_LED 13
#define MAX_TERMINAL_STRING_OUT_PACKET_SIZE 128

#define FULL_CLOCK 600000000
#define IDLE_CLOCK 200000000

#define FACTORY_RESET_PIN 16
#define AMALOG_MUX_PIN_0 22
#define AMALOG_MUX_PIN_1 41
#define AMALOG_MUX_EN 31
#define ANALOG_PIN_0 A3
#define ANALOG_PIN_1 A2

#define AOutput1 14
#define AOutput2 18
#define AOutput3 19
#define AOutput4 15
#define AOutput5 25
#define AOutput6 36
#define AOutput7 2
#define AOutput8 24

#define laundryTag 0
#define crawl2Tag 1
#define crawl1Tag 2
#define basementTag 3
#define netSettingsTag 4

#endif
