/*******************************************************************
    Copyright (C) 2009 FreakLabs
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.

    Originally written by Christopher Wang aka Akiba.
    Please post support questions to the FreakLabs forum.

*******************************************************************/
/*!
    \file
    \ingroup


*/
/**************************************************************************/

#ifndef CMD_H
#define CMD_H

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <vector>
#include <algorithm>

#include <string.h>
#include "logger.hpp"

#define MAX_MSG_SIZE    128
#include <stdint.h>
#include <functional>

#include <shellFunctor.hpp>
#include <argvp.h>


#define PROMT "elements>"
typedef std::function<void () > OnCloseCallBack;


typedef struct _cmd_t {
    char *cmd;
    char *cmd_dsc;
    TerminalCallback func;
    struct _cmd_t *next;
} cmd_t;

class SerialTerminal {
    public:
        SerialTerminal() = delete;
        SerialTerminal(ShellFunctor& _shell, Scheduler& ts_) : shell(_shell), ts(ts_) {
            // msgSaved[0] = 0;
            // init the msg ptr
            msg_ptr = msg;
            // init the command table
            cmd_tbl_list = NULL;
            memset(msg, 0, sizeof(msg));
        };

        const SerialTerminal & operator= (const SerialTerminal& obj) { // a = b, obj = b
            this->cmd_tbl_list = obj.cmd_tbl_list;
            this->cmd_tbl = obj.cmd_tbl;
            return *this;
        }

        void begin(Stream* _stream, bool _echoOn = true,  OnCloseCallBack _onClose = NULL, bool _promptOn = true, uint32_t _poolInterval = 100 * TASK_MILLISECOND) {
            stream = _stream;
            echoOn = _echoOn;
            promptOn = _promptOn;
            msg_ptr = msg;
            poolInterval = _poolInterval;
            pollTask.setInterval(poolInterval);
            cmd_display();
            pollTask.restart();
            onClose = _onClose;
        }

        void setPrompt(bool value) {
            promptOn = value;
        }

        void end() {
            pollTask.disable();
            stopPoll = true;
        }

        void cmdPoll() {
            if(stopPoll) {
                return;
            }

            int ret = stream->available();

            while(ret) {
                cmd_handler();
                ret = stream->available();
            }
        }

        void help(Stream* printStream) { // not used
            printStream->printf("Valid commands:\n\r");

            for(cmd_t *cmd_entry = cmd_tbl; cmd_entry != NULL; cmd_entry = cmd_entry->next) {
                if(!cmd_entry->cmd || !cmd_entry->cmd_dsc || !strlen(cmd_entry->cmd_dsc) || !cmd_entry->func) {
                    continue;
                }

                printStream->printf("\t%12s\t%s\n\r", cmd_entry->cmd, cmd_entry->cmd_dsc);
            }

            printStream->printf("\n\r");
        }

        void printHistory() {
            for(auto it = history.begin() ; it != history.end(); ++it) {
                char *cmd = (char*) it->c_str();
                stream->printf("\t\n%s", cmd);
            }
        }

    private:
        bool echoOn;
        bool promptOn;
        uint32_t  poolInterval = 100 * TASK_MILLISECOND;
        bool escapeIsIn = false;
        int  escapeCount = 0;
        char escapeBuffer[4];
        const static  int Max_Escape_Char = 3;
        Stream* stream;
        bool stopPoll = false;
        uint8_t msg[MAX_MSG_SIZE];
        uint8_t *msg_ptr;
        cmd_t *cmd_tbl_list, *cmd_tbl;
        const String cmd_unrecog = "\tCommand not recognized.";
        std::vector<String> history;
        OnCloseCallBack onClose = NULL;
        ShellFunctor& shell;
        Scheduler& ts;
        int lastCommandIndex = -1;

        void cmd_display() {
            char buf[] = PROMT;

            if(promptOn)  {
                stream->printf("%s", buf);
            }
        }

