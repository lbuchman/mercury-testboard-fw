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

#ifndef SIMPLE_EVENT_H
#define SIMPLE_EVENT_H

#include <stdio.h>
#include <string.h>
#include <functional>

#ifdef ARDUINO
#include <Arduino.h>
#else
#define String std::string
using namespace std;
#endif

typedef std::function<void (void*) > event;

class SimpleEvent {
    private:
        const static int maxEvents = 32;
        std::array<event, maxEvents> events;

    protected:
        String name;
        SimpleEvent(String _name = "noname") : name(_name) {
            // memset(events, 0, sizeof(events));
        };
        bool registerEvent(int eventId, event eventFunction) {
            if((eventId >= maxEvents)) {
                return false;
            }

            events[eventId] = eventFunction;
            return true;
        };

        void fireEvent(int eventId, void *pdata) {
            if(events[eventId]) {
                events[eventId](pdata);
            }
        }
};
#endif
