#ifndef SSEController_h
#define SSEController_h

#include <Arduino.h>
#include <Controller.h>
#include <RecoveryControl.h>

struct ClientInfo
{
public:
    ClientInfo(const String &_id, EthClient &_client) :
        id(_id),
        client(_client)
    {
    }

    ClientInfo(const String &_id) :
        id(_id)
    {
    }

    ClientInfo(const ClientInfo &clientInfo)
    {
        *this = clientInfo;
    }

    ClientInfo &operator=(const ClientInfo &clientInfo)
    {
        *const_cast<String *>(&id) = clientInfo.id;
        client = clientInfo.client;
        return *this;
    }

    bool operator==(const ClientInfo &c) const
    {
        return id == c.id;
    }

    const String id;
    EthClient client;
};

class SSEController : public Controller
{
public:
    SSEController() : Controller("SSE")
    {
    }

    bool Get(EthClient &client, String &resource, ControllerContext &context);
    bool Post(EthClient &client, String &resource, ControllerContext &context);
    bool Put(EthClient &client, String &resource, ControllerContext &context);
    bool Delete(EthClient &client, String &resource, ControllerContext &context);
    void Init();
    bool DeleteClient(EthClient &client, bool stopClient);
    bool IsValidId(const String &id);
    void AddClient(const String &id);

private:
    static void RecoveryStateChanged(const RecoveryStateChangedParams &params, const void *context);
    static void AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context);
    static void ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    static void RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context);
    void NotifyState(const String &id);
    void UpdateStateLastRecoveryTime();
    void DeleteClient(const ClientInfo &clientInfo, bool stopClient);
    void DeleteUnusedClients();

private:
    //static int id;
    typedef LinkedList<ClientInfo> ClientsList;
    ClientsList clients;

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