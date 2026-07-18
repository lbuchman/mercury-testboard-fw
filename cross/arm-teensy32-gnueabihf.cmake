# this one is important

#install cross compile arm for every arch
# make sure prebuild all additional libs and place .h files in sysroot which is  like  /opt/cross-pi-gcc-9.3.0-64/aarch64-linux-gnu/libc/usr/include


#do not define CMAKE_SYSROOT, what will ahppen if include is taken fro sysroot it will brake 64 bit gcc because it will take gcc .h from the board not from the local gcc dir
#sudo apt-get install gdb-multiarch
#gdb-multiarch  #make sure to do stepi in gdp before doing c or the program will not run
#gdbserver --multi mysticlakelinux:2001

#cmake -DRPI2=1 -DCMAKE_TOOLCHAIN_FILE=../arm-linux-gnueabihf.cmake ..

CMAKE_MINIMUM_REQUIRED(VERSION 3.20) 

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)# Define the cross compiler locations


SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_CROSSCOMPILING 1)
get_filename_component(ARDUINO_INST "~/arduino-ide" REALPATH BASE_DIR)

SET(TOOLSPATH ${ARDUINO_INST}/hardware/tools)
SET(COMPILERPATH  ${TOOLSPATH}/arm/bin)
SET(CROSS ${COMPILERPATH}/arm-none-eabi)
#SET(CMAKE_SYSROOT, ${TOOLSPATH}/arm)
#SET(TEENSY_ROOT ${TOOLSPATH}/arm)
SET (CORE_PATH ${ARDUINO_INST}/hardware/teensy/avr/cores/teensy3)


SET(MCU MK20DX256)
SET(MCU_LOADER TEENSY32)
SET(MCU_LD ${CORE_PATH}/mk20dx256.ld)
SET(MCU_DEF ARDUINO_TEENSY32)
SET(teensyStaticLib teensy32)

SET(OPTIONS -D__${MCU}__ -DARDUINO=10805 -DTEENSYDUINO=144 -DF_CPU=48000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH -D${MCU_DEF})
SET(CPUOPTIONS  -mcpu=cortex-m4 -mthumb)



SET(CPPFLAGS ${CPUOPTIONS} ${OPTIONS} -ffunction-sections -fdata-sections -DHW=32 -I${CORE_PATH})
SET(CXXFLAGS ${CPPFLAGS} -felide-constructors -fno-exceptions -fpermissive -fno-rtti -Wno-error=narrowing)

# linker options
SET(LDFLAGS -Wl,--gc-sections,--defsym=__rtc_localtime=0 ${SPECS} ${CPUOPTIONS} -T${MCU_LD} --specs=nano.specs)

#without this cmake will not pass test compile
SET(LIBS -l${teensyStaticLib})



add_compile_options(
    "$<$<COMPILE_LANGUAGE:C>:${CPPFLAGS}>"
    "$<$<COMPILE_LANGUAGE:CXX>:${CXXFLAGS}>"
)

add_link_options(${LDFLAGS} ${LIBS})

SET(OBJCOPY ${CROSS}-objcopy) 
set(SIZE  ${CROSS}-size) 
SET(CMAKE_C_COMPILER ${CROSS}-gcc)
SET(CMAKE_CXX_COMPILER ${CROSS}-gcc)
# Define the sysroot path for the RaspberryPi distribution in our tools folder 
SET(CMAKE_FIND_ROOT_PATH ${TEENSY_ROOT}/)# 
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories only
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
