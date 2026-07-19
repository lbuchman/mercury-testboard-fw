#include <Arduino.h>
#include <cstdint>
#include <hw.h>
#include <init.h>

namespace std
{
void __throw_bad_function_call() {
    LoggerSerialDev.println("CRITICAL: Bad Function Call Stub!");

    while (1)
        ;
}

void __throw_length_error(char const* error) {
    LoggerSerialDev.print("CRITICAL Length Error: ");
    LoggerSerialDev.println(error);

    while (1)
        ;
}
} // namespace std

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    pinMode(WATCHDOG_LED, OUTPUT);
    LoggerSerialDev.begin(SERIAL_BAUDRATE);
    setupFw();
    mainLoop();
}
