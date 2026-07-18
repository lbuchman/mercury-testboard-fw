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
#include <ArduinoJson.h>
#include <etherUtilities.h>
#include <logger.hpp>


#define EEPROM_REGION_SIZE 256

typedef struct TNetworkConfig_ {
    uint32_t  randomNumber;
    uint8_t   mac[6] = { 0, 0, 0, 0, 0, 0 };
    uint8_t   ntpServer[4] = { 91, 189, 91, 157 };
    int32_t   tag;
    uint8_t   padding[3];
    uint8_t   crc;
} TNetworkConfig;

typedef struct  TDevice_ { // = 64 bytes,  structure for EEPROM not packed to avoid performance hit, therefore padding is required for all data
    uint32_t          randomNumber;
    int32_t           tag;
    uint32_t          sensorIp;
    uint32_t          sensorPort;
    time_t            totalRunTimeResetEpoch;
    uint32_t          minValue;
    uint32_t          maxValue;
    uint32_t          maxRunTimeSec;
    uint32_t          minRunTimeSec;
    uint32_t          devicePowerWatt;
    uint32_t          analogOutputPort;
    uint32_t          totalRunTimeSec;
    char              name[32];
    uint8_t           padding[3];
    uint8_t           crc;
} TDevice;


void persistentDataInit();
TNetworkConfig getNetworkConfig();
void setNetworkConfig(TNetworkConfig config);
TDevice getDeviceConfig(int32_t tag);
TDevice& getDeviceData(int32_t tag);
TDevice getDeviceConfigEEPROM(int32_t tag);
void printDeviceConfig(TDevice config, int32_t tag);
uint32_t getPowerValue(uint32_t tag);
void setPowerValue(uint32_t value, int32_t tag);
void devicedata2eeprom(TDevice data, int32_t tag);
void network2eeprom(TNetworkConfig data);
#endif
