#ifndef SSEController_h
#define SSEController_h

#include <Arduino.h>
#include <Controller.h>
#include <RecoveryControl.h>

struct ClientInfo
{
public:
    ClientInfo(const String &_id, EthernetClient &_client, IPAddress _remoteIP, word _remotePort, time_t _timeToDie) :
        id(_id),
        client(&_client),
        remoteIP(_remoteIP),
        remotePort(_remotePort),
        timeToDie(_timeToDie),
        waitingResponce(false)
    {
    }

    ClientInfo(const String &_id) :
        id(_id),
        client(NULL),
        remotePort(0),
        timeToDie(INT32_MAX),
        waitingResponce(false)
    {
    }

    const String id;
    EthernetClient *client;
    IPAddress remoteIP;
    word remotePort;
    time_t timeToDie;
    bool waitingResponce;
};

class SSEController : public Controller
{
public:
    SSEController() : Controller("SSE")
    {
    }

    bool Get(EthernetClient &client, String &resource);
    bool Post(EthernetClient &client, String &resource);
    void Init();
    void Maintain();
    void DeleteClient(const IPAddress &remoteIP, word remotePort);
    bool IsValidId(const String &id);
    void AddClient(const String &id);

private:
    static void RecoveryStateChanged(const RecoveryStateChangedParams &params, const void *context);
    static void AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context);
    static void ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    static void RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    void NotifyState();
    void UpdateStateLastRecoveryTime();
    void DeleteClient(ListNode<ClientInfo*> *&clientInfo);

private:
    static int id;
    LinkedList<ClientInfo *> clients;

    class SSEControllerState
    {
    public:
        bool autoRecovery;
        RecoveryTypes recoveryType;
        PowerState modemState;
        PowerState routerState;
        bool showLastRecovery;
        int days;
        int hours;
        int minutes;
        int seconds;
    };

    SSEControllerState state;
};

extern SSEController sseController;

#endif // SSEController_h