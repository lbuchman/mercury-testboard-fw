#!/bin/sh

set -e
if [ -z $1 ]; then
   echo must specify teensy to build libs for, teensy3 or teensy4 and version 2, 6 for teensy 3 and 0,1 for teensy 4
   exit 0;
fi
if [ -z $2 ]; then
   echo must specify teensy to build libs for, teensy3 or teensy4 and version 2, 6 for teensy 3 and 0,1 for teensy 4
   exit 0;
fi
teensyType=$1
teensyVersion=$2
ROOT_DIR=$PWD
ARDUINO_INST=$(realpath ~/arduino-ide)
COREPATH=${ARDUINO_INST}/hardware/teensy/avr/cores/${teensyType}

if [ -f "${COREPATH}/Makefile.orig" ]; then
    cp -f ${COREPATH}/Makefile.orig ${COREPATH}/Makefile 
else
    cp -f ${COREPATH}/Makefile ${COREPATH}/Makefile.orig 
fi

if [ ! -f "${COREPATH}/main.cpp.orig" ]; then
    cp -f ${COREPATH}/main.cpp ${COREPATH}/main.cpp.orig
fi


set +e
rm ${COREPATH}/main.cpp 
set -e

sed -i '/^OBJS :=*/a $(TARGET).a: $(OBJS) $(MCU_LD)\n\t$(AR) rcs $@ $^' ${COREPATH}/Makefile
AR=${ARDUINO_INST}/hardware/tools/arm/bin/arm-none-eabi-gcc-ar

	
if [ "$teensyType" = "teensy3" ]; then

    cd $COREPATH
    

    if [ ! -f "${COREPATH}/pins_teensy.c.orig" ]; then
        cp -f ${COREPATH}/pins_teensy.c ${COREPATH}/pins_teensy.c.orig
    fi
    sed -i 's$delay(TEENSY_INIT_USB_DELAY_BEFORE)$delay(10)$g' ${COREPATH}/pins_teensy.c
    sed -i 's$delay(TEENSY_INIT_USB_DELAY_AFTER)$delay(10)$g' ${COREPATH}/pins_teensy.c
    if [ "$teensyVersion" = 2 ]; then
        MCU=MK20DX256
        sed -i '/^OPTIONS =/ s/$/ -DSERIAL1_RX_BUFFER_SIZE=1024 -DSERIAL1_TX_BUFFER_SIZE=1024/' ${COREPATH}/Makefile
        rm -f main.a
        sed -i 's$F_CPU=48000000$F_CPU=96000000$g' ${COREPATH}/Makefile
        make MCU=$MCU clean
        make MCU=$MCU main.a
        mv main.a ${ARDUINO_INST}/hardware/tools/arm/arm-none-eabi/lib/armv7e-m/libteensy3${2}.a
        exit 0
    fi

    if [ "$teensyVersion" = 6 ]; then
        MCU=MK66FX1M0
        sed -i '/^OPTIONS =/ s/$/ -DSERIAL1_RX_BUFFER_SIZE=1024 -DSERIAL1_TX_BUFFER_SIZE=1024 -fsingle-precision-constant -mfloat-abi=hard -mfpu=fpv4-sp-d16/' ${COREPATH}/Makefile
        sed -i 's$F_CPU=48000000$F_CPU=180000000$g' ${COREPATH}/Makefile
        rm -f main.a
        make MCU=$MCU main.a clean
        make MCU=$MCU main.a
        mv main.a ${ARDUINO_INST}/hardware/tools/arm/arm-none-eabi/lib/armv7e-m/fpu/libteensy3${2}.a
        exit 0
    fi
    cp ${COREPATH}/main.cpp.orig ${COREPATH}/main.cpp
    cp ${COREPATH}/pins_teensy.c.orig  ${COREPATH}/pins_teensy.c
fi

if [ "$teensyType" = "teensy4" ]; then

    GCC_LIB=/home/lbuchman/arduino-1.8.19/hardware/tools/arm/lib/gcc/arm-none-eabi/11.3.1/thumb/v7e-m+dp/hard
    cd $COREPATH
    sed -i '/^CPPFLAGS =/ s/$/ -DSERIAL1_TX_BUFFER_SIZE=256 -DSERIAL1_RX_BUFFER_SIZE=256 -DSERIAL2_TX_BUFFER_SIZE=256 -DSERIAL2_RX_BUFFER_SIZE=256 -DSERIAL7_TX_BUFFER_SIZE=256 -DSERIAL7_RX_BUFFER_SIZE=256/' ${COREPATH}/Makefile

    if [ "$teensyVersion" = 0 ]; then
        rm -f main.a
        MCU=IMXRT1062
        MCU_LD=imxrt1062.ld
        MCU_DEF=ARDUINO_TEENSY40
        make MCU=$MCU MCU_LD=${MCU_LD} MCU_DEF=${MCU_DEF} clean
        make MCU=$MCU MCU_LD=${MCU_LD} MCU_DEF=${MCU_DEF} main.a
        mkdir -p build
        mv main.a ${GCC_LIB}/libteensy4${2}.a
        exit 0
    fi

    if [ "$teensyVersion" = 1 ]; then
        rm -f main.a
        MCU=IMXRT1062
        MCU_LD=imxrt1062_t41.ld
        MCU_DEF=ARDUINO_TEENSY41
        make MCU=$MCU MCU_LD=${MCU_LD} MCU_DEF=${MCU_DEF} clean
        make MCU=$MCU MCU_LD=${MCU_LD} MCU_DEF=${MCU_DEF} main.a
        mv main.a ${GCC_LIB}/libteensy4${2}.a
    fi
    exit 0
fi

echo "Invalid parameters. Shall be teensy3(4) 0,1,2,6"
