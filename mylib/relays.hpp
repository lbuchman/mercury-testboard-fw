

#ifndef RELAYS_H
#define RELAYS_H

#include "board.h"
#include <TaskSchedulerDeclarations.h>
#include <shellFunctor.hpp>

class Relay {
    public:
        Relay(Scheduler& _ts, int _pin, String _prefix): ts(_ts), pin(_pin) {
            pinMode(pin, INPUT_PULLUP);
            prefix = _prefix;
        }

        void begin() {
            ShellFunctor::getInstance().add(prefix, getPinValue);
        }

        int getValue() {
            return digitalRead(pin);
        }

        int getPin() {
            return pin;
        }

    private:
        String prefix;
        Scheduler& ts;
        int pin;
        shellFunc getPinValue = [this](int arg_cnt, char **args, Stream & stream) -> int {
            if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"none\", \"desc\": \"read relay input state\" },\n\r", stream)) {
                return 1;
            }

            int value = digitalRead(pin);
            stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], value);
            return 1;
        };
};

#endif
