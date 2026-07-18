
#ifndef BOARD_H
#define BOARD_H
#include "HardwareSerial.h"

struct rdIO {
    int pinN;
    int pinMode;
    int defValue;
    int value;

};

struct readerPins {
    rdIO d0;
    rdIO d1;
    rdIO rLed;
    rdIO gLed;
    rdIO bz;
    rdIO de;
    HardwareSerialIMXRT& serial;
};

void boardInit();

#endif
