#include <Arduino.h>
#include <ArduinoJson.h>
#include <shellFunctor.hpp>
#include <board.h>
#include <logger.hpp>
#include <utility.h>
#include <watchdog.h>

uint8_t  output2Pin(int output) {
    switch(output) {
        case 1:
            return AOutput1;

        case 2:
            return AOutput2;

        case 3:
            return AOutput3;

        case 4:
            return AOutput4;

        case 5:
            return AOutput5;

        case 6:
            return AOutput6;

        case 7:
            return AOutput7;

        case 8:
            return AOutput8;

        default:
            return 0;
    }
}

void boardInit() {
    static shellFunc reboot = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\" },\n\r", args[0]);
            return 1;
        }

        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "executing %s", args[0]);
        stream.printf("\t{  \"cmd\": \"%s\",\"status\": true }\n\r", args[0]);
        // Todo save eeprom power data
        watchdogReboot();
        return 1;
    };

    static shellFunc dmesg = [](int arg_cnt, char **args, Stream & stream) -> int {
        (void)arg_cnt;

        if(arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"display system log buffer\" },\n\r", args[0]);
            return 1;
        }

        const char* str = nullptr;
        int count = 0;

        while(logger().getLog(&str)) {
            int cntr = strlen(str);
            char* pstr = (char*) &str[0];
            pstr[cntr - 1] = 0;
            stream.print(str);
            stream.print('\r');
            free((char*) str);
            count += 1;
        }

        if(!count) {
            stream.print("\n\r");
        }

        return 1;
    };

    static shellFunc watchdogtest = [](int arg_cnt, char **args, Stream & stream) {
        if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"arg\": \"null\", \"desc\": \"Watchdog test\" },\n\r", stream)) {
            return 1;
        }

        while(1);

        return 1;
    };

    static shellFunc setll = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"1 - 5\", \"desc\": \"set log level, error = 1 .. trace = 5\" },\n\r", stream)) {
            return 1;
        }

        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "executing %s %s", args[0], args[1]);

        int ll = atoi(args[1]);

        if(ll < 1 ||  ll > 5) {
            stream.printf("\t{ \"cmd\": \"%s\", \"status\": false, \"error\": \"invalid argument\" }\n\r", args[0], logger().getLogLevel());
            return 1;
        }

        logger().setLogLevel((logLevel)atoi(args[1]));
        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true }\n\r", args[0]);
        return 1;
    };

    static shellFunc ll = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"get or set log level, error = 1 .. trace = 5\" },\n\r", args[0]);
            return 1;
        }

        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "executing %s", args[0]);

        if(arg_cnt == 1) {
            stream.printf("\t{ \"loglevel\": %d}\n\r", logger().getLogLevel());
            return 1;
        }

        logger().setLogLevel((logLevel)atoi(args[1]));
        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"loglevel\": %d }\n\r", args[0], logger().getLogLevel());
        return 1;
    };

    static shellFunc setpromt =  [](int arg_cnt, char **args, Stream & stream) -> int {
        if(arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\", \"arg\": \"0 or 1\", \"desc\": \"enable or disable terminal prompt\" },\n\r", args[0]);
            return 1;
        }

        if(arg_cnt < 2) {
            logger().error(logger().printHeader, (char*) __FILE__, __LINE__, "setPrompt() invalid argument");
            stream.printf("\t{ \"status\": false, \"error\": \"invalid argument\" }\n\r");
            return 1;
        }

        logger().debug(logger().printHeader, (char*) __FILE__, __LINE__, "executing %s arg = %s", args[0], args[1]);
        // Todo pSerialTerminal->setPrompt(atoi(args[1]));
        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true }\n\r", args[0]);

        return 1;
    };

    static shellFunc  closeTerminal = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"desc\": \"close current connection, only valid for telnet\" },\n\r", stream)) {
            return 1;
        }

        stream.printf("\t{ \"cmd\": \"%s\", \"status\": \"true\" }\n\r", args[0]);
        stream.peek();
        return 1;
    };


    static shellFunc setlight = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"input pwm\", \"desc\": \"write pwm to HW\" },\n\r", stream)) {
            return 1;
        }

        uint32_t value =  atoi(args[1]);

        if(value < 50) {
            value = 80;
        }

        analogWrite(output2Pin(1), atoi(args[1]));
        analogWrite(output2Pin(2), atoi(args[1]));
        analogWrite(output2Pin(3), atoi(args[1]));
        analogWrite(output2Pin(4), atoi(args[1]));

        JsonDocument doc;
        JsonObject  retData =  doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = true;
        serializeJsonPretty(retData, stream);
        return 1;
    };

    static shellFunc trypwm = [](int arg_cnt, char **args, Stream & stream) -> int {
        if(!checkArgument(3, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"input pwm\", \"desc\": \"write pwm to HW\" },\n\r", stream)) {
            return 1;
        }

        int input = atoi(args[1]);

        if(input < 1 || input > 8) {
            stream.printf("\t{\"cmd\": \"%s\", \"status\": \"false\", \"error\": \"invalid argument\" }\n\r", args[0]);
            return 1;
        }

        analogWrite(output2Pin(input), atoi(args[2]));

        JsonDocument doc;
        JsonObject  retData =  doc.to<JsonObject>();
        retData["cmd"] = args[0];
        retData["status"] = true;
        serializeJsonPretty(retData, stream);
        return 1;
    };

    ShellFunctor::getInstance().add("reboot", reboot);
    ShellFunctor::getInstance().add("dmesg", dmesg);
    ShellFunctor::getInstance().add("watchdogtest", watchdogtest);
    ShellFunctor::getInstance().add("ll", ll);
    ShellFunctor::getInstance().add("setpromt", setpromt);
    ShellFunctor::getInstance().add("setll", setll);
    ShellFunctor::getInstance().add("closeTerminal", closeTerminal);
    // GPIO Init

    pinMode(AOutput1, OUTPUT);
    pinMode(AOutput2, OUTPUT);
    pinMode(AOutput3, OUTPUT);
    pinMode(AOutput4, OUTPUT);
    pinMode(AOutput5, OUTPUT);
    pinMode(AOutput6, OUTPUT);
    pinMode(AOutput7, OUTPUT);
    pinMode(AOutput8, OUTPUT);
    analogWrite(AOutput1, 80);
    analogWrite(AOutput2, 80);
    analogWrite(AOutput3, 80);
    analogWrite(AOutput4, 80);
}



