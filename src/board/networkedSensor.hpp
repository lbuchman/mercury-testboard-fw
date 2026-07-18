#ifndef NETWORKED_SENSOR__H
#define NETWORKED_SENSOR__H

#include <Arduino.h>
#include <memory>
#include <utility>
#include <ArduinoJson.h>
#include <logger.hpp>
#include <NativeEthernet.h>
#include <TaskSchedulerDeclarations.h>
#include <datatypes.h>

typedef std::function<void(void* data)> sensorStateChangeCallback;

struct SensorState {
    float temp;
    float humidity;
    int count;
};

class NetSensor: public SimpleEvent {
    public:
        const  int kLineSonsorStateChange = 0;
        const  int kLineSensorError = 1;

        NetSensor() = delete;
        NetSensor(Scheduler& _ts, IPAddress _ip, uint32_t _port): ts(_ts), ip(_ip), port(_port) {};

        void begin() {
            logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "initializing sensor %s", ipToString(ip).c_str());
            uint16_t _port = 10000 + (rand() % 50000);
            Udp.begin(_port);
            sensorPullTask.enable();
        };
        void onChange(sensorStateChangeCallback eventFunction) {
            registerEvent(kLineSonsorStateChange, eventFunction);
        }
        void onError(sensorStateChangeCallback eventFunction) {
            registerEvent(kLineSensorError, eventFunction);
        }

    private:
        Scheduler& ts;
        IPAddress ip;
        uint16_t port;
        EthernetUDP Udp;
        SensorState sensorState = {0, 0, 0};
        static constexpr int bufferSizeIn = 256;
        static constexpr int maxReadCount = 9;

        Task sensorPullTask{SENSORS_PULL_INTERVAL, TASK_FOREVER, [this](void) -> void {
                // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "requesting sensor %s:%d", ipToString(ip).c_str(), port);

                if(!Udp.beginPacket(ip, port)) {
                    return;
                }

                JsonDocument  doc;
                JsonObject  jsonDocument = doc.to<JsonObject>();
                jsonDocument["aria"] = ipToString(ip);
                jsonDocument["tag"] = 0;
                jsonDocument["isOn"] = true;

                serializeJsonPretty(jsonDocument, Udp);
                Udp.endPacket();
                sensorReadTask.restartDelayed(TASK_MILLISECOND * 100);
            }, &ts, false, NULL, NULL
        };

        Task sensorReadTask{TASK_MILLISECOND * 100, maxReadCount, [this](void) -> void {
                int packetSize = Udp.parsePacket();

                if(packetSize > bufferSizeIn - 1) {
                    logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "udp data exceed buffer size = %d", packetSize);
                    return;
                }

                if(packetSize) {
                    char buffer[bufferSizeIn];
                    Udp.read(buffer, packetSize);
                    buffer[packetSize] = 0;
                    // logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "sensor %s data -> %s", ipToString(ip).c_str(), buffer);
                    JsonDocument doc;
                    JsonObject  jsonDocument = doc.to<JsonObject>();
                    DeserializationError error = error.Code::EmptyInput;
                    error = deserializeJson(jsonDocument, buffer);

                    if(error != error.Code::Ok) {
                        logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "{ \"error\": %s, \"status\": false,  }", error.c_str());
                        fireEvent(kLineSensorError, &sensorState);
                        return;
                    }

                    sensorState.temp = jsonDocument["temp"];
                    sensorState.humidity = jsonDocument["Humidity"];
                    sensorState.count = jsonDocument["count"];
                    fireEvent(kLineSonsorStateChange, &sensorState);

                    sensorReadTask.disable();
                }
                else {
                    if(sensorReadTask.getIterations() == 0) {
                        // logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "sensor %s read error", ipToString(ip).c_str());
                        fireEvent(kLineSensorError, &sensorState);
                    }
                }
            }, &ts, false
        };

};

#endif
