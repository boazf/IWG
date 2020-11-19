#include <SSEController.h>
#include <Relays.h>
#include <MemUtil.h>
#include <TimeUtil.h>

#define SESSION_LENGTH 300

bool SSEController::Get(EthernetClient &client, String &id)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.print("SSEController Get, Client id=");
    Serial.print(id);
    Serial.print(", socket=");
    Serial.println(client.getSocketNumber());
#endif


    ListNode<ClientInfo *> *clientInfo = clients.head;
    while (clientInfo != NULL)
    {
        if (clientInfo->value->id.equals(id))
        {
            DeleteClient(clientInfo);
            break;
        }
        clientInfo = clientInfo->next;
    }

    clients.Insert(new ClientInfo(id, client, client.getSocketNumber(), t + SESSION_LENGTH));
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/event-stream");
    client.println("Connection: keep-alive");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    //client.println("Transfer-Encoding: chunked");
    client.println();
    client.flush();

    NotifyState();

    return true;
}

bool SSEController::Post(EthernetClient &client, String &id)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.print("SSEController Post, Client id=");
    Serial.print(id);
    Serial.print(", socket=");
    Serial.println(client.getSocketNumber());
#endif
    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println("Content-Length: 0");
    client.println();
    client.flush();

    ListNode<ClientInfo *> *clientInfo = clients.head;

    while (clientInfo != NULL)
    {
        if (clientInfo->value->id == id)
        {
            clientInfo->value->timeToDie = t + SESSION_LENGTH;
            clientInfo->value->waitingResponce = false;
        }
        clientInfo = clientInfo->next;
    }

    return true;
}

void SSEController::NotifyState()
{
    TRACK_FREE_MEMORY(__func__);

    UpdateStateLastRecoveryTime();

    for (ListNode<ClientInfo *> *clientInfo = clients.head; clientInfo != NULL; clientInfo = clientInfo->next)
    {
        EthernetClient *client = clientInfo->value->client;
        if (client == NULL)
            continue;
#ifdef DEBUG_HTTP_SERVER
        Serial.print("Notifiying client id=");
        Serial.print(clientInfo->value->id);
        Serial.print(", socket=");
        Serial.println(client->getSocketNumber());
#endif
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
        Serial.println(event);
#endif
        client->print("Content-Length: ");
        client->println(event.length());
        client->println();
        client->print(event);
        client->println();
        client->flush();
        clientInfo->value->timeToDie = t + 5;
        clientInfo->value->waitingResponce = true;
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
    state.showLastRecovery = lastRecovery != UINT32_MAX && recoveryControl.GetRecoveryState() == NoRecovery;
    if (state.showLastRecovery)
    {
        time_t timeSinceLastRecovery = t - lastRecovery;
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
    controller->NotifyState();
}

void SSEController::AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.print("AutoRecoveryStateChanged: ");
    Serial.println(params.m_autoRecovery);
#endif
    SSEController *controller = (SSEController *)context;
    controller->state.autoRecovery = params.m_autoRecovery;
    controller->NotifyState();
}

void SSEController::ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = (SSEController *)context;
    controller->state.modemState = params.m_state;
    controller->NotifyState();
}

void SSEController::RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = (SSEController *)context;
    controller->state.routerState = params.m_state;
    controller->NotifyState();
}

void SSEController::Maintain()
{
    ListNode<ClientInfo *> *clientInfo = clients.head;

    while (clientInfo != NULL)
    {
        if (t >= clientInfo->value->timeToDie)
        {
#ifdef DEBUG_HTTP_SERVER
            Serial.print("Time to die client id=");
            Serial.print(clientInfo->value->id);
            Serial.print(", socket=");
            Serial.println(clientInfo->value->socket);
#endif
            if (!clientInfo->value->waitingResponce)
            {
                clientInfo->value->client->stop();
                clientInfo->value->client = NULL;
                clientInfo->value->socket = MAX_SOCK_NUM;
                clientInfo->value->timeToDie = t + 5;
                clientInfo->value->waitingResponce = true;
                continue;
            }
            DeleteClient(clientInfo);
            continue;
        }
        clientInfo = clientInfo->next;
    }
}

void SSEController::DeleteClient(SOCKET socket)
{
    ListNode<ClientInfo *> *clientInfo = clients.head;

    while (clientInfo != NULL)
    {
        if (clientInfo->value->socket == socket)
        {
            DeleteClient(clientInfo);
            break;
        }
        clientInfo = clientInfo->next;
    }
}

void SSEController::DeleteClient(ListNode<ClientInfo *> *&clientInfo)
{
#ifdef DEBUG_HTTP_SERVER
    if (clientInfo->value->client != NULL)
    {
        Serial.print("Deleting previous session id=");
        Serial.print(clientInfo->value->id);
        Serial.print(", socket=");
        Serial.println(clientInfo->value->client->getSocketNumber());
    }
#endif
    if (clientInfo->value->client != NULL)
        clientInfo->value->client->stop();
    delete clientInfo->value;
    clientInfo->value = NULL;
    clientInfo = clients.Delete(clientInfo);
}

bool SSEController::IsValidId(const String &id)
{
    bool isValid = false;

    ListNode<ClientInfo *> *clientInfo = clients.head;
    while (clientInfo != NULL)
    {
        isValid = clientInfo->value->id.equals(id);
        if (isValid)
            break;
        clientInfo = clientInfo->next;
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