#include <Arduino.h>
#include <NativeEthernet.h>
#include <TaskSchedulerDeclarations.h>
#include <etherUtilities.h>
#include <hw.h>
#include <logger.hpp>
#include <networking.hpp>
#include <persistent.hpp>
#include <shellFunctor.hpp>
#include <singleLEDLibraryMod.h>
#include <utility.h>
#include <watchdogWrapper.h>

#define CcnfigNetPin 35

bool startNetwork(unsigned long timeout, unsigned long responseTimeout, sllibMod& watchDogLed);

shellFunc ifconfig = [](int arg_cnt, char** args, Stream& stream) {
    if (!checkArgument(1, arg_cnt, args, (char*)"\t{ \"cmd\": \"%s\", \"desc\": \"returns board IP address\" }", stream)) {
        return 1;
    }

    TNetworkConfig netConfig = getNetworkConfig();
    stream.printf("\t{ \"cmd\": \"%s\", \"status\":true, \"ip\" : \"%s\", \"GW\": \"%s\", \"DNS\": \"%s\",  \"MAC\": \"%s\" }\n\r", args[0],
                  ipToString(Ethernet.localIP()).c_str(), ipToString(Ethernet.gatewayIP()).c_str(),
                  ipToString(Ethernet.dnsServerIP()).c_str(), macToString(netConfig.mac).c_str());
    return 1;
};

shellFunc setmac = [](int arg_cnt, char** args, Stream& stream) -> int {
    if (!checkArgument(2, arg_cnt, args,
                       (char*)"\t{ \"cmd\": \"%s\",  \"arg\": \"mac address hex string\", \"desc\": \"set the board Ethernet Mac address "
                              "the format is xx:xx:xx:xx:xx:xx\" }",
                       stream)) {
        return 1;
    }

    uint8_t* mac = str2mac(args[1]);

    if (!mac) {
        stream.printf("\t{\"cmd\": \"%s\", \"status\": false, \"error\": \"invalid MAC address\" }\n\r", args[0]);
        return 1;
    }

    TNetworkConfig netConfig = getNetworkConfig();
    memcpy(netConfig.mac, mac, 6);

    bool persisted = network2eeprom(netConfig);

    stream.printf("\t{\"cmd\": \"%s\", \"status\": %s, \"reboot_required\": %s }\n\r", args[0], persisted ? "true" : "false",
                  persisted ? "true" : "false");
    return 1;
};

void initNetworking(Scheduler& ts, sllibMod& watchDogLed, bool networkConfigValid) {
    pinMode(CcnfigNetPin, INPUT_PULLUP);
    static Task ethernetTask(TASK_SECOND * 3, TASK_FOREVER, std::function<void()>([&](void) -> void {
                                 static bool ethernetStatus = false;

                                 if (Ethernet.linkStatus() == LinkOFF) {
                                     watchDogLed.ledNoEtherlink();
                                     ethernetStatus = false;
                                     logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Ethernet cable is not connected.");
                                     return;
                                 }

                                 if (ethernetStatus) {
                                     watchDogLed.ledWatchdog();
                                     return;
                                 }

                                 watchDogLed.ledNoDHCPIp();
                                 ethernetStatus = startNetwork(4000, 3900, watchDogLed); // to prevent watchdog reset
                             }),
                             &ts, false);

    ShellFunctor::getInstance().add("setmac", setmac);

    if (!networkConfigValid) {
        watchDogLed.ledNoDHCPIp();
        logger().error(logger().printHeader, (char*)__FILE__, __LINE__,
                       "Invalid network EEPROM configuration; set a MAC address and reboot");
        return;
    }

    startNetwork(4000, 3900, watchDogLed); // to prevent watchdog reset
    ShellFunctor::getInstance().add("ifconfig", ifconfig);

    if (!digitalRead(CcnfigNetPin))
        ethernetTask.enable();
}

bool startNetwork(unsigned long timeout, unsigned long responseTimeout, sllibMod& watchDogLed) {
    TNetworkConfig netConfig = getNetworkConfig();

    if (netConfig.tag == -1) {
        logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Invalid network EEPROM config; generating random MAC");
        netConfig.mac[0] = 0x02;
        for (size_t i = 1; i < sizeof(netConfig.mac); ++i) {
            netConfig.mac[i] = (uint8_t)random(0x100);
        }
        netConfig.tag = netSettingsTag;
        network2eeprom(netConfig);
    }

    bool retStatus = false;

    logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Configuring ethernet adapter DHCP, MAC = %s ...",
                  macToString(netConfig.mac).c_str());

    if (digitalRead(CcnfigNetPin)) {
        IPAddress ip(192, 168, 0, 60);
        IPAddress myDns(192, 168, 0, 6);
        Ethernet.begin(netConfig.mac, ip, myDns);
        watchDogLed.ledWatchdog();
        return true;
    }

    if (Ethernet.begin(netConfig.mac, timeout, responseTimeout) == 0) {
        logger().info(logger().printHeader, (char*)__FILE__, __LINE__, "Failed to configure Ethernet using DHCP");
        retStatus = false;

        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            logger().info(logger().printHeader, (char*)__FILE__, __LINE__,
                          "Ethernet shield was not found.  Sorry, can't run without hardware. :(");

            while (true) {
                delay(1); // do nothing, no point running without Ethernet hardware
            }
        }
    } else {
        retStatus = true;
    }

    logger().info(logger().printHeader, __FILE__, __LINE__, "MAC address: %s", macToString(netConfig.mac).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "My IP address: %s", ipToString(Ethernet.localIP()).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "GW address: %s", ipToString(Ethernet.gatewayIP()).c_str());
    logger().info(logger().printHeader, __FILE__, __LINE__, "dns server address: %s", ipToString(Ethernet.dnsServerIP()).c_str());

    return retStatus;
}
