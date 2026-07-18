#include <Arduino.h>
#include <hw.h>
#include <TaskSchedulerDeclarations.h>
#include <NativeEthernet.h>
#include <watchdog.h>
#include <NativeEthernet.h>
#include <singleLEDLibraryMod.h>
#include <UdpNtpClient.hpp>
#include <etherUtilities.h>
#include <logger.hpp>
#include <persistent.hpp>
#include <networking.hpp>
#include <shellFunctor.hpp>
#include <utility.h>

#define CcnfigNetPin 35

static bool ethernetStatus = false;
bool startNetwork(unsigned long timeout, unsigned long responseTimeout, sllibMod& watchDogLed);

shellFunc ifconfig = [](int arg_cnt, char **args, Stream & stream) {
    if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"desc\": \"returns board IP address\" },\n\r", stream)) {
        return 1;
    }

    TNetworkConfig netConfig = getNetworkConfig();
    stream.printf("\t{ \"cmd\": \"%s\", \"status\":true, \"ip\" : \"%s\", \"GW\": \"%s\", \"DNS\": \"%s\",  \"MAC\": \"%s\" }\n\r", args[0], ipToString(Ethernet.localIP()).c_str(), ipToString(Ethernet.gatewayIP()).c_str(), ipToString(Ethernet.dnsServerIP()).c_str(), macToString(netConfig.mac).c_str());
    return 1;
};

shellFunc setmac = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\",  \"arg\": \"mac address hex string\", \"desc\": \"set the board Ethernet Mac address the format is xx:xx:xx:xx:xx:xx\" },\n\r", stream)) {
        return 1;
    }

    uint8_t* mac = str2mac(args[1]);
    TNetworkConfig netConfig = getNetworkConfig();
    memcpy(netConfig.mac, mac, 6);

    network2eeprom(netConfig);

    stream.printf("\t{\"cmd\": \"%s\", \"status\": true }\n\r", args[0]);
    return 1;
};


shellFunc setntpserver = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(2, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"arg\": \"ntpTimeServer ipaddress\", \"desc\": \"set ntpserver\" },\n\r", stream)) {
        return 1;
    }

    TNetworkConfig netConfig = getNetworkConfig();
    IPAddress ip;
    ip = str2Ip(args[1]);
    ipAddressToBCD(ip, netConfig.ntpServer);
    //Todo save configuration
    stream.printf("\t{ \"cmd\": \"%s\", \"status\": true}\n\r", args[0]);
    return 1;
};

time_t getTeensy3Time() {
    return Teensy3Clock.get();
};

void initNetworking(Scheduler& ts, sllibMod& watchDogLed) {
    pinMode(CcnfigNetPin, INPUT_PULLUP);
    static Task ethernetTask(TASK_SECOND * 3, TASK_FOREVER, std::function<void()>([&](void) -> void { //todo what about renew and ntp update
        static bool ethernetStatus = false;
        static bool runOnes = true;

        if(ethernetStatus && runOnes) {
            ntpInit();
            runOnes = false;
        }

        if(Ethernet.linkStatus() == LinkOFF) {
            watchDogLed.ledNoEtherlink();
            ethernetStatus = false;
            logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Ethernet cable is not connected.");
            return;
        }

        if(ethernetStatus) {
            watchDogLed.ledWatchdog();
            return;
        }

        watchDogLed.ledNoDHCPIp();
        ethernetStatus = startNetwork(4000, 3900, watchDogLed); // to prevent watchdog reset
    }), &ts, false);

    ethernetStatus = startNetwork(4000, 3900, watchDogLed); // to prevent watchdog reset
    ShellFunctor::getInstance().add("setntpserver", setntpserver);
    ShellFunctor::getInstance().add("ifconfig", ifconfig);
    ShellFunctor::getInstance().add("setmac", setmac);
    setSyncProvider(getTeensy3Time);
    setSyncInterval(24 * 3600);

    if (!digitalRead(CcnfigNetPin)) ethernetTask.enable();
}

bool startNetwork(unsigned long timeout, unsigned long responseTimeout, sllibMod& watchDogLed) {
    TNetworkConfig netConfig = getNetworkConfig();
    bool retStatus = false;

    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Configuring ethernet adapter DHCP, MAC = %s ...", macToString(netConfig.mac).c_str());

    if (digitalRead(CcnfigNetPin)) {
        IPAddress ip(192, 168, 0, 60);
        IPAddress myDns(192, 168, 0, 6);
        Ethernet.begin(netConfig.mac, ip, myDns);
        watchDogLed.ledWatchdog();
        return true;
    }

    if(Ethernet.begin(netConfig.mac, timeout, responseTimeout) == 0) {
        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Failed to configure Ethernet using DHCP");
        retStatus = false;

        // Check for Ethernet hardware present
        if(Ethernet.hardwareStatus() == EthernetNoHardware) {
            logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Ethernet shield was not found.  Sorry, can't run without hardware. :(");

            while(true) {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
    }
    else {
        retStatus = true;
    }

    logger().info(logger().printHeader, __FILE__, __LINE__, "MAC address: %s",  macToString(netConfig.mac).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "My IP address: %s", ipToString(Ethernet.localIP()).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "GW address: %s", ipToString(Ethernet.gatewayIP()).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "dns server address: %s", ipToString(Ethernet.dnsServerIP()).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "time server address: %s", ipToString(netConfig.ntpServer).c_str());

    return retStatus;
}
