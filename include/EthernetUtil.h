#ifndef EthernetUtil_h
#define EthernetUtil_h

#include <Ethernet.h>

void InitEthernet();
void MaintainEthernet();
bool IsZeroIPAddress(const IPAddress &address);

#endif // EthernetUtil_h