#ifdef PlatformTeensy

#include "watchdogWrapper.h"
#include "Arduino.h"
#include <TaskSchedulerDeclarations.h>
#include <Watchdog_t4.h>
#include <logger.hpp>

WDT_T4<WDT1> wdt;

void myCallback() { Serial.println("!!!!!!!!!!!!!!!!!!!! watchdog is about to reboot !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"); }

void enableWatchdog() {
#ifndef DISABLE_WD
    WDT_timings_t config;
    config.trigger = WD_TRIGGER; /* in seconds, 0->128 , isr will be called after WD_EXPIRE - WD_TRIGGER*/
    config.timeout = WD_EXPIRE;  /* in seconds, 0->128 */
    config.callback = myCallback;
    wdt.begin(config);
#endif

#ifdef PlatformSTstm32
#include <IWatchdog.h>
#endif
}

void watchdogReboot() {
#ifndef DISABLE_WD
    wdt.reset();
#endif
}

void watchdog() {
#ifndef DISABLE_WD
    wdt.feed();
#endif
}

#endif
