#include <SSEController.h>
#include <Relays.h>
#include <TimeUtil.h>

#define SESSION_LENGTH 300

bool SSEController::Get(EthClient &client, String &id)
{
#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Trace("SSEController Get, Client id=");
        Trace(id);
#ifndef USE_WIFI
        Trace(", socket=");
        Traceln(client.getSocketNumber());
#else
        Traceln();
#endif
    }
#endif


    ListNode<ClientInfo *> *clientInfo = clients.head;
    while (clientInfo != NULL)
    {
        if (clientInfo->value->id.equals(id))
        {
            DeleteClient(clientInfo, true);
            break;
        }
        clientInfo = clientInfo->next;
    }
#ifdef DEBUG_HTTP_SERVER
    Tracef("Adding SSE client: id=%s, IP=%s, port=%d, object=%lx\n", id.c_str(), client.remoteIP().toString().c_str(), client.remotePort(), (ulong)&client);
#endif
    clients.Insert(new ClientInfo(id, client, t_now + SESSION_LENGTH));
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/event-stream");
    client.println("Connection: keep-alive");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    //client.println("Transfer-Encoding: chunked");
    client.println();
    client.flush();

    NotifyState(id);

    return true;
}

bool SSEController::Post(EthClient &client, String &resource, size_t contentLength, String contentType)
{
    return false;
}

bool SSEController::Put(EthClient &client, String &resource)
{
    return false;
}

bool SSEController::Delete(EthClient &client, String &resource)
{
    return false;
}

void SSEController::NotifyState(const String &id)
{
    UpdateStateLastRecoveryTime();

    String event("data:{");
    event += "\"autoRecovery\": ";
    event += state.autoRecovery ? "true" : "false";
    event += ", \"modemState\": ";
    event += state.modemState;
    event += ", \"routerState\": ";
    event += state.routerState;
    event += ", \"recoveryType\": ";
    event += state.recoveryType;
    event += ", \"showLastRecovery\": ";
    event += state.showLastRecovery ? "true" : "false";
    event += ", \"days\": ";
    event += state.days;
    event += ", \"hours\": ";
    event += state.hours;
    event += ", \"minutes\": ";
    event += state.minutes;
    event += ", \"seconds\": ";
    event += state.seconds;
    event += "}\n";
#ifdef DEBUG_HTTP_SERVER
    Trace(event);
#endif

    for (ListNode<ClientInfo *> *clientInfo = clients.head; clientInfo != NULL; clientInfo = clientInfo->next)
    {
        if (!id.equals("") && !id.equals(clientInfo->value->id))
            continue;
        EthClient *client = clientInfo->value->client;
        if (client == NULL)
            continue;
#ifdef DEBUG_HTTP_SERVER
        {
            LOCK_TRACE();
            Trace("Notifying client id=");
            Trace(clientInfo->value->id);
#ifndef USE_WIFI
            Trace(", socket=");
            Traceln(client->getSocketNumber());
#else
            Traceln();
#endif
        }
#endif
        client->print(event);
        client->println();
        client->flush();
        if (!id.equals(""))
            break;
    }
}

void SSEController::Init()
{
    state.autoRecovery = recoveryControl.GetAutoRecovery();
    state.recoveryType = recoveryControl.GetRecoveryState();
    state.modemState = GetModemPowerState();
    state.routerState = GetRouterPowerState();
    UpdateStateLastRecoveryTime();
    recoveryControl.GetRecoveryStateChanged().addObserver(RecoveryStateChanged, this);
    recoveryControl.GetAutoRecoveryStateChanged().addObserver(AutoRecoveryStateChanged, this);
    recoveryControl.GetModemPowerStateChanged().addObserver(ModemPowerStateChanged, this);
    recoveryControl.GetRouterPowerStateChanged().addObserver(RouterPowerStateChanged, this);
}

void SSEController::UpdateStateLastRecoveryTime()
{
    time_t lastRecovery = recoveryControl.GetLastRecovery();
    state.showLastRecovery = lastRecovery != INT32_MAX && recoveryControl.GetRecoveryState() == NoRecovery;
    if (state.showLastRecovery)
    {
        time_t timeSinceLastRecovery = t_now - lastRecovery;
        state.seconds = timeSinceLastRecovery % 60;
        state.minutes = (timeSinceLastRecovery / 60) % 60;
        state.hours = (timeSinceLastRecovery / 3600) % 24;
        state.days = (timeSinceLastRecovery / 3600 / 24);
    }
}

void SSEController::RecoveryStateChanged(const RecoveryStateChangedParams &params, const void *context)
{
    SSEController *controller = (SSEController *)context;
    controller->state.recoveryType = params.m_recoveryType;
    controller->UpdateStateLastRecoveryTime();
    controller->NotifyState("");
}

void SSEController::AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context)
{
#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Trace("AutoRecoveryStateChanged: ");
        Traceln(params.m_autoRecovery);
    }
#endif
    SSEController *controller = (SSEController *)context;
    controller->state.autoRecovery = params.m_autoRecovery;
    controller->NotifyState("");
}

void SSEController::ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = (SSEController *)context;
    controller->state.modemState = params.m_state;
    controller->NotifyState("");
}

void SSEController::RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = (SSEController *)context;
    controller->state.routerState = params.m_state;
    controller->NotifyState("");
}

bool SSEController::DeleteClient(EthClient &client, bool stopClient)
{
    for (ListNode<ClientInfo *> *clientInfo = clients.head; clientInfo != NULL; clientInfo = clientInfo->next)
    {
        if (clientInfo->value->client == &client)
        {
            DeleteClient(clientInfo, stopClient);
            return true;
        }
    }

    return false;
}

void SSEController::DeleteClient(ListNode<ClientInfo *> *&clientInfo, bool stopClient)
{
#ifdef DEBUG_HTTP_SERVER
    if (clientInfo->value->client != NULL)
    {
        LOCK_TRACE();
        Trace("Deleting previous session id=");
        Trace(clientInfo->value->id);
#ifndef USE_WIFI
        Trace(", socket=");
        Traceln(clientInfo->value->client->getSocketNumber());
#else
        Traceln();
#endif
    }
#endif
    if (clientInfo->value->client != NULL && stopClient)
    {
        clientInfo->value->client->stop();
    }
    clientInfo->value->client = NULL;
    delete clientInfo->value;
    clientInfo->value = NULL;
    clientInfo = clients.Delete(clientInfo);
}

bool SSEController::IsValidId(const String &id)
{
    bool isValid = false;
    
    for (ListNode<ClientInfo *> *clientInfo = clients.head; clientInfo != NULL; clientInfo = clientInfo->next)
    {
        isValid = clientInfo->value->id.equals(id);
        if (isValid)
            break;
    }

    return isValid;
}

void SSEController::AddClient(const String &id)
{
    if (IsValidId(id))
        return;

    clients.Insert(new ClientInfo(id));
}

SSEController sseController;