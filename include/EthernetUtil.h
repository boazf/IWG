#ifndef EthernetUtil_h
#define EthernetUtil_h

#ifdef ESP32
#include <WiFi.h>
#else
#include <Ethernet.h>
#endif

#ifdef ESP32
#define Ethernet WiFi
#define EthernetClient WiFiClient
#define EthernetServer WiFiServer
#endif

void InitEthernet();
void MaintainEthernet();
bool IsZeroIPAddress(const IPAddress &address);

#endif // EthernetUtil_h