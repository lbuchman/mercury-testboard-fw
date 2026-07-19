#ifndef INPUTS_H
#define INPUTS_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <board.h>
#include <shellFunctor.hpp>

class Input {
public:
    Input(Scheduler& scheduler, int inputPin, String commandPrefix)
        : ts(scheduler),
          pin(inputPin),
          prefix(commandPrefix) {
        pinMode(pin, OUTPUT);
        analogWrite(pin, 0);
    }

    void begin() { ShellFunctor::getInstance().add(prefix + "s", setValue); }

private:
    Scheduler& ts;
    int pin;
    String prefix;
    int valueSet = 0;

    shellFunc setValue = [this](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(2, arg_cnt, args,
                           "\t{ \"cmd\": \"%s\", \"arg\": \"0-255, 255 = 4.93V\", \"desc\": \"write supervised voltage\" }", stream)) {
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
