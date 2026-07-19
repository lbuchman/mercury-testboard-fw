#ifndef NETWORKING___H
#define NETWORKING___H

#include <TaskSchedulerDeclarations.h>
#include <singleLEDLibraryMod.h>

void initNetworking(Scheduler& ts, sllibMod& watchDogLed, bool networkConfigValid);

#endif
