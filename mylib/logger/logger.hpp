/*
MIT License


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __LOGGER__H___
#define __LOGGER__H___

#include <Arduino.h>
#include <hw.h>
#include <stdio.h>
#include <stdarg.h>
#include <TimeLib.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <ringbuffer.hpp>
#include <time.h>

enum class logLevel : int8_t { kLogNone, kLogError, kLogWarning, kLogInfo, kLogDebug, kLogTrace };

template<size_t buffer_size>
class Logger {
    public:
        Logger(Stream& _stream) : stream(_stream) {};
        int  trace(bool printHeader, const char* filename, int lineNo, const char *Format, ...);
        int  info(bool printHeader, const char* filename, int lineNo, const char *Format, ...);
        int  warn(bool printHeader, const char* filename, int lineNo, const char *Format, ...);
        int  debug(bool printHeader, const char* filename, int lineNo, const char *Format, ...);
        int  error(bool printHeader, const char* filename, int lineNo, const char *Format, ...);


        int getTimeStamp() {
            return millis();
        }

        void setLogLevel(logLevel level) {
            systemLogLevel = level;
        };

        logLevel getLogLevel() {
            return  systemLogLevel;
        };
        void hexDump(const void * memory, size_t bytes, bool printTimetsamp = false, logLevel loglevel = logLevel::kLogInfo);
        void setStream(Stream& _stream) {
            stream = _stream;
        };
        Stream& getLogStream() {
            return stream;
        };
        int  available();
        bool getLog(const char** buffer);
        bool putStrLog(const char* str);
        String getFileName(const String& s) {
            char sep = '/';
            int i = s.lastIndexOf(sep);

            if(i != -1) {
                String filename = s.substring(i + 1);
                return (filename);
            }

            return ("");
        }

        void setIsrContext() {
            isrContext = true;
        };
        void clearIsrContext() {
            isrContext = false;
        };
        void enableCriticalSection(bool value) {
            // criticalSection = true;
        }

        const static bool printHeader = true;
        const static bool noHeader = true;

    private:
        Logger(const Logger &logger) = delete;
        Logger<LOGGER_SIZE> *instancePtr;

    protected:
        Stream& stream;
        bool criticalSection = false;
        bool streamDevNull = false;
        logLevel systemLogLevel = logLevel::kLogDebug;
        jnk0le::Ringbuffer<const char*, buffer_size, false, 8> logsRingBuffer;
        uint32_t hexDumpPrevPrintTimestamp = 0;
        bool isrContext = false;
        int makeHeader(logLevel loglevel, bool printHeader, const char* filename, int lineNo, char* printbuffer);
        int printCharNTimes(char ch, int n, char* buffer);
};


template<size_t buffer_size>
int Logger<buffer_size>::makeHeader(logLevel loglevel, bool printHeader, const char* filename, int lineNo, char* printbuffer) {
    char             LogType[128];

    printbuffer[0] = 0;

    if(loglevel > systemLogLevel) {
        return 0;
    }

    if(printHeader) {
        if(criticalSection) {
            noInterrupts();
        }

        switch(loglevel) {
            case logLevel::kLogError:
                sprintf(LogType, "%5s", "\033[0;31m[error]\033[0m  ");
                break;

            case logLevel::kLogWarning:
                sprintf(LogType, "%5s", "\033[0;36m[warn]\033[0m   ");
                break;

            case logLevel::kLogDebug:
                sprintf(LogType, "%5s", "\033[0;33m[debug]\033[0m  ");
                break;

            case logLevel::kLogInfo:
                sprintf(LogType, "\033[0;32m%5s\033[0m", "[info]   ");
                break;

            case logLevel::kLogTrace:
                sprintf(LogType, "\033[0;37m%5s\033[0m", "[trace] ");
                break;

            case logLevel::kLogNone:
                break;

            default:
                if(criticalSection) {
                    interrupts();
                }

                return 0;
        }

        if(filename) {
            sprintf(&LogType[strlen(LogType)], "%s:[%d] ", getFileName(filename).c_str(), lineNo);
        }

        if(criticalSection) {
            interrupts();
        }

        if(criticalSection) {
            noInterrupts();
        }

        int ret = sprintf(printbuffer, "[%11d] %7s", getTimeStamp(),  LogType);

        if(criticalSection) {
            interrupts();
        }

        return ret;
    }

    return sprintf(printbuffer, "[%11d]", getTimeStamp());
}


template<size_t buffer_size>
int Logger<buffer_size>::info(bool printHeader, const char* filename, int lineNo, const char *Format, ...) {


    va_list          args;
    char             printbuffer[512];

    if(!makeHeader(logLevel::kLogInfo, printHeader, filename, lineNo, printbuffer)) {
        return 0;
    }

    if(criticalSection) {
        noInterrupts();
    }

    va_start(args, Format);
    vsprintf(&printbuffer[strlen(printbuffer)], Format, args);
    va_end(args);
    int size = 0;

    if(!streamDevNull) {
        if(!isrContext) {
            stream.printf("%s\n\r", printbuffer);
        }
    }

    if(criticalSection) {
        interrupts();
    }

    strcat(printbuffer, "\n\r");
    putStrLog(printbuffer);
    return size;
}

template<size_t buffer_size>
int Logger<buffer_size>::trace(bool printHeader, const char* filename, int lineNo, const char *Format, ...) {


    va_list          args;
    char             printbuffer[512];

    if(!makeHeader(logLevel::kLogTrace, printHeader, filename, lineNo, printbuffer)) {
        return 0;
    }

    if(criticalSection) {
        noInterrupts();
    }

    va_start(args, Format);
    vsprintf(&printbuffer[strlen(printbuffer)], Format, args);
    va_end(args);
    int size = 0;

    if(!streamDevNull) {
        if(!isrContext) {
            stream.printf("%s\n\r", printbuffer);
        }
    }

    if(criticalSection) {
        interrupts();
    }

    strcat(printbuffer, "\n\r");
    putStrLog(printbuffer);
    return size;
}

template<size_t buffer_size>
int Logger<buffer_size>::debug(bool printHeader, const char* filename, int lineNo, const char *Format, ...) {


    va_list          args;
    char             printbuffer[512];

    if(!makeHeader(logLevel::kLogDebug, printHeader, filename, lineNo, printbuffer)) {
        return 0;
    }

    if(criticalSection) {
        noInterrupts();
    }

    va_start(args, Format);
    vsprintf(&printbuffer[strlen(printbuffer)], Format, args);
    va_end(args);
    int size = 0;

    if(!streamDevNull) {
        if(!isrContext) {
            stream.printf("%s\n\r", printbuffer);
        }
    }

    if(criticalSection) {
        interrupts();
    }

    strcat(printbuffer, "\n\r");
    putStrLog(printbuffer);
    return size;
}

template<size_t buffer_size>
int Logger<buffer_size>::warn(bool printHeader, const char* filename, int lineNo, const char *Format, ...) {


    va_list          args;
    char             printbuffer[512];

    if(!makeHeader(logLevel::kLogWarning, printHeader, filename, lineNo, printbuffer)) {
        return 0;
    }

    if(criticalSection) {
        noInterrupts();
    }

    va_start(args, Format);
    vsprintf(&printbuffer[strlen(printbuffer)], Format, args);
    va_end(args);
    int size = 0;

    if(!streamDevNull) {
        if(!isrContext) {
            stream.printf("%s\n\r", printbuffer);
        }
    }

    if(criticalSection) {
        interrupts();
    }

    strcat(printbuffer, "\n\r");
    putStrLog(printbuffer);
    return size;
}

template<size_t buffer_size> int Logger<buffer_size>::error(bool printHeader, const char* filename, int lineNo, const char *Format, ...) {
    va_list          args;
    char             printbuffer[512];

    if(!makeHeader(logLevel::kLogError, printHeader, filename, lineNo, printbuffer)) {
        return 0;
    }

    if(criticalSection) {
        noInterrupts();
    }

    va_start(args, Format);
    vsprintf(&printbuffer[strlen(printbuffer)], Format, args);
    va_end(args);
    int size = 0;

    if(!streamDevNull) {
        if(!isrContext) {
            stream.printf("%s\n\r", printbuffer);
        }
    }

    if(criticalSection) {
        interrupts();
    }

    strcat(printbuffer, "\n\r");
    putStrLog(printbuffer);
    return size;
}

template<size_t buffer_size>
bool Logger<buffer_size>::getLog(const char** buffer) {
    return logsRingBuffer.remove(*buffer);
}

template<size_t buffer_size>
bool Logger<buffer_size>::putStrLog(const char* str) {
    (void) str;

    size_t strLength = strlen(str);    // does not include \0 will have o add 1

    if(strLength > 254) {
        return false;
    }

    char* fstr = (char*) malloc(256);      // to avoid memory fragmentation

    if(!fstr) {
        return false;
    }

    strcpy(fstr, str);
    // FastCRC8 fastcrc;
    // uint32_t crc8 = fastcrc.smbus((uint8_t*) str, strLength + 1);
    fstr[strLength + 1] = 0; //crc8;

    if(criticalSection) {
        noInterrupts();
    }

    if(!logsRingBuffer.insert(fstr)) {
        free((char*) fstr);
    }

    if(criticalSection) {
        interrupts();
    }

    return true;
}

template<size_t buffer_size>
int Logger<buffer_size>::available() {
    const char **str = logsRingBuffer.peek();

    if(!str) {
        return 0;
    }

    int ret = strnlen(*str, 256);
    return ret;
}

template<size_t buffer_size>
int Logger<buffer_size>::printCharNTimes(char ch, int n, char* buffer) {
    int count = 0;

    if(n > 257) {
        return 0;
    }

    for(count = 0; count < n; count++) {
        buffer[count] = ch;
    }

    return count;
}

template<size_t buffer_size>
void Logger<buffer_size>::hexDump(const void * memory, size_t bytes, bool printTimetsamp, logLevel loglevel) {
    unsigned long timeNow = millis();

    if(loglevel > systemLogLevel) {
        hexDumpPrevPrintTimestamp = timeNow;
        //  return;
    }

    char buffer[512];
    const  char * p, * q;
    int i;

    p = (char *) memory;

    while(bytes) {
        q = p;
        buffer[0] = 0;
        int strLength = 0;

        if(printTimetsamp) {
            float uSecTime = (float)(timeNow / 1000.0);
            float intervalUsecTime = (float)((timeNow - hexDumpPrevPrintTimestamp) / 1000.0);
            sprintf(buffer, "[%11.3f : %9.3f]  ", uSecTime, intervalUsecTime);

        }
        else {
            sprintf(buffer, "%s", (char*) "                           ");
        }

        printTimetsamp = false;

        for(i = 0; i < 16 && bytes; ++i) {
            strLength = strlen(buffer);
            sprintf(&buffer[strLength], "%02X ", (char) *p  & 0xFF);
            ++p;
            --bytes;
        }

        bytes += i;

        while(i < 16) {
            strLength = strlen(buffer);
            sprintf(&buffer[strLength], "XX ");
            ++i;
        }

        strLength = strlen(buffer);
        sprintf(&buffer[strLength], "| ");
        p = q;

        for(i = 0; i < 16 && bytes; ++i) {
            strLength = strlen(buffer);
            // sprintf(&buffer[strLength], "\033[1;32m%c\033[0m", isprint(*p) && !isspace(*p) ? (char)*p & 0xFF : ' ');
            sprintf(&buffer[strLength], "%c", isprint(*p) && !isspace(*p) ? (char) *p & 0xFF : ' ');
            ++p;
            --bytes;
        }

        while(i < 16) {
            strLength = strlen(buffer);
            sprintf(&buffer[strLength], " ");
            ++i;
        }

        strLength = strlen(buffer);
        sprintf(&buffer[strLength], " |\n\r");

        if(!streamDevNull) {
            if(!isrContext) {
                stream.print(buffer);
            }
        }

        putStrLog(buffer);
    }

    hexDumpPrevPrintTimestamp = timeNow;
}

Logger<LOGGER_SIZE>& logger();
void loggerInit(Stream& _stream);
#endif //__LOGGER__H__
