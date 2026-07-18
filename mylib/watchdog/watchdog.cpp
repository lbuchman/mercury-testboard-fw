/* copyright by Leo Buchman */

#include "Arduino.h"
#include <watchdog.h>
#include <Watchdog_t4.h>
#include <logger.hpp>
#include <TaskSchedulerDeclarations.h>

extern Scheduler  ts;

// https://github.com/tonton81/WDT_T4.git

WDT_T4<WDT1> wdt;
void

wdCallback() {
    Task* pcurrentTask = ts.getCurrentTask();
    // char* taskname = (char*) pcurrentTask->getTaskName().c_str();
    logger().warn(false, (char*) __FILE__, __LINE__, "Watchdog will expire in %d Sec, cur task =%d", WD_TRIGGER, pcurrentTask->getId());
    Serial1.print("task ID ");
}

/*
*/
void enableWatchdog() {
#ifndef DISABLE_WD
    WDT_timings_t config;
    config.trigger = WD_TRIGGER; /* in seconds, 0->128 , isr will be called after WD_EXPIRE - WD_TRIGGER*/
    config.timeout = WD_EXPIRE; /* in seconds, 0->128 */
    config.callback = wdCallback;
    wdt.begin(config);
#endif
}
/*
*/
void watchdogReboot() {
#ifndef DISABLE_WD
    wdt.reset();
#endif
}

/*
*/
void watchdog() {
#ifndef DISABLE_WD
    wdt.feed();
#endif
}
