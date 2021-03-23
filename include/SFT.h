#ifndef SFT_h
#define SFT_h

#include <Common.h>
#include <SPI.h>
#include <SDUtil.h>
#include <EthernetUtil.h>

class SFT
{
public:
    static void Init();
    static void DoService();

private:
    static EthServer server;
#ifndef ESP32
    static SdVolume vol;
    static SdFile curDir;
#endif
    static char curPath[MAX_PATH + 1];

private:
    static void Connect(EthClient &client);
    static void Upload(EthClient &client);
    static void Download(EthClient &client);
    static void ListDirectory(EthClient &client);
    static void ChangeDirectory(EthClient &client);
    static void MakeDirectory(EthClient &client);
    static void Delete(EthClient &client);
    static void RemoveDirectory(EthClient &client);
    static bool WaitForClient(EthClient &client);
};

void InitSFT();
void DoSFTService();

#endif // SFT_h