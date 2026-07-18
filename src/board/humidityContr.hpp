#ifndef HUMIDITY_CTL__H
#define HUMIDITY_CTL__H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <logger.hpp>
#include <TaskSchedulerDeclarations.h>
#include <persistent.hpp>
#include <networkedSensor.hpp>
// #include <stopwatch.hpp>

class HumidityContr {
    public:

        HumidityContr() = delete;
        HumidityContr(Scheduler& _ts, TDevice& _deviceData): ts(_ts), deviceData(_deviceData)  {};

        void begin() {
            deviceTask.enable();
            sensor.begin();
            sensor.onChange([this](void* _data) -> void {
                SensorState* data = (SensorState*) _data;
                humidity = data->humidity;
                temp = data->temp;
                sensorErrorCount = 0;
                sensorOk = true;
                // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "sensor %s %s count = %d, humidity = %3.1f%%, temp = %3.1fC", deviceData.name,  ipToString(deviceData.sensorIp).c_str(), data->count, data->humidity, data->temp);
            });

            sensor.onError([this](void* data) -> void {
                if(sensorErrorPrintCounter > 30) {
                    logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "sensor %s %s read error", deviceData.name, ipToString(deviceData.sensorIp).c_str());
                    sensorErrorPrintCounter = 0;
                }
                sensorErrorPrintCounter += 1;
                sensorErrorCount += 1;
            });

            deviceTask.enable();
            eepromSaveTask.enable();
        };

        String getDevName() {
            return deviceData.name;
        }

        String isOn() {
            return isDeviceOn;
        }

        float getKwattUsed() {
            uint32_t powerWattHour;
            deviceData.totalRunTimeSec = actualOnTime / 1000;
            powerWattHour = (float)(deviceData.devicePowerWatt) * (actualOnTime / 3600000.0);
            // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "sensor %s devicePowerWatt = %d  runTimeCount = %d", deviceData.name, deviceData.devicePowerWatt, runTimeCount, deviceData.totalRunTimeSec);
            return powerWattHour / 1000.0;
        }

        void getHumidityDeviceData(Stream & stream, int device) {
            JsonDocument  doc;
            JsonObject  retData = doc.to<JsonObject>();
            retData["cmd"] = "gethumiddata";
            retData["sensorOk"] = sensorOk;
            retData["sensorIp"] = ipToString(deviceData.sensorIp);
            retData["minHumidity"] = deviceData.minValue;
            retData["maxHumidity"] = deviceData.maxValue;
            retData["fuseTimeToBlow"] = deviceData.maxRunTimeSec;
            retData["fuseTimeToReset"] = deviceData.minRunTimeSec;
            retData["power"] = deviceData.devicePowerWatt;
            retData["name"] = deviceData.name;
            retData["supervisedInput"] = 0;
            retData["tag"] = deviceData.tag;
            retData["sensorPort"] = deviceData.sensorPort;
            retData["loggerIp"] = "1234";
            retData["loggingInterval"] = 10000;
            serializeJsonPretty(retData, stream);
        }

        bool getSensorStatus() {
            return sensorOk;
        }

        bool getInTempRange() {
            return true;
        }
        bool manualOverride() {
            return false;
        }
        float getTemp() {
            return temp;
        }
        float getHumidity() {
            return humidity;
        }

        TDevice& getDeviceData() {
            return deviceData;
        }

        void clearPowerUsed() {
            actualOnTime = 0;
        }

        String getLastPowerResetDateTime() {
            time_t refTime = deviceData.totalRunTimeResetEpoch;
            return epochToString(refTime);
        }

        String getRunTimeCount() {
            return String(runTimeCount / 1000);
        }

    private:
        Scheduler& ts;
        TDevice& deviceData;
        float humidity = 0;
        float temp = 0;
        static constexpr uint32_t maxSensorErrorCount = 10;
        uint32_t relaxTimeConst = (deviceData.minRunTimeSec * TASK_SECOND) / (SENSORS_PULL_INTERVAL / 3);
        uint32_t relaxTime = 0; // after off this time wait before next on. This is to protect dehumidifier compressor
        uint32_t runTimeCount = 0;
        bool isDeviceOn = false;
        uint32_t sensorErrorCount = 0;
        int64_t actualOnTime = deviceData.totalRunTimeSec * 1000; /* mSec */
        uint32_t timeCapture = 0;
        bool    sensorOk = false;
        uint32_t prevOnTime = deviceData.minRunTimeSec * -1;
        int sensorErrorPrintCounter = 0;

        NetSensor sensor{ ts, deviceData.sensorIp, deviceData.sensorPort };

        void deviceOnOff(bool value) {
            // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "Termostat  < %s > deviceOnOff(%d) errorCount = %d OnTime = %lld, Relax Count = %d runTimeCount = %d", deviceData.name, value, errorCount, actualOnTime / 1000, relaxTime, runTimeCount);

            if(value == false && isDeviceOn == true && runTimeCount >= relaxTimeConst) {
                relaxTime = relaxTimeConst;
                isDeviceOn = false;
                analogWrite(deviceData.analogOutputPort, 0);
                logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "sensor < %s > is OFF", deviceData.name);
                return;
            }

            if(value == false) {
                return;
            }

            if((relaxTime && (value == true)) || isDeviceOn) {
                return;
            }

            timeCapture = millis();

            if(((millis() - prevOnTime) < deviceData.minRunTimeSec) && isDeviceOn) {
                logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "Termostat  < %s > Sensor state change to On in less then 120 Sec", deviceData.name);
                return;
            }

            isDeviceOn = true;
            prevOnTime = millis();
            analogWrite(deviceData.analogOutputPort, 255);
            logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "Termostat  < %s > is On", deviceData.name);
        }

        Task deviceTask{SENSORS_PULL_INTERVAL / 3, TASK_FOREVER, [this](void) -> void {
                relaxTime -= 1;

                if(relaxTime < 0) {
                    relaxTime = 0;
                }

                if(isDeviceOn) {
                    runTimeCount += 1;
                    actualOnTime += millis() - timeCapture;
                    timeCapture = millis();
                }

                if(sensorErrorCount >= maxSensorErrorCount) {
                    // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "Termostat  < %s > Sensor is Off, Emergency shut down the device", deviceData.name);
                    deviceOnOff(false);
                    sensorErrorCount = maxSensorErrorCount;
                    sensorOk = false;
                    return;
                }

                if(runTimeCount > ((deviceData.maxRunTimeSec * TASK_SECOND) / (SENSORS_PULL_INTERVAL / 3))) {
                    runTimeCount = 0;
                    deviceOnOff(false);
                }

                if(isDeviceOn) {
                    if(humidity <= deviceData.minValue) {
                        deviceOnOff(false);
                    }
                }
                else {
                    if(humidity >= deviceData.maxValue) {
                        // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "Termostat  < %s > humidity = %f, max = %d", deviceData.name, humidity, deviceData.maxValue);
                        deviceOnOff(true);
                        runTimeCount = 0;
                    }
                }
            }, &ts, false };

        Task eepromSaveTask{TASK_HOUR / 4, TASK_FOREVER, [this](void) -> void {
                deviceData.totalRunTimeSec = actualOnTime / 1000;
                logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "Updating device = %d total power", deviceData.tag);

                devicedata2eeprom(deviceData, deviceData.tag);
            }, &ts, false };
};

#endif
