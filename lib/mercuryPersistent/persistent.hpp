/*
MIT License


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef PERSISTENT_HH
#define PERSISTENT_HH

#include <EEPROM.h>
#include <etherUtilities.h>
#include <hw.h>
#include <logger.hpp>

#define EEPROM_REGION_SIZE 256

typedef struct TNetworkConfig_ {
    uint32_t randomNumber;
    uint8_t mac[6] = {0, 0, 0, 0, 0, 0};
    uint8_t ntpServer[4] = {91, 189, 91, 157};
    int32_t tag;
    uint8_t padding[3];
    uint8_t crc;
} TNetworkConfig;

bool persistentDataInit();
TNetworkConfig getNetworkConfig();
bool network2eeprom(TNetworkConfig data);
#endif
