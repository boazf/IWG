#ifndef SFT_h
#define SFT_h

#include <Common.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <SPI.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <Ethernet.h>
#endif

class SFT
{
public:
    static void Init();
    static void DoService();

private:
    static EthernetServer server;
#ifndef ESP32
    static SdVolume vol;
    static SdFile curDir;
#endif
    static char curPath[MAX_PATH + 1];

private:
    static void Connect(EthernetClient &client);
    static void Upload(EthernetClient &client);
    static void Download(EthernetClient &client);
    static void ListDirectory(EthernetClient &client);
    static void ChangeDirectory(EthernetClient &client);
    static void MakeDirectory(EthernetClient &client);
    static void Delete(EthernetClient &client);
    static void RemoveDirectory(EthernetClient &client);
    static bool WaitForClient(EthernetClient &client);
};

void InitSFT();
void DoSFTService();

#endif // SFT_h