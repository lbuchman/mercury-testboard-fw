/*singleLEDLibrary
* A library for non interupting lighting effects for single LED's
* Pim Ostendorf - 2017.11.24
*/

#ifndef singleLEDLibrary_h
#define singleLEDLibrary_h
#include "Arduino.h"


class sllib {
    public:
        //public variables and fucntions
        sllib(int pin, bool _invertLed = false, bool pwmPin = false);

        //breathing
        void breathSingle(int speed);

        //blink
        void blinkSingle(int speed);
        void blinkSingle(int timeHigh, int timeLow);
        void blinkRandomSingle(int minTime, int maxTime);

        //flicker
        void flickerSingle();
        void flickerSingle(int intMin, int intMax);
        void flickerSingle(int intMin, int intMax, int speed);

        //blink pattern
        void patternSingle(int pattern[], int speed);

        //future update function
        void update();
        void setPatternSingle(int pattern[], int lenghtarray);
        void setBreathSingle(int speed);
        void setFlickerSingle();
        void setBlinkSingle(int speed);
        void setRandomBlinkSingle(int minTime, int maxTime);
        void setOffSingle();
        void seOnSingle() {
            runningFunction = 0;
            setOn();
        }
        void setMaxPwm(int pwm) {
            maxPWM = pwm;
        }

    private:
        //private variables and fucntion
        //gobal variables
        int _pin;
        bool invertLed;
        unsigned long milOld = 0;
        int runningFunction = 0;
        int speedp = 0;
        int timep = 0;
        //int pPatt[];
        int* arrP = 0;

        //variables for blinking
        bool ioBlink = false;
        int rndTemp = 0;
        int maxPWM = 10;
        bool _pwmPin = false;;

        //variables for pattern
        int counter = 0;

        void setOn() {
            if(_pwmPin) {
                uint8_t pwm_;

                if(invertLed) {
                    pwm_ = 0;
                }
                else {
                    pwm_ = maxPWM;
                }

                analogWrite(_pin, pwm_);
            }
            else {
                if(!invertLed) {
                    digitalWrite(_pin, HIGH);
                }
                else {
                    digitalWrite(_pin, LOW);
                }
            }
        };
        void setOff() {
            if(_pwmPin) {
                int8_t pwm_ = maxPWM;

                if(invertLed) {
                    pwm_ = maxPWM;
                }
                else {
                    pwm_ = 0;
                }

                analogWrite(_pin, pwm_);
            }
            else {
                if(!invertLed) {
                    digitalWrite(_pin, LOW);
                }
                else {
                    digitalWrite(_pin, HIGH);
                }
            }
        };
};

#endif
