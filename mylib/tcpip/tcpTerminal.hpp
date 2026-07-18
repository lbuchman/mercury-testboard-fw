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

#ifndef TCPTERMINAL___H
#define TCPTERMINAL___H

#include <Arduino.h>
#include <wiring.h>
#include <memory>
#include <utility>
#include <ArduinoJson.h>
#include <logger.hpp>
#include <NativeEthernet.h>
#include <TaskSchedulerDeclarations.h>
#include <etherUtilities.h>
#include <argvp.h>
#include <cmd.h>

#define OUTPUT_BUFFER_SIZE 4096*2
#define INPUT_BUFFER_SIZE 4096


struct TClient {
    TClient(EthernetClient _client, ShellFunctor& _shell, Scheduler& ts): client(_client), shell(_shell), taskScheduler(ts) {
        pserialTerminal = new SerialTerminal(shell, taskScheduler);
        client.printf("%s\n\r", cmd_telnet);
        pserialTerminal->begin(&client, false, [this]() -> void {
            closed = true;
            client.close();
            pserialTerminal->end();
            delete pserialTerminal;
            pserialTerminal = NULL;
        });
    }

    ~TClient() {
        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Deleting Client");
        delete pserialTerminal;
    }

    bool isClosed() {
        return closed;
    }
    const  char* cmd_telnet = "welcome to Teensydoino terminal";
    EthernetClient client;
    ShellFunctor& shell;
    SerialTerminal *pserialTerminal;
    bool closed = false;
    Scheduler& taskScheduler;
};



class TCPTerminal: public SimpleEvent {
    public:
        TCPTerminal(ShellFunctor& _shell, int port, Scheduler& ts_): shell(_shell), ts(ts_) {
            pserver = new EthernetServer(port);
        };

        void begin() {
            dataInTask.enable();
            garbageCollectorTask.enable();
        };


    private:
        ShellFunctor& shell;
        EthernetServer* pserver;
        std::vector<TClient*> clients;
        Scheduler& ts;
        Task dataInTask{TASK_MILLISECOND * 50, TASK_FOREVER, [this](void) -> void {
                EthernetClient client;

                if((client = pserver->accept())) {
                    TClient* newClient = new TClient(client, shell, ts);
                    clients.push_back(newClient);
                    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Client connected %s:%d", ipToString(client.remoteIP()).c_str(), client.remotePort());
                }
            }, &ts, false, NULL, NULL
        };

        Task garbageCollectorTask{TASK_SECOND * 3, TASK_FOREVER, [this](void) -> void {
                auto it = clients.begin();

                while(it != clients.end()) {
                    if(clients.size() == 0) {
                        return;
                    }

                    if(!(*it)->isClosed()) {
                        ++it;
                        continue;
                    }

                    delete *it;
                    it = clients.erase(it);
                    ++it;
                }

            }, &ts, false, NULL, NULL


        };
};


#endif
