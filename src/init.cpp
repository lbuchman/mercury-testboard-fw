// Copyright 2024 Leo Buchman
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <hw.h>
#include <TaskScheduler.h>
#include <watchdog.h>
#include <NativeEthernet.h>
#include <singleLEDLibraryMod.h>
#include <shellFunctor.hpp>
#include <UdpNtpClient.hpp>
#include <udpTerminal.hpp>
#include <etherUtilities.h>
#include <logger.hpp>
#include <persistent.hpp>
#include <networking.hpp>
#include <cmd.h>
#include <utility.h>
#include <board.h>
#include <tcpTerminal.hpp>
#include <reader.hpp>
#include <relays.hpp>
#include <inputs.hpp>

#define NO_NETWORK_LED_INT 1500000
#define OK_NETWORK_LED_INT 500000
#define WATCHDOG_INT TASK_SECOND * 2
using namespace std;
Scheduler ts;

#define Rev "0.1"

readerPins rd1Pins = { {9, OUTPUT, 0, 0}, {10, OUTPUT, 0, 0}, {5, INPUT_PULLUP, 0, 0}, {11, INPUT_PULLUP, 0, 0}, {40, INPUT_PULLUP, 0, 0}, {6, OUTPUT, 0, 0}, Serial2 };
readerPins rd2Pins = { {32, OUTPUT, 0, 0}, {26, OUTPUT, 0, 0}, {12, INPUT_PULLUP, 0, 0}, {30, INPUT_PULLUP, 0, 0}, {39, INPUT_PULLUP, 0, 0}, {27, OUTPUT, 0, 0}, Serial7 };
int Rs485TermPin = 31; // no need for termination, set to low
// int Rd1Rs485De = 6; // no need for termination, set to low
// int Rd2Rs485De = 27;
int rl1pin = 23;
int rl2pin = 3;
int rl3pin = 33;
int rl4pin = 37;

int sp1pin = 14;
int sp2pin = 18;
int sp3pin = 19;
int sp4pin = 15;

int main() {
    // pinMode(Rd1Rs485De, OUTPUT);
    // digitalWrite(Rd1Rs485De, LOW);
    // pinMode(Rd2Rs485De, OUTPUT);
    // digitalWrite(Rd2Rs485De, LOW);
    pinMode(Rs485TermPin, OUTPUT);
    digitalWrite(Rs485TermPin, LOW);
    pinMode(WATCHDOG_LED, OUTPUT);
    LoggerSerialDev.begin(LoggerSerialBaudRate);
    ShellFunctor& cshell = ShellFunctor::getInstance();
    loggerInit((Stream&)LoggerSerialDev);
    sllibMod watchDogLed(WATCHDOG_LED, false);
    logger().warn(logger().printHeader,  __FILE__, __LINE__, "\n\r*************** REBOOT ******************** %d");
    String revBuild = String(Rev) + "" + __DATE__ + "-" + __TIME__;
    logger().info(logger().printHeader, __FILE__, __LINE__, "Micronode Plus testing board %s. Build Time <%s-%s>", Rev, __DATE__, __TIME__);
    logger().setLogLevel(logLevel::kLogInfo);

    Task watchdogTaskHw(WATCHDOG_INT, TASK_FOREVER, function<void()> ([](void) -> void {
        watchdog();
    }), &ts, false);

    watchDogLed.begin();
    watchDogLed.ledNoEtherlink();
    watchdogTaskHw.enable();
    enableWatchdog();
    persistentDataInit();
    boardInit();
    initNetworking(ts, watchDogLed);

    SerialTerminal serialTerminal(cshell, ts);
    Serial.begin(115200);
    serialTerminal.begin(&Serial, true);
    TCPTerminal tCPTerminal{cshell, 23, ts};
    tCPTerminal.begin();
    UdpTerminal udpTerminal(cshell, 4111, ts);
    udpTerminal.begin();
    Reader reader1(ts, rd1Pins, "rd1");
    Reader reader2(ts, rd2Pins, "rd2");
    Relay  rl1(ts, rl1pin, "rl1");
    Relay  rl2(ts, rl2pin, "rl2");
    Relay  rl3(ts, rl3pin, "rl3");
    Relay  rl4(ts, rl4pin, "rl4");
    Input  sp_i1(ts, sp1pin, "si1");
    Input  sp_i2(ts, sp2pin, "si2");
    Input  sp_i3(ts, sp3pin, "si3");
    Input  sp_i4(ts, sp4pin, "si4");
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


    shellFunc getAllData = [&](int arg_cnt, char **args, Stream & stream) -> int {
        if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"get all active pins value\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"values\": [{\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}, {\"pin\": %d, \"value\": %d}]}\n\r", args[0], rl1.getPin(), rl1.getValue(), rl2.getPin(), rl2.getValue(), rl3.getPin(), rl3.getValue(), rl4.getPin(), rl4.getValue(), sp_i1.getPin(), sp_i1.getValue(), sp_i2.getPin(), sp_i2.getValue(), sp_i3.getPin(), sp_i3.getValue(), sp_i4.getPin(), sp_i4.getValue(), reader1.getD0pinN(), reader1.getD0Value(), reader1.getD1PinN(), reader1.getD1Value(), reader1.getRLedPinN(), reader1.getRLedPin(), reader1.getGLedPinN(), reader1.getGLedPin(), reader1.getBzPinN(), reader1.getBzPin(), reader2.getD0pinN(), reader2.getD0Value(), reader2.getD1PinN(), reader2.getD1Value(), reader2.getRLedPinN(), reader2.getRLedPin(), reader2.getGLedPinN(), reader2.getGLedPin(), reader2.getBzPinN(), reader2.getBzPin());
        return 1;
    };
    ShellFunctor::getInstance().add("getalldata", getAllData);

    while(true) {
        ts.execute();
    }

    return 1;
}



