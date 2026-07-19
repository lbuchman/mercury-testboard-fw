#include "TaskScheduler.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Cmd.h>
#include <TaskScheduler.h>
#include <board.h>
#include <hw.h>
#include <inputs.hpp>
#include <logger.hpp>
#include <networking.hpp>
#include <persistent.hpp>
#include <reader.hpp>
#include <relays.hpp>
#include <shellFunctor.hpp>
#include <singleLEDLibraryMod.h>
#include <stdlib.h>
#include <string.h>
#include <tcpTerminal.hpp>
#include <udpTerminal.hpp>
#include <watchdogWrapper.h>

Scheduler ts;

readerPins rd1Pins = { {9, OUTPUT, 0, 0}, {10, OUTPUT, 0, 0}, {5, INPUT_PULLUP, 0, 0}, {11, INPUT_PULLUP, 0, 0}, {40, INPUT_PULLUP, 0, 0}, {6, OUTPUT, 0, 0}, Serial2 };
readerPins rd2Pins = { {32, OUTPUT, 0, 0}, {26, OUTPUT, 0, 0}, {12, INPUT_PULLUP, 0, 0}, {30, INPUT_PULLUP, 0, 0}, {39, INPUT_PULLUP, 0, 0}, {27, OUTPUT, 0, 0}, Serial7 };
int Rs485TermPin = 31;
int rl1pin = 23;
int rl2pin = 3;
int rl3pin = 33;
int rl4pin = 37;

int sp1pin = 14;
int sp2pin = 18;
int sp3pin = 19;
int sp4pin = 15;

Task watchDogTask((WD_EXPIRE * TASK_SECOND) / 3, TASK_FOREVER,
                    [](void) -> void {
                        static int value = 0;
                        digitalWrite(WATCHDOG_LED, value);
                        value = value ^ 1;
                        watchdog();
                    },
                    &ts, false, NULL, NULL);

int setupFw() {
    pinMode(Rs485TermPin, OUTPUT);
    digitalWrite(Rs485TermPin, LOW);
    pinMode(WATCHDOG_LED, OUTPUT);
    CmdSerialDev.begin(SERIAL_BAUDRATE);
    LoggerSerialDev.begin(SERIAL_BAUDRATE);

    ShellFunctor& cshell = ShellFunctor::getInstance();
    loggerInit((Stream&)LoggerSerialDev);
    static sllibMod watchDogLed(WATCHDOG_LED, false);
    logger().warn(logger().printHeader, __FILE__, __LINE__, "*************** REBOOT ********************");
    logger().info(logger().printHeader, __FILE__, __LINE__, "Mercury test board FW ver %f. Build Time <%s-%s>", FWVERSION, __DATE__, __TIME__);
    logger().setLogLevel(logLevel::kLogInfo);

    watchDogTask.enable();
    enableWatchdog();
    watchDogLed.begin();
    boardInit();
    persistentDataInit();
    initNetworking(ts, watchDogLed);

    static SerialTerminal serialTerminalPi(cshell, ts);
    serialTerminalPi.begin(&CmdSerialDev, false, NULL, false);
    static TCPTerminal tCPTerminal {cshell, 23, ts};
    tCPTerminal.begin();
    static UdpTerminal udpTerminal(cshell, 4111, ts);
    udpTerminal.begin();

    static Reader reader1(ts, rd1Pins, "rd1");
    static Reader reader2(ts, rd2Pins, "rd2");
    static Relay rl1(ts, rl1pin, "rl1");
    static Relay rl2(ts, rl2pin, "rl2");
    static Relay rl3(ts, rl3pin, "rl3");
    static Relay rl4(ts, rl4pin, "rl4");
    static Input sp_i1(ts, sp1pin, "si1");
    static Input sp_i2(ts, sp2pin, "si2");
    static Input sp_i3(ts, sp3pin, "si3");
    static Input sp_i4(ts, sp4pin, "si4");
    reader1.begin();
    reader2.begin();
    rl1.begin();
    rl2.begin();
    rl3.begin();
    rl4.begin();
    sp_i1.begin();
    sp_i2.begin();
    sp_i3.begin();
    sp_i4.begin();

    ShellFunctor::getInstance().add("setterminalmode", [&](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(2, arg_cnt, args,
                           "\t{ \"cmd\": \"%s\", \"arg\": \"human|script\", \"desc\": \"set terminal interaction mode\" }", stream)) {
            return 1;
        }

        String mode = args[1];
        if (mode == "human") {
            serialTerminalPi.setSilentMode(false);
        } else if (mode == "script") {
            serialTerminalPi.setSilentMode(true);
        } else {
            JsonDocument doc;
            doc["status"] = false;
            doc["cmd"] = args[0];
            doc["error"] = "invalid argument";
            serializeJsonPretty(doc, stream);
            stream.println();
            return 1;
        }

        JsonDocument doc;
        doc["status"] = true;
        doc["cmd"] = args[0];
        doc["mode"] = mode;
        serializeJsonPretty(doc, stream);
        stream.println();
        return 1;
    });

    ShellFunctor::getInstance().add("about", [](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"desc\": \"info\" }", stream)) {
            return 1;
        }

        unsigned long totalSeconds = millis() / 1000;
        unsigned long totalMinutes = totalSeconds / 60;
        unsigned long totalHours = totalMinutes / 60;

        JsonDocument doc;
        doc["status"] = true;
        doc["cmd"] = args[0];
        doc["fw"] = FWVERSION;
        doc["uptime_days"] = totalHours / 24;
        doc["uptime_hours"] = totalHours % 24;
        doc["uptime_minutes"] = totalMinutes % 60;

        serializeJsonPretty(doc, stream);
        stream.println();
        return 1;
    });

    shellFunc getAllData = [&](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"get all active pins value\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"values\": [{\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}]}\n\r", args[0], rl1.getPin(), rl1.getValue(), rl2.getPin(), rl2.getValue(), rl3.getPin(), rl3.getValue(), rl4.getPin(), rl4.getValue(), sp_i1.getPin(), sp_i1.getValue(), sp_i2.getPin(), sp_i2.getValue(), sp_i3.getPin(), sp_i3.getValue(), sp_i4.getPin(), sp_i4.getValue(), reader1.getD0pinN(), reader1.getD0Value(), reader1.getD1PinN(), reader1.getD1Value(), reader1.getRLedPinN(), reader1.getRLedPin(), reader1.getGLedPinN(), reader1.getGLedPin(), reader1.getBzPinN(), reader1.getBzPin(), reader2.getD0pinN(), reader2.getD0Value(), reader2.getD1PinN(), reader2.getD1Value(), reader2.getRLedPinN(), reader2.getRLedPin(), reader2.getGLedPinN(), reader2.getGLedPin(), reader2.getBzPinN(), reader2.getBzPin());
        return 1;
    };
    ShellFunctor::getInstance().add("getalldata", getAllData);

    return 0;
}

void mainLoop() {
    while (true) {
        ts.execute();
    }
}
