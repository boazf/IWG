#ifndef SSEController_h
#define SSEController_h

#include <Arduino.h>
#include <Controller.h>
#include <RecoveryControl.h>

struct ClientInfo
{
public:
    ClientInfo(const String &_id, EthernetClient &_client, time_t _timeToDie) :
        id(_id),
        client(&_client)
    {
    }

    ClientInfo(const String &_id) :
        id(_id),
        client(NULL)
    {
    }

    const String id;
    EthernetClient *client;
};

class SSEController : public Controller
{
public:
    SSEController() : Controller("SSE")
    {
    }

    bool Get(EthernetClient &client, String &resource);
    bool Post(EthernetClient &client, String &resource, size_t contentLength, String contentType);
    bool Put(EthernetClient &client, String &resource);
    bool Delete(EthernetClient &client, String &resource);
    void Init();
    bool DeleteClient(EthernetClient &client, bool stopClient);
    bool IsValidId(const String &id);
    void AddClient(const String &id);

private:
    static void RecoveryStateChanged(const RecoveryStateChangedParams &params, const void *context);
    static void AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context);
    static void ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    static void RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    void NotifyState(const String &id);
    void UpdateStateLastRecoveryTime();
    void DeleteClient(ListNode<ClientInfo*> *&clientInfo, bool stopClient);

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