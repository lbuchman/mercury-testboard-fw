

#ifndef INPUTS_H
#define INPUTS_H

#include "board.h"
#include <TaskSchedulerDeclarations.h>
#include <shellFunctor.hpp>

class Input {
    public:
        Input(Scheduler& _ts, int _pin, String _prefix): ts(_ts), pin(_pin) {
            pinMode(pin, OUTPUT);
            prefix = _prefix;
            analogWrite(pin, 0);
        }

        void begin() {
            ShellFunctor::getInstance().add(prefix + "s", setValue);
        }


        int getValue() {
            return valueSet;
        }



        int getPin() {
            return pin;
        }

    private:
        String prefix;
        Scheduler& ts;
        int pin;
        int valueSet = 0;
        shellFunc setValue = [this](int arg_cnt, char **args, Stream & stream) -> int {
            if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"0-255, 255 = 4.93V, \", \"desc\": \"write suppervised voltage\" },\n\r", stream)) {
                return 1;
            }

            int value = atoi(args[1]);
            analogWrite(pin, value);
            valueSet = value;
            stream.printf("\t{\"cmd\": \"%s\", \"status\": true, \"value\": %d }\n\r", args[0], value);
            return 1;
        };
};

#endif
