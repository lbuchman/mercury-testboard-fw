#Copyright (c) Microsoft. All rights reserved.
#Lic`sed under the MIT license. See LICENSE file in the project root for full license information.

#cmake -DRPI2=1 -DCMAKE_TOOLCHAIN_FILE=../arm-linux-gnueabihf.cmake ..

cmake_minimum_required(VERSION 3.20)
set(PROJECT_NAME mnptester)

include(ExternalProject)
project(mnptester)

set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS -----------------------------Cross Compiling --------------------)

include_directories(./arduinoLib/TaskScheduler/src/)
include_directories(./arduinoLib/Time/)
include_directories(./arduinoLib/Ring-Buffer) 
include_directories(${ARDUINO_INST}/hardware/teensy/avr/libraries/FNET/src)
include_directories(./arduinoLib/Wdt_t4)
include_directories(${ARDUINO_INST}/hardware/teensy/avr/libraries/NativeEthernet/src)
include_directories(${ARDUINO_INST}/hardware/teensy/avr/libraries/EEPROM/)
include_directories(./arduinoLib/singleLEDLibrary)
include_directories(./arduinoLib/ArduinoJson/src/) 
include_directories(arduinoLib/CRCx/)

#//
include_directories(./mylib/)
include_directories(./mylib/logger)
include_directories(./mylib/ntpclient)
include_directories(./mylib/watchdog)
include_directories(./mylib/singleLEDLibraryMod)
include_directories(./mylib/networking) 
include_directories(./mylib/tcpip) 
#//

include_directories(./src/board)

#//////

file(GLOB_RECURSE NativeEthernet_lib ${ARDUINO_INST}/hardware/teensy/avr/libraries/NativeEthernet/src/*.cpp ${ARDUINO_INST}/hardware/teensy/avr/libraries/NativeEthernet/src/*.c)
file(GLOB_RECURSE eeprom_lib ${ARDUINO_INST}/hardware/teensy/avr/libraries/EEPROM/*.cpp)
file(GLOB_RECURSE arduinojson ./arduinoLib/ArduinoJson/*.cpp)
file(GLOB_RECURSE time_lib ./arduinoLib/Time/*.cpp)
file(GLOB_RECURSE watchdog_t4 ./arduinoLib/Wdt_t4/*.cpp)
file(GLOB_RECURSE fnet_lib ${ARDUINO_INST}/hardware/teensy/avr/libraries/FNET/src/*.cpp ${ARDUINO_INST}/hardware/teensy/avr/libraries/FNET/src/*.c)

#//
file(GLOB_RECURSE sslib ./arduinoLib/singleLEDLibrary*.cpp)
file(GLOB_RECURSE watchdog ./mylib/watchdog/*.cpp)
file(GLOB_RECURSE sslibmod ./mylib/singleLEDLibraryMod/*.cpp)
file(GLOB_RECURSE logger ./mylib/logger/*.cpp)
file(GLOB_RECURSE networking ./mylib/networking/*.cpp)
#//

file(GLOB_RECURSE board ./src/board/*.cpp)
file(GLOB_RECURSE watchdog ./mylib/*.cpp)
#/////

SET(MY_DEBUG_OPTIONS -s -Os -Wall  -I${CORE_PATH} -D_TASK_STD_FUNCTION -D_TASK_WDT_IDS)
SET(MY_RELEASE_OPTIONS  -s -Os -Wall -I${CORE_PATH} -D_TASK_STD_FUNCTION -D_TASK_WDT_IDS)
message(STATUS "Running cmake for Tensy ${MCU_DEF}")


add_executable(${PROJECT_NAME} ${time_lib} ${sslib} ${board} ${sslibmod} ${networking}  ${logger} ${fnet_lib} ${watchdog} ${eeprom_lib} ${arduinojson} ${watchdog_t4} ${NativeEthernet_lib} src/init.cpp)


target_link_libraries(${PROJECT_NAME}
     ${MATH_LIB}
      stdc++ 
      ${teensyStaticLib}
     m
)
 
install(CODE "MESSAGE(\"installing target\")") 
 
 
add_custom_target(format
    COMMAND
        ../format.sh
    VERBATIM
)

add_custom_target(lint
    COMMAND
        cpplint --quiet --counting=detailed --linelength=120 --recursive ../src/
    VERBATIM
)


add_custom_target(size
    COMMAND
        ${SIZE} ${PROJECT_NAME}
    VERBATIM
)

add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND
        ${OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${PROJECT_NAME} ${PROJECT_NAME}.hex
        
    VERBATIM
)

add_custom_target(flash
    COMMAND
        ${TOOLSPATH}/teensy_loader_cli  -mmcu=${MCU_LOADER} -w -s -v  ${PROJECT_NAME}.hex
        #${TOOLSPATH}/teensy_post_compile -board=${MCU_LOADER} -file=${PROJECT_NAME} -path=${CMAKE_BINARY_DIR} -tools=${TOOLSPATH} -reboot
    VERBATIM
)
 
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS YES
)
 
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${MY_DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${MY_RELEASE_OPTIONS}>")


