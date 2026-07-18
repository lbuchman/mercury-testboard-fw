/* copyright by Leo Buchman */

#ifndef WATCHDOG_H

#define WD_TRIGGER 1 // interrupt this time before WD_EXPIRE
#define WD_EXPIRE  9
#include <hw.h>

#define WATCHDOG_H
void watchdog();
void enableWatchdog();
void waitWithWatchdog(int wait);
void watchdogReboot();
#endif
