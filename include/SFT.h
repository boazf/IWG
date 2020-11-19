#ifndef SFT_h
#define SFT_h

#include <SPI.h>
#include <Ethernet.h>
#include <Common.h>

class SFT
{
public:
    static void Init();
    static void DoService();

private:
    static EthernetServer server;
    static SdVolume vol;
    static SdFile curDir;
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
    static void ReadAppConfig(EthernetClient &client);
    static void WriteAppConfig(EthernetClient &client);
    static bool WaitForClient(EthernetClient &client);
};

void InitSFT();
void DoSFTService();

#endif // SFT_h