        void cmd_parse(char *cmd) {
            if(!cmd) {
                return;
            }

            constexpr size_t argc_MAX = 16;
            char* argv[argc_MAX] = { 0 };
            int argc = 0;
            parseStrToArgcArgvInsitu((char*) cmd, argc_MAX, &argc, argv);

            if(String(argv[0]) == "exit" && onClose) {
                onClose();
                pollTask.disable();
                return;
            }

            shell(argv[0], argc, argv, (*stream));
            cmd_display();
        }

        Task pollTask{poolInterval, TASK_FOREVER, [this](void) -> void {
                cmdPoll();
            }, &ts, false};


        void cmd_handler() {
            char c = stream->read();

            if(escapeIsIn) {
                escapeBuffer[ escapeCount] = c;
                escapeCount += 1;

                if(escapeCount == Max_Escape_Char) {
                    escapeIsIn = false;
                    escapeCount = 0;
                    uint32_t escapeCode;
                    memcpy(&escapeCode, escapeBuffer, sizeof(escapeCode));

                    if(escapeCode == 0x415b1b) {
                        if(lastCommandIndex < 0) {
                            return;;
                        }

                        stream->printf("\33[2K\r%s%s", PROMT, history[lastCommandIndex].trim().c_str());
                        strncpy((char*) msg, history[lastCommandIndex].trim().c_str(), sizeof(msg));

                        msg_ptr = &msg[strlen((char*) msg)];

                        if((uint32_t) lastCommandIndex > 0)  {
                            lastCommandIndex -= 1;
                        }

                        return;
                    }

                    if(escapeCode == 0x425b1b) {
                        if(!history.size() || (unsigned) lastCommandIndex > history.size() - 1) {
                            return;
                        }


                        stream->printf("\33[2K\r%s%s", PROMT, history[lastCommandIndex].trim().c_str());
                        strncpy((char*) msg, history[lastCommandIndex].trim().c_str(), sizeof(msg));

                        msg_ptr = &msg[strlen((char*) msg)];

                        if((uint32_t) lastCommandIndex < history.size() - 1) {
                            lastCommandIndex += 1;
                        }

                        return;
                    }
                }
            }
            else
                switch(c) {
                    case '\r': {
                            // terminate the msg and reset the msg ptr. then send
                            // it to the handler for processing.
                            *msg_ptr = '\0';

                            if(echoOn) {
                                stream->printf("\n\r");
                            }

                            if(!strlen((char*) msg)) {
                                cmd_display();
                                //  stream->flush();
                                msg_ptr = msg;
                                return;
                            }


                            String str = String((const char*) msg);
                            str.trim();

                            // stream->flush(); takes ages 180uSec on HW Serial for some reason, USB is fine
                            auto it = find_if(history.begin(), history.end(), [&str](const String & obj) {
                                return str == obj;
                            });

                            if(it == history.end()) {
                                history.push_back(str);
                                lastCommandIndex = history.size() - 1;
                            }

                            cmd_parse((char *) str.c_str());
                            // stream->flush();
                            msg_ptr = msg;

                            break;
                        }

                    case '\n':
                        break;

                    case '\b':
                        if(msg_ptr > msg) {
                            msg_ptr--;
                            stream->printf("\033[D\033[J", c);
                        }

                        break;

                    case 0xFF:
                    case 0xFD:
                    case 0x03:
                    case 0xFB:
                    case 0x18:
                    case 0x1F:
                    case 0x21:
                    case 0x27:
                    case 0x22:
                    case 0x23:
                    case 0x05:
                        break;

                    case 0x1B: {// escape
                            escapeIsIn = true;
                            escapeBuffer[0] = c;
                            escapeCount = 1;
                            break;
                        }

                    default:

                        // normal character entered. add it to the buffer
                        if(echoOn) {
                            stream->printf("%c", c);
                        }

                        *msg_ptr++ = c;
                        break;
                }
        }
};
#endif //CMD_H
