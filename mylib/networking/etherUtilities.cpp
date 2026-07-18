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

#include <etherUtilities.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


String macToString(uint8_t* mac) {
    static char strmac[32];
    sprintf(strmac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(strmac);
}

String ipToString(IPAddress ip, char *strip) {
    sprintf(strip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return strip;
}

String ipToString(IPAddress ip) {
    static char strip[64];
    sprintf(strip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return strip;
}

String ipToString(uint8_t* ip, char *strip) {
    sprintf(strip, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return strip;
}

uint8_t* str2mac(char *str) {
    static uint8_t mac[6];
    char buffer[20];
    strcpy(buffer, str);
    const char* delims = ":";
    char* tok = strtok(buffer, delims);

    for(int i = 0; i < 6; i++) {
        mac[i] = strtol(tok, NULL, 16);
        tok = strtok(NULL, delims);
    }

    return mac;
}

IPAddress str2Ip(char *str) {
    IPAddress ip;
    const char* delims = ".";
    char* tok = strtok(str, delims);

    for(int i = 0; i < 4; i++) {
        if(!tok) {
            return ip;
        }

        ip[i] = strtol(tok, NULL, 10);
        tok = strtok(NULL, delims);
    }

    return ip;
}

void ipAddressToBCD(IPAddress ip, uint8_t *bcd) {
    bcd[0] = ip[0];
    bcd[1] = ip[1];
    bcd[2] = ip[2];
    bcd[3] = ip[3];
}

uint32_t Swap(uint32_t value) {
    uint8_t *p = (uint8_t*) &value;
    return p[3] | (p[2] << 8) | (p[1] << 16) | (p[0] << 24);
}

IPAddress bytesToIpaddress(uint8_t *bytes) {
    IPAddress ip;
    ip[0] = bytes[0];
    ip[1] = bytes[1];
    ip[2] = bytes[2];
    ip[3] = bytes[3];
    return ip;
}

String linkStatus2Str(EthernetLinkStatus status) {
    switch(status) {
        case Unknown:
            return  "Unknown";

        case LinkON:
            return "LinkON";

        case LinkOFF:
            return  "LinkOFF";

        default:
            return "N/A";
    }
}

String ethHwStatus2Str(EthernetHardwareStatus status) {
    switch(status) {
        case EthernetNoHardware:
            return "EthernetNoHardware";

        case EthernetW5100:
            return "EthernetW5100";

        case EthernetW5200:
            return "EthernetW5200";

        case EthernetW5500:
            return "EthernetW5500";

        default:
            return "not known";
    }
}
