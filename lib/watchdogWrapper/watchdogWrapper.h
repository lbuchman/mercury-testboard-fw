#ifndef WATCHDOG_WRAPPER_H
#define WATCHDOG_WRAPPER_H

#define WD_TRIGGER (WD_EXPIRE >> 1)

void watchdog();
void enableWatchdog();
void watchdogReboot();

#endif
