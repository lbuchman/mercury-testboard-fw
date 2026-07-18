# f2fSimPanel



cmake -DCMAKE_TOOLCHAIN_FILE=../cross/arm-teensy41-gnueabihf.cmake  -DCMAKE_BUILD_TYPE=Release ../


Make sure to change Eth buffer size

   #define FNET_SOCKET_DEFAULT_SIZE 1024 * 4

   #define MAX_SOCK_NUM 16

in NativeEthernet.h

