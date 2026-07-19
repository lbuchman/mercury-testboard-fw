#ifndef READER_H
#define READER_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <board.h>
#include <shellFunctor.hpp>

class Reader {
public:
    Reader(Scheduler& scheduler, readerPins& readerPins, String commandPrefix): ts(scheduler), pins(readerPins), prefix(commandPrefix) {
        pinMode(pins.d0.pinN, pins.d0.pinMode);
        pinMode(pins.d1.pinN, pins.d1.pinMode);
        pinMode(pins.rLed.pinN, pins.rLed.pinMode);
        pinMode(pins.gLed.pinN, pins.gLed.pinMode);
        pinMode(pins.bz.pinN, pins.bz.pinMode);
        pinMode(pins.de.pinN, pins.de.pinMode);
        pins.serial.transmitterEnable(pins.de.pinN);
        digitalWrite(pins.d0.pinN, pins.d0.defValue);
        digitalWrite(pins.d1.pinN, pins.d1.defValue);
        pins.serial.begin(SERIAL_BAUDRATE);
        osdpTask.enable();
    }

    void begin() {
        ShellFunctor::getInstance().add(prefix + "d0", setD0);
        ShellFunctor::getInstance().add(prefix + "d1", setD1);
        ShellFunctor::getInstance().add(prefix + "rd0", getD0);
        ShellFunctor::getInstance().add(prefix + "rd1", getD1);
        ShellFunctor::getInstance().add(prefix + "rled", getRLed);
        ShellFunctor::getInstance().add(prefix + "gled", getGLed);
        ShellFunctor::getInstance().add(prefix + "bz", getBz);
    }

    int getRLedPin() { return digitalRead(pins.rLed.pinN); }
    int getGLedPin() { return digitalRead(pins.gLed.pinN); }
    int getBzPin() { return digitalRead(pins.bz.pinN); }
    int getD0Value() { return pins.d0.value; }
    int getD1Value() { return pins.d1.value; }
    int getRLedPinN() { return pins.rLed.pinN; }
    int getGLedPinN() { return pins.gLed.pinN; }
    int getBzPinN() { return pins.bz.pinN; }
    int getD0pinN() { return pins.d0.pinN; }
    int getD1PinN() { return pins.d1.pinN; }

private:
    Scheduler& ts;
    readerPins& pins;
    String prefix;

    shellFunc setD0 = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(2, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"0|1\", \"desc\": \"set D0\" },\n\r", stream)) {
            return 1;
        }

        digitalWrite(pins.d0.pinN, atoi(args[1]));
        pins.d0.value = atoi(args[1]);
        stream.printf("\t{\"cmd\": \"%s\", \"status\": true }\n\r", args[0]);
        return 1;
    };

    shellFunc getD0 = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read D0\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], pins.d0.value);
        return 1;
    };

    shellFunc setD1 = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(2, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"0|1\", \"desc\": \"set D1\" },\n\r", stream)) {
            return 1;
        }

        digitalWrite(pins.d1.pinN, atoi(args[1]));
        pins.d1.value = atoi(args[1]);
        stream.printf("\t{\"cmd\": \"%s\", \"status\": true }\n\r", args[0]);
        return 1;
    };

    shellFunc getD1 = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read D1\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], pins.d1.value);
        return 1;
    };

    shellFunc getRLed = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read red led input state\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], digitalRead(pins.rLed.pinN));
        return 1;
    };

    shellFunc getGLed = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read green led input state\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], digitalRead(pins.gLed.pinN));
        return 1;
    };

    shellFunc getBz = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read buzzer input state\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], digitalRead(pins.bz.pinN));
        return 1;
    };

    Task osdpTask { TASK_MILLISECOND, TASK_FOREVER, [this]() {
                       if (pins.serial.available()) {
                           char inChar = pins.serial.read();
                           pins.serial.write(inChar);
                       }
                   },
                   &ts, false };
};

#endif
