/*singleLEDLibrary
* A library for non interupting lighting effects for single LED's
* Pim Ostendorf - 2017.11.24
*/

#ifndef singleLEDLibraryMod_hh
#define singleLEDLibraryMod_hh
#include <Arduino.h>
#include <wiring.h>
#include <singleLEDLibrary.h>
#include <TaskSchedulerDeclarations.h>



static constexpr int noLinkPattern[] = {100, 300, 100, 3000}; // 2 short
static constexpr int noDHCPPattern[] = {100, 300, 100, 300, 100, 3000}; // 3 short
static constexpr int watchdogPattern[] = {400, 600};

class sllibMod : public sllib {
    public:
        //public variables and fucntions
        sllibMod(int pin, bool pwmPin = false): sllib(pin, pwmPin) {
            self = this;
        };

        void begin() {
            setMaxPwm(255);
            ledNoEtherlink();
            ledTask.begin(ledISR, 100000);
        };

        void ledNoEtherlink() {
            if(patern == 0) {
                return;
            }

            noInterrupts();
            setPatternSingle((int*) noLinkPattern, sizeof(noLinkPattern) / sizeof(int));
            interrupts();
            patern = 0;
        };

        void ledNoDHCPIp() {
            if(patern == 2) {
                return;
            }

            noInterrupts();
            setPatternSingle((int*) noDHCPPattern, sizeof(noDHCPPattern) / sizeof(int));
            interrupts();
            patern = 2;
        };

        void ledWatchdog() {
            if(patern == 1) {
                return;
            }

            noInterrupts();
            setPatternSingle((int*) watchdogPattern, sizeof(watchdogPattern) / sizeof(int));
            interrupts();
            patern = 1;
        };


    private:
        int patern = -1;
        static sllibMod* self;
        IntervalTimer ledTask;
        static  void ledISR(void) {
            self->update();
            // volatile static bool ones = true;
            //  hang_counter += 1;

            // if(hang_counter > 20) {
            //  if(!ones) {
            //     return;
            // }

            // ones = false;
            // Task* pcurrentTask = pts->getCurrentTask();
            // char* taskname = pcurrentTask->getTaskName().c_str();
            // Serial1.print("task ID ");
            // Serial1.println(pcurrentTask->getId());
            // }

        }
};

#endif
