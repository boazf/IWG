#include <SSEController.h>
#include <Relays.h>
#include <TimeUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool SSEController::Get(EthClient &client, String &id, ControllerContext &context)
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

    struct Params
    {
        SSEController *controller;
        String id;
    } params = { this, id };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = (Params *)param;
        String id = params->id;
        if (clientInfo.id.equals(id))
        {
            params->controller->DeleteClient(clientInfo, true);
            return false;
        }

        return true;
    }, &params);
#ifdef DEBUG_HTTP_SERVER
    Tracef("Adding SSE client: id=%s, IP=%s, port=%d, object=%lx\n", id.c_str(), client.remoteIP().toString().c_str(), client.remotePort(), (ulong)&client);
#endif
    clients.Insert(ClientInfo(id, client));

    HttpHeaders httpHeaders(client);
    httpHeaders.sendStreamHeaderSection();
    context.keepAlive = true;

    NotifyState(id);

    return true;
}

bool SSEController::Post(EthClient &client, String &resource, ControllerContext &context)
{
    return false;
}

bool SSEController::Put(EthClient &client, String &resource, ControllerContext &context)
{
    return false;
}

bool SSEController::Delete(EthClient &client, String &id, ControllerContext &context)
{
    struct Params
    {
        SSEController *controller;
        const String &id;
    } params = { this, id };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = (Params *)param;
        String id = params->id;
        if (clientInfo.id.equals(id))
        {
            params->controller->DeleteClient(clientInfo, true);
            return false;
        }

        return true;
    }, &params);

    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}

void SSEController::NotifyState(const String &id)
{
    UpdateStateLastRecoveryTime();

    String event("data:{");
    event += "\"autoRecovery\": ";
    event += state.autoRecovery ? "true" : "false";
    event += ", \"modemState\": ";
    event += static_cast<int>(state.modemState);
    event += ", \"routerState\": ";
    event += static_cast<int>(state.routerState);
    event += ", \"recoveryType\": ";
    event += static_cast<int>(state.recoveryType);
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
    event += ", \"rDisco\": ";
    event += AppConfig::getRDisconnect();
    event += ", \"mDisco\": ";
    event += AppConfig::getMDisconnect();
    event += ", \"rPeriodic\": ";
    event += AppConfig::getPeriodicallyRestartRouter() ? "true" : "false";
    event += ", \"mPeriodic\": ";
    event += AppConfig::getPeriodicallyRestartModem() ? "true" : "false";
    event += "}\n";
#ifdef DEBUG_HTTP_SERVER
    Trace(event);
#endif

    struct Params
    {
        String id;
        String event;
    } params = { id, event };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = (Params *)param;
        String id = params->id;
        if (!id.equals("") && !id.equals(clientInfo.id))
            return true;
        EthClient client = clientInfo.client;
        if (client.connected())
        {
#ifdef DEBUG_HTTP_SERVER
            {
                LOCK_TRACE();
                Trace("Notifying client id=");
                Trace(clientInfo.id);
                Tracef(" IP=%s, port=%d", client.remoteIP().toString().c_str(), client.remotePort());
#ifndef USE_WIFI
                Trace(", socket=");
                Traceln(client.getSocketNumber());
#else
                Traceln();
#endif
            }
#endif
            client.print(params->event);
            client.println();
#ifdef USE_WIFI
            client.flush();
#endif
        }

        if (!id.equals(""))
            return false;
            
        return true;
    }, &params);

    if (state.recoveryType == RecoveryTypes::RouterSingleDevice)
        state.recoveryType = RecoveryTypes::Router;
}

void SSEController::DeleteUnusedClients()
{
    ClientsList clientsToDelete;

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        ClientsList *clientsToDelete = (ClientsList *)param;
        EthClient client = clientInfo.client;
        if (!client.connected())
            clientsToDelete->Insert(clientInfo);
        return true;
    }, &clientsToDelete);

    clientsToDelete.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        SSEController *controller = (SSEController *)param;
        EthClient client = clientInfo.client;
        controller->DeleteClient(clientInfo, client);
        return true;
    }, this);
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
    xTaskCreate([](void *param)
    {
        while (true)
        {
            delay(5000);
            ((SSEController *)param)->DeleteUnusedClients();
        }
    },
    "SSE_DeleteUnusedClients",
    2 * 1024,
    this,
    tskIDLE_PRIORITY,
    NULL);
}

void SSEController::UpdateStateLastRecoveryTime()
{
    time_t lastRecovery = recoveryControl.GetLastRecovery();
    state.showLastRecovery = lastRecovery != INT32_MAX && recoveryControl.GetRecoveryState() == RecoveryTypes::NoRecovery;
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
    struct Params
    {
        SSEController *controller;
        const EthClient client;
        bool stopClient;
        bool ret;
    } params = { this, client, stopClient, false };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = (Params *)param;
        EthClient client = clientInfo.client;
        if (client == params->client)
        {
            params->controller->DeleteClient(clientInfo, params->stopClient);
            params->ret = true;
            return false;
        }

        return true;
    }, &params);

    return params.ret;
}

void SSEController::DeleteClient(const ClientInfo &clientInfo, bool stopClient)
{
    EthClient client = clientInfo.client;
    if (client)
    {
#ifdef DEBUG_HTTP_SERVER
        {
            LOCK_TRACE();
            Tracef("%d Deleting previous session id=%s", client.remotePort(), clientInfo.id.c_str());
#ifndef USE_WIFI
            Trace(", socket=");
            Traceln(client.getSocketNumber());
#else
            Traceln();
#endif
        }
#endif
        if (stopClient)
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("%d Stopping client\n", client.remotePort());
#endif        
            client.stop();
        }
    }
#ifdef DEBUG_HTTP_SERVER
    else
        Tracef("Deleting client info id=%s\n", clientInfo.id);
#endif
    clients.Delete(clientInfo);
}

bool SSEController::IsValidId(const String &id)
{
    struct Params
    {
        String id;
        bool isValid;
    } params = { id, false };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = (Params *)param;
        params->isValid = clientInfo.id.equals(params->id);
        return !params->isValid;
    }, &params);

    return params.isValid;
}

void SSEController::AddClient(const String &id)
{
    if (IsValidId(id))
        return;

    clients.Insert(ClientInfo(id));
}

SSEController sseController;