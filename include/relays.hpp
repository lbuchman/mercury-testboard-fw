#ifndef RELAYS_H
#define RELAYS_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <board.h>
#include <shellFunctor.hpp>

class Relay {
public:
    Relay(Scheduler& scheduler, int relayPin, String commandPrefix)
        : ts(scheduler),
          pin(relayPin),
          prefix(commandPrefix) {
        pinMode(pin, INPUT_PULLUP);
    }

    void begin() { ShellFunctor::getInstance().add(prefix, getPinValue); }

private:
    Scheduler& ts;
    int pin;
    String prefix;

    shellFunc getPinValue = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(1, arg_cnt, args, "\t{ \"cmd\": \"%s\", \"arg\": \"none\", \"desc\": \"read relay input state\" }", stream)) {
            return 1;
        }

        int value = digitalRead(pin);
        stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], value);
        return 1;
    };
};

#endif
