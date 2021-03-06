#ifndef USE_WIFI
#ifndef NTPClient_h
#define NTPClient_h

#include <EthernetUtil.h>
#include <time.h>
#include <Common.h>

class NTPClient
{
public:
    static time_t getUTC();

private:
    static const unsigned int localPort;  // local port to listen for UDP packets
    static const int NTP_PACKET_SIZE;     // NTP time stamp is in the first 48 bytes of the message
    static byte packetBuffer[];           //buffer to hold incoming and outgoing packets
    static EthUDP Udp;               // A UDP instance to let us send and receive packets over UDP
};

#endif // NTPClient_h
#endif // USE_WIFI