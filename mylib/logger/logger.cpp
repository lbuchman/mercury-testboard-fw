#include <logger.hpp>


static Logger<LOGGER_SIZE> *instancePtr;

void loggerInit(Stream& _stream) {
    instancePtr = new Logger<LOGGER_SIZE>(LoggerSerialDev);
}
Logger<LOGGER_SIZE>& logger() {
    return *instancePtr;
}

