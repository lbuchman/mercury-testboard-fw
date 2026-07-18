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
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHERtherpy
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <persistent.hpp>
#include <CRCx.h>
#include <logger.hpp>
#include <CRCx.h>
#include <shellFunctor.hpp>
#include <utility.h>

#define BUILTIN_SDCARD 254
const uint8_t SD_CS_PIN = BUILTIN_SDCARD;
#define SPI_CLOCK SD_SCK_MHZ(50)

TNetworkConfig getNetworkDataFromEpprom();

static TNetworkConfig netConfig;
static TDevice deviceData[4] = {
    { 1234, 0, 0x1102A8C0, 10000, 0, 50, 55, 3600, 180, 700, AOutput5, 0, "laundry" },
    { 1234, 1, 0x2202A8C0, 10000, 0, 50, 55, 3600, 180, 375, AOutput6, 0, "crawl2" },
    { 1234, 2, 0x4702A8C0, 10000, 0, 50, 55, 3600, 180, 375, AOutput7, 0, "crawl1" },
    { 1234, 3, 0x1302A8C0, 10000, 0, 50, 55, 3600, 180, 550, AOutput8, 0, "basement" }
};

static String epochToString(time_t t) {
    const char *str = ctime(&t);
    return str;
}

void printDeviceConfig(TDevice &data, char* cmd, Stream & stream) {
    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = cmd;
    retData["tag"] = data.tag;
    IPAddress ip = data.sensorIp;
    retData["sensorIp"] = ipToString(ip);
    retData["sensorPort"] = data.sensorPort;
    retData["minValue"] = data.minValue;
    retData["maxValue"] = data.maxValue;
    retData["maxRunTimeSec"] = data.maxRunTimeSec;
    retData["minRunTimeSec"] = data.minRunTimeSec;
    retData["devicePowerWatt"] = data.devicePowerWatt;
    retData["analogOutputPort"] = data.analogOutputPort;
    retData["totalRunTimeSec"] = data.totalRunTimeSec;
    retData["name"] = data.name;
    retData["status"] = true;
    retData["lastPowerResetDateTime"] = epochToString(data.totalRunTimeResetEpoch);
    serializeJsonPretty(retData, stream);
}

shellFunc printdevdata = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", \"desc\": \"read device data\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    TDevice data = getDeviceConfig(tag);
    printDeviceConfig(data, args[0], stream);
    return 1;
};



shellFunc printdevdataeeprom = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", \"desc\": \"read device data\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    TDevice data = getDeviceConfigEEPROM(tag);
    printDeviceConfig(data, args[0], stream);
    return 1;
};

shellFunc savedevdata = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", \"desc\": \"read device data\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    TDevice data = deviceData[tag];
    data.tag = tag;
    devicedata2eeprom(data, tag);
    data = getDeviceConfigEEPROM(tag);

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];

    if(data.tag > -1) {
        retData["status"] = true;
        deviceData[tag] = data;
    }
    else {
        retData["status"] = false;
    }

    serializeJsonPretty(retData, stream);
    return 1;
};

shellFunc updatethermsensor = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(4, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", sensorIp port \"desc\": \"update thermostat sensor ip and port\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    IPAddress sensorIp = str2Ip(args[2]);
    int port = atoi(args[3]);
    TDevice& data = deviceData[tag];
    data.sensorIp = sensorIp;
    data.sensorPort = port;
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];
    retData["status"] = true;
    serializeJsonPretty(retData, stream);

    return 1;
};

shellFunc updatethermminmax = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(4, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", min max\"desc\": \"update termostat min max limits\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    int min = atoi(args[2]);
    int max = atoi(args[3]);
    TDevice& data = deviceData[tag];
    data.minValue = min;
    data.maxValue = max;
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];
    retData["status"] = true;
    serializeJsonPretty(retData, stream);

    return 1;
};


shellFunc updatethermtiming = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(4, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", minRunTimeSec mxnRunTimeSec\"desc\": \"update termostat minRunTimeSec mxnRunTimeSec \" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    TDevice& data = deviceData[tag];
    data.minRunTimeSec = atoi(args[2]);
    data.maxRunTimeSec = atoi(args[3]);
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];
    retData["status"] = true;
    serializeJsonPretty(retData, stream);

    return 1;
};

shellFunc updatethermdevicepower = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(3, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", device power\"desc\": \"update termostat device i.e. dehumidifier pwer \" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);

    if(tag < 0 || tag > 3) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }

    TDevice& data = deviceData[tag];
    data.devicePowerWatt = atoi(args[2]);
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];
    retData["status"] = true;
    serializeJsonPretty(retData, stream);

    return 1;
};

