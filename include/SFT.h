#ifndef SFT_h
#define SFT_h

#ifdef USE_SFT

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

#else

#define InitSFT()
#define DoSFTService()

#endif // USE_SFT
#endif // SFT_h