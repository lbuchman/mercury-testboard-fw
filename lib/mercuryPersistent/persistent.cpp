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

#include <FastCRC.h>
#include <hw.h>
#include <logger.hpp>
#include <persistent.hpp>

TNetworkConfig getNetworkDataFromEpprom();

static TNetworkConfig netConfig;

TNetworkConfig getNetworkConfig() { return netConfig; };

bool persistentDataInit() {
    netConfig = getNetworkDataFromEpprom();
    return netConfig.tag == netSettingsTag;
}

bool checkCRC8(uint8_t* buffer, int size, uint8_t expectedCRC8) {
    FastCRC8 fastcrc;
    uint32_t crc8 = fastcrc.smbus(buffer, size);

    if (crc8 != expectedCRC8) {
        logger().error(logger().printHeader, (char*)__FILE__, __LINE__, "Invalid CRC8, expected = 0x%x,actual 0x%x, datasize = %d",
                       expectedCRC8, crc8, size);
        return false;
    }

    return true;
}

TNetworkConfig getNetworkDataFromEpprom() {
    TNetworkConfig data;
    EEPROM.get(EEPROM_REGION_SIZE * netSettingsTag, data);
    logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Read EEPROM at address 0x%x", EEPROM_REGION_SIZE * 4);

    if (!checkCRC8((uint8_t*)&data, sizeof(data) - 1, data.crc)) {
        logger().error(logger().printHeader, (char*)__FILE__, __LINE__, "Invalid EEPROM CRC8 for network data");
        data.tag = -1; // invalid
    }

    return data;
};

bool network2eeprom(TNetworkConfig data) {
    FastCRC8 fastcrc;

    data.tag = netSettingsTag;
    uint32_t crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);

    if (crc8 == 0) {
        data.randomNumber = random(0xFFFFFFFF);
        crc8 = fastcrc.smbus((uint8_t*)&data, sizeof(data) - 1);
    }

    data.crc = crc8;
    logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Write EEPROM at address 0x%x", EEPROM_REGION_SIZE * 4);
    EEPROM.put(EEPROM_REGION_SIZE * netSettingsTag, data);
    TNetworkConfig verified = getNetworkDataFromEpprom();

    if (memcmp(&verified, &data, sizeof(data)) != 0) {
        logger().error(logger().printHeader, (char*)__FILE__, __LINE__, "Network configuration EEPROM readback failed");
        return false;
    }

    netConfig = data;
    return true;
};