shellFunc updatethermname = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(3, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"device [0-3]\", sensorIp port\"desc\": \"update termostat name\" },\n\r", stream)) {
        return 1;
    }

    int tag = atoi(args[1]);
    TDevice& data = deviceData[tag];

    if(tag < 0 || tag > 3 || strlen(args[2]) >= sizeof(data.name)) {
        JsonDocument doc;
        JsonObject  retData = doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = false;
        retData["error"] = "invalid argument";
        serializeJsonPretty(retData, stream);
        return 1;
    }


    strcpy(data.name, args[2]);
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;

    JsonDocument doc;
    JsonObject  retData = doc.to<JsonObject>();
    retData["cmd"] = args[0];
    retData["status"] = true;
    serializeJsonPretty(retData, stream);

    return 1;
};

TNetworkConfig getNetworkConfig() {
    return netConfig;
};

void persistentDataInit() {
    TDevice data;
    netConfig = getNetworkDataFromEpprom();

    data = getDeviceConfigEEPROM(0);

    if(data.tag > -1) {
        deviceData[0] = data;
    }

    data = getDeviceConfigEEPROM(1);

    if(data.tag > -1) {
        deviceData[1] = data;
    }

    data = getDeviceConfigEEPROM(2);

    if(data.tag > -1) {
        deviceData[2] = data;
    }

    data = getDeviceConfigEEPROM(3);

    if(data.tag > -1) {
        deviceData[3] = data;
    }

}

bool checkCRC8(uint8_t* buffer, int size,  uint8_t expectedCRC8) {
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus(buffer, size);

    if(crc8 != expectedCRC8) {
        logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Invalid CRC8, expected = 0x%x,actual 0x%x, datasize = %d", expectedCRC8, crc8, size);
        return false;
    }

    return true;
}

void setNetworkConfig(TNetworkConfig config) {};

TNetworkConfig getNetworkDataFromEpprom() {
    TNetworkConfig data;
    EEPROM.get(EEPROM_REGION_SIZE * netSettingsTag, data);
    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Read EEPROM at address 0x%x", EEPROM_REGION_SIZE * 4);

    if(!checkCRC8((uint8_t*)&data, sizeof(data) - 1, data.crc)) {
        logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Invalid EEPROM CRC8 for network data");
        data.tag = -1; // invalid
    }

    return data;
};

void network2eeprom(TNetworkConfig data) {
    FastCRC8 fastcrc;

    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;
    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Write EEPROM at address 0x%x", EEPROM_REGION_SIZE * 4);
    EEPROM.put(EEPROM_REGION_SIZE * netSettingsTag, data);
    netConfig = data;
    getNetworkDataFromEpprom();

};


TDevice getDeviceConfigEEPROM(int32_t tag) {
    TDevice data;
    EEPROM.get(EEPROM_REGION_SIZE * tag, data);
    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Read EEPROM at address 0x%x", EEPROM_REGION_SIZE * tag);

    if(!checkCRC8((uint8_t*)&data, sizeof(data) - 1, data.crc)) {
        // logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Invalid EEPROM CRC8, tag = %d", tag);
        data.tag = -1; // invalid
    }

    return data;
};


TDevice getDeviceConfig(int32_t tag) {
    TDevice data = deviceData[tag];

    if(!checkCRC8((uint8_t*)&data, sizeof(data) - 1, data.crc)) {
        logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Invalid Data CRC8, tag = %d", tag);
        data.tag = -1; // invalid
    }

    return data;
};


TDevice& getDeviceData(int32_t tag) {
    if(!checkCRC8((uint8_t*)&deviceData[tag], sizeof(deviceData[tag]) - 1, deviceData[tag].crc)) {
        logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Invalid Data CRC8, tag = %d", tag);
        deviceData[tag].tag = -1; // invalid
    }

    return deviceData[tag];
};

void devicedata2eeprom(TDevice data, int32_t tag) {
    if(tag < 0) {
        return;
    }

    data.tag = tag;
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if(crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;
    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Write EEPROM at address 0x%x, tag = %d, crc = 0x%x", EEPROM_REGION_SIZE * tag, tag, data.crc);
    EEPROM.put(EEPROM_REGION_SIZE * tag, data);
};

uint32_t getPowerValue(int32_t tag) {
    return deviceData[tag].totalRunTimeSec;
};

void setPowerValue(uint32_t value, int32_t tag) {
    TDevice& data = deviceData[tag];
    data.totalRunTimeSec = value;
};
