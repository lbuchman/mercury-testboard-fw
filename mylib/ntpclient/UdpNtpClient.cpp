/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi

 This code is in the public domain.

 */
#include <watchdog.h>
#include <logger.hpp>
#include <etherUtilities.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <persistent.hpp>
#include <TaskSchedulerDeclarations.h>
// #include <boardConfig.hpp>
#include <UdpNtpClient.hpp>
#include <shellFunctor.hpp>
#include <utility.h>

extern Scheduler ts;

void sendNTPpacket(IPAddress address);
unsigned int localPort = 5871;       // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
static EthernetUDP Udp;
static bool ntpStatus = false;
static IPAddress timeServerIp;

shellFunc epoch = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(!checkArgument(1, arg_cnt, args, (char*) "\t{ \"cmd\": \"%s\", \"desc\": \"display apoch time\" },\n\r", stream)) {
        return 1;
    }

    stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"epoch\": \"%d\"}\n\r", args[0], now());
    return 1;
};


shellFunc uptime = [](int arg_cnt, char **args, Stream & stream) -> int {
    if(arg_cnt == 0xFF) {
        stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"display system uptime\" },\n\r", args[0]);
        return 1;
    }

    long day = 86400000; // 86400000 milliseconds in a day
    long hour = 3600000; // 3600000 milliseconds in an hour
    long minute = 60000; // 60000 milliseconds in a minute
    long second =  1000; // 1000 milliseconds in a second
    long timeNow = millis();

    int days = timeNow / day ;                                //number of days
    int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
    int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
    int seconds = (((timeNow % day) % hour) % minute) / second;
    stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"days\": %d, \"hours\": %2d, \"minutes\": %2d,  \"seconds\": %2d }\n\r",  args[0], days, hours, minutes, seconds);
    return 1;
};

shellFunc date = [](int arg_cnt, char **args, Stream & stream) -> int {
    (void)arg_cnt;

    if(arg_cnt == 0xFF) {
        stream.printf("\t{ \"cmd\": \"%s\", \"desc\": \"display system date and time\" },\n\r", args[0]);
        return 1;
    }

    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "%s %02d %02d:%02d:%02d UTC %d", monthShortStr(month()), day(), hour(), minute(), second(), year());
    stream.printf("\t{ \"cmd\": \"%s\", \"status\": true, \"year\": %d, \"day\": %02d, \"month\": %02d, \"hour\": %02d, \"minute\": %02d,  \"second\": %02d }\n\r", args[0], year(), day(), month(), hour(), minute(), second());
    return 1;
};

/*
 *
 */
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress address) {
    logger().info(logger().printHeader, (char*) __FILE__, __LINE__,  "sending NTP packet to %s", ipToString(address).c_str());
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;


    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); // NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}


void ntpInit() {
    TNetworkConfig netConfig = getNetworkConfig();

    static Task ntpTask(TASK_IMMEDIATE, TASK_ONCE, std::function<void()> ([](void) -> void {
        int ret = 0;

        if(!(ret = Udp.parsePacket())) {
            if(!ntpStatus) {
                logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "NTP failed to get reply");
            }

            sendNTPpacket(timeServerIp); // send an NTP packet to a time server
            ntpTask.enableDelayed(TASK_SECOND * 4);
            ntpTask.restartDelayed(TASK_SECOND * 4);
            return;
        }

        logger().debug(logger().printHeader, (char*) __FILE__, __LINE__,  "received %d bytes data", ret);

        // We've received a packet, read the data from it
        if(ret != NTP_PACKET_SIZE) {
            logger().debug(logger().printHeader, (char*) __FILE__, __LINE__,  "ret  %d != NTP_PACKET_SIZE = 48", ret);
            ntpTask.enableDelayed(TASK_SECOND * 4);
            ntpTask.restartDelayed(TASK_SECOND * 4);
            return;
        }

        ret = Udp.read(packetBuffer, ret); // read the packet into the buffer
        logger().debug(logger().printHeader, (char*) __FILE__, __LINE__,  "read ret %d bytes", ret);
        // the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, extract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        unsigned long epoch = secsSince1900 - seventyYears;
        setTime(epoch);
        Teensy3Clock.set(epoch);
        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "NTP reply is received");
        logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "%s %02d %02d:%02d:%02d UTC %d", monthShortStr(month()), day(), hour(), minute(), second(), year());
        ntpTask.enableDelayed(TASK_HOUR * 24 * 30);
        ntpTask.restartDelayed(TASK_HOUR * 24 * 30);
        ntpStatus = true;
    }), &ts, false);

    logger().info(logger().printHeader, (char*) __FILE__, __LINE__, "Intializing NTP timeserver: %s", ipToString(netConfig.ntpServer).c_str());

    IPAddress ip = bytesToIpaddress(netConfig.ntpServer);
    Udp.begin(localPort);
    ntpStatus = false;
    timeServerIp = ip;
    sendNTPpacket(timeServerIp); // send an NTP packet to a time server
    ntpTask.enableDelayed(TASK_SECOND * 4);
    ntpTask.restartDelayed(TASK_SECOND * 4);
    ShellFunctor::getInstance().add("date", date);
    ShellFunctor::getInstance().add("uptime", uptime);
}


String epochToString(time_t t) {
    const char *str = ctime(&t);
    return str;
}

