#include <Arduino.h>
#include <ArduinoJson.h>
#include <board.h>
#include <hw.h>
#include <logger.hpp>
#include <shellFunctor.hpp>
#include <utility.h>
#include <watchdogWrapper.h>

void boardInit() {
    static shellFunc reboot = [](int arg_cnt, char** args, Stream& stream) -> int {
        if (arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\" },\n\r", args[0]);
            return 1;
        }

        logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "executing %s", args[0]);
        stream.printf("\t{  \"cmd\": \"%s\",\"status\": true }\n\r", args[0]);
        // Todo save eeprom power data
        watchdogReboot();
        return 1;
    };

    static shellFunc dmesg = [](int arg_cnt, char** args, Stream& stream) -> int {
        (void)arg_cnt;

        if (arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"display system log buffer\" },\n\r", args[0]);
            return 1;
        }

        const char* str = nullptr;
        int count = 0;

        while (logger().getLog(&str)) {
            int cntr = strlen(str);

            if (cntr > 0) {
                stream.write((const uint8_t*)str, cntr - 1);
            }

            stream.print('\r');
            free((char*)str);
            count += 1;
        }

        if (!count) {
            stream.print("\n\r");
        }

        return 1;
    };

    static shellFunc watchdogtest = [](int arg_cnt, char** args, Stream& stream) {
        if (!checkArgument(1, arg_cnt, args, (char*)"\t{ \"cmd\": \"%s\", \"arg\": \"null\", \"desc\": \"Watchdog test\" }", stream)) {
            return 1;
        }

        while (1)
            ;

        return 1;
    };

    static shellFunc setll = [](int arg_cnt, char** args, Stream& stream) -> int {
        if (!checkArgument(2, arg_cnt, args,
                           (char*)"\t{ \"cmd\": \"%s\",  \"arg\": \"1 - 5\", \"desc\": \"set log level, error = 1 .. trace = 5\" }",
                           stream)) {
            return 1;
        }

        logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "executing %s %s", args[0], args[1]);

        int ll = atoi(args[1]);

        if (ll < 1 || ll > 5) {
            stream.printf("\t{ \"cmd\": \"%s\", \"status\": false, \"error\": \"invalid argument\" }\n\r", args[0], logger().getLogLevel());
            return 1;
        }

        logger().setLogLevel((logLevel)atoi(args[1]));
        stream.printf("\t{ \"cmd\": \"%s\", \"status\": true }\n\r", args[0]);
        return 1;
    };

    static shellFunc ll = [](int arg_cnt, char** args, Stream& stream) -> int {
        if (arg_cnt == 0xFF) {
            stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"get or set log level, error = 1 .. trace = 5\" },\n\r", args[0]);
            return 1;
        }

        logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "executing %s", args[0]);

        if (arg_cnt != 1) {
            stream.printf("\t{ \"cmd\": \"%s\", \"status\": false, \"error\": \"invalid argument\" }\n\r", args[0]);
            return 1;
        }

        stream.printf("\t{ \"loglevel\": %d}\n\r", logger().getLogLevel());
        return 1;
    };

    ShellFunctor::getInstance().add("reboot", reboot);
    ShellFunctor::getInstance().add("dmesg", dmesg);
    ShellFunctor::getInstance().add("watchdogtest", watchdogtest);
    ShellFunctor::getInstance().add("ll", ll);
    ShellFunctor::getInstance().add("setll", setll);
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
