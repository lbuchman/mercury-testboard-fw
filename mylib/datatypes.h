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

#ifndef DATATYPES_H
#define DATATYPES_H

#include <Arduino.h>

enum class DeviceType : uint32_t {
    HumidityThermostat = 0,
    TemperatureThermostat = 1,
    NoneDevice            = 2
};

enum class IoType : uint8_t {
    input = 0,
    output = 1, // must be 1
    analogOutput = 2, // must be 2
    analogInput = 3
};

enum class IODefinition : uint32_t {
    DigitalInput1    = 0,
    DigitalInput2    = 1,
    DigitalInput3    = 2,
    DigitalInput4    = 3,
    DigitalInput5    = 4,
    DigitalInput6    = 5,
    DigitalInput7    = 6,
    DigitalInput8    = 7,
    DigitalInput9    = 8,
    DigitalInput10   = 9,
    AnalogOutput1    = 10,
    AnalogOutput2    = 11,
    AnalogOutput3    = 12,
    AnalogOutput4    = 13,
    AnalogOutput5    = 14,
    AnalogOutput6    = 15,
    AnalogOutput7    = 16,
    AnalogOutput8    = 17,
    DigitalOutput1   = 18,
    DigitalOutput2   = 19,
    DigitalOutput3   = 20,
    DigitalOutput4   = 21
};



#define ThermostatType2Int(x) static_cast<std::underlying_type_t<ThermostatType>>(x)
#define int2ThermostatType(x) static_cast<ThermostatType>(x)
#define IODefinition2Int(x) static_cast<std::underlying_type_t<IODefinition>>(x)
#define int2IODefinition(x) static_cast<IODefinition>(x)
#define IoType2Int(x) static_cast<std::underlying_type_t<IoType>>(x)
#define int2IoType(x) static_cast<IoType>(x)


const char * const BoolToString(bool b);
#endif
