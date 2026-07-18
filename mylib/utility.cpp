#include <Arduino.h>
#include <utility.h>


bool checkArgument(int expectedArgCnt, int arg_cnt, char **args, char* helpFormat, Stream & stream) {
    if(arg_cnt == 0xFF && helpFormat) {
        stream.printf(helpFormat, args[0]);
        return false;
    }

    if(arg_cnt != expectedArgCnt) {
        stream.printf("\t{ \"cmd\": \"%s\", \"status\": false, \"error\": \"invalid argument\" }\n\r", args[0]);
        return false;
    }

    return true;
};
