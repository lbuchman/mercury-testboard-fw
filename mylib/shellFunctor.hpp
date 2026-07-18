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

#ifndef SHELL_FUNCTOR_H
#define SHELL_FUNCTOR_H

#include "Arduino.h"
#include <stdint.h>
#include <functional>
#include <map>
#include <TaskSchedulerDeclarations.h>


typedef std::function<int (int argc, char **argv, Stream* stream) > TerminalCallback;
typedef std::function<int (int argc, char **argv, Stream& stream) > shellFunc;
typedef std::map<String, shellFunc> shellEntry;

class ShellFunctor {
        bool m_isIncremental;
        int m_count;
    public:
        static ShellFunctor& getInstance() {
            static ShellFunctor instance; // Guaranteed to be destroyed.
            // Instantiated on first use.
            return instance;
        }

        void add(String cmd, shellFunc proc) {
            shallFunctions.insert({ cmd, proc });
        }

        int operator()(String func, int argc, char **argv, Stream& stream = Serial) {

            auto it =  shallFunctions.find(func);

            if(it != shallFunctions.end()) {
                return it->second(argc, argv, stream);
            }
            else {
                //logger.info(logger.printHeader, (char*) __FILE__, __LINE__, "executing cmd: motorgohome()");
                stream.printf("\t{ \"cmd\": \"%s\", \"status\": false, \"error\": \"command not found\" }\n\r", func.c_str());
            }

            return 0;
        }

        void help(Stream& stream, String _cmd) {
            stream.printf("[");
            String cmd;

            for(const auto& funcs : shallFunctions) {
                char *argv[1];

                cmd = funcs.first;

                if(cmd == "help") {
                    continue;
                }

                if(_cmd.length() && _cmd != cmd) {
                    continue;
                }

                argv[0] = (char*) cmd.c_str();
                funcs.second(0xFF, argv, stream);
            }

            stream.printf("]\n\r");
        }
    private:
        shellFunc printHelp = [this](int arg_cnt, char **args, Stream & stream) -> int {
            (void) arg_cnt;
            (void) args;
            char tmps[32] = {0};

            if(arg_cnt == 2) {
                strcpy(tmps, args[1]);
            }

            help(stream, tmps);
            return 1;
        };
        std::map<String, shellFunc> shallFunctions;
        ShellFunctor() {
            add("help", printHelp);
        };
        ShellFunctor(ShellFunctor const&) = delete;
        void operator=(ShellFunctor const&)  = delete;
};
#endif
