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

#ifndef UDPTERMINAL___H
#define UDPTERMINAL___H

#include <Arduino.h>
#include <wiring.h>
#include <memory>
#include <utility>
#include <ArduinoJson.h>
#include <logger.hpp>
#include <NativeEthernet.h>
#include <TaskSchedulerDeclarations.h>
#include <etherUtilities.h>
#include <argvp.h>
#include <simpleEvents.h>

#define OUTPUT_BUFFER_SIZE 4096*2
#define INPUT_BUFFER_SIZE 4096

class UdpStream: public Stream {
    public:
        UdpStream() {
            buffer[0] = 0;
        };
        void begin() {}
        virtual int available(void) {
            return pointer;
        };

        virtual int peek(void) {
            return 0;
        };

        virtual void flush(void) {
            pointer = 0;
            buffer[0] = 0;
        };

        int getStreamSize() {
            return pointer;
        }

        virtual size_t write(uint8_t c) {
            if(pointer < OUTPUT_BUFFER_SIZE) {
                buffer[pointer] = c;
                pointer += 1;
            }

            return 1;
        }

        virtual int read(void) {
            return 0;
        };

        char* getBuffer() {
            buffer[pointer] = 0;
            return buffer;

        }
    private:
        char buffer[OUTPUT_BUFFER_SIZE];
        int pointer = 0;
};


class UdpTerminal: public SimpleEvent {
    public:
        UdpTerminal(ShellFunctor& _shell, int _port, Scheduler& ts_): shell(_shell), port(_port), ts(ts_) {
        };

        void begin(void) {
            Udp.begin(port);
            dataInTask.enable();
        };

    private:
        ShellFunctor& shell;
        EthernetUDP Udp;
        int  port;
        JsonDocument  doc;
        JsonObject  jsonDocument = doc.to<JsonObject>();

        char buffer[1024 * 2];
        UdpStream stream;
        Scheduler& ts;
        static constexpr int dataPullInterval = TASK_MILLISECOND * 50;

        Task dataInTask{dataPullInterval, TASK_FOREVER, [this](void) -> void {
                int packetSize = Udp.parsePacket();

                if(packetSize) {
                    // powerSetFullSpeed();

                    if(packetSize < INPUT_BUFFER_SIZE - 1) {
                        Udp.read(buffer, packetSize);
                    }
                    else {
                        logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "Received packet of size %d exeeds buffer size %d", packetSize, INPUT_BUFFER_SIZE);
                        return;
                    }

                    //char tmpStr1[32];
                    //logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Received packet from %s", ipToString(Udp.remoteIP(), tmpStr1).c_str()) ;
                    buffer[packetSize] = 0;
                    DeserializationError error = deserializeJson(jsonDocument, buffer);

                    if(error != error.Code::Ok) {
                        logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "{ \"error\": %s, \"status\": false,  }", error.c_str());
                        return;
                    }

                    String cmd = jsonDocument["cmd"];
                    String arg = jsonDocument["arg"];
                    String cmdLine = cmd;

                    if(arg != "null") {
                        cmdLine = cmd + " " + arg;
                    }

                    constexpr size_t argc_MAX = 16;
                    char* argv[argc_MAX] = { 0 };
                    int argc = 0;
                    parseStrToArgcArgvInsitu((char*) cmdLine.c_str(), argc_MAX, &argc, argv);
                    stream.flush();
                    logger().debug(logger().printHeader, (char*) __FILE__, __LINE__,  "udp get cmd: %s", argv[0]);
                    shell(argv[0], argc, argv, stream);


                    if(!Udp.beginPacket(Udp.remoteIP(), Udp.remotePort())) {
                        return;
                    }

                    if(!stream.getStreamSize()) {
                        JsonDocument doc;
                        JsonObject  retData =  doc.to<JsonObject>();
                        retData["cmd"] = "missing command reply, bug?";
                        retData["status"] = false;
                        serializeJsonPretty(retData, stream);
                        return;
                    }

                    Udp.write(stream.getBuffer(), stream.getStreamSize());
                    Udp.endPacket();
                    stream.flush();
                }
            }, &ts, false, NULL, NULL
        };
};

#endif
