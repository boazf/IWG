/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <SSEController.h>
#include <Relays.h>
#include <TimeUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool SSEController::Get(HttpClientContext &context, const String id)
{
    EthClient client = context.getClient();

#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Trace("SSEController Get, Client id=");
        Trace(id);
#ifndef USE_WIFI
        Trace(", socket=");
        Traceln(context.getClient().getSocketNumber());
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
        const Params *params = static_cast<const Params *>(param);
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

bool SSEController::Post(HttpClientContext &context, const String id)
{
    return false;
}

bool SSEController::Put(HttpClientContext &context, const String id)
{
    return false;
}

bool SSEController::Delete(HttpClientContext &context, const String id)
{
    struct Params
    {
        SSEController *controller;
        const String &id;
    } params = { this, id };

    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        const Params *params = static_cast<const Params *>(param);
        String id = params->id;
        if (clientInfo.id.equals(id))
        {
            params->controller->DeleteClient(clientInfo, true);
            return false;
        }

        return true;
    }, &params);

    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(context.getClient());
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
        const Params *params = static_cast<const Params *>(param);
        String id = params->id;
        if (!id.equals("") && !id.equals(clientInfo.id))
            return true;
        EthClient client = clientInfo.client;
        if (client.connected())
        {
#ifdef DEBUG_HTTP_SERVER
            TRACE_BLOCK
            {
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
        ClientsList *clientsToDelete = const_cast<ClientsList *>(static_cast<const ClientsList *>(param));
        EthClient client = clientInfo.client;
        if (!client.connected())
            clientsToDelete->Insert(clientInfo);
        return true;
    }, &clientsToDelete);

    clientsToDelete.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        SSEController *controller = const_cast<SSEController *>(static_cast<const SSEController *>(param));
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
        SSEController *controller = static_cast<SSEController *>(param);

        while (true)
        {
            delay(5000);
            controller->DeleteUnusedClients();
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
    SSEController *controller = const_cast<SSEController *>(static_cast<const SSEController *>(context));
    controller->state.recoveryType = params.m_recoveryType;
    controller->UpdateStateLastRecoveryTime();
    controller->NotifyState("");
}

void SSEController::AutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params, const void *context)
{
#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Trace("AutoRecoveryStateChanged: ");
        Traceln(params.m_autoRecovery);
    }
#endif
    SSEController *controller = const_cast<SSEController *>(static_cast<const SSEController *>(context));
    controller->state.autoRecovery = params.m_autoRecovery;
    controller->NotifyState("");
}

void SSEController::ModemPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = const_cast<SSEController *>(static_cast<const SSEController *>(context));
    controller->state.modemState = params.m_state;
    controller->NotifyState("");
}

void SSEController::RouterPowerStateChanged(const PowerStateChangedParams &params, const void *context)
{
    SSEController *controller = const_cast<SSEController *>(static_cast<const SSEController *>(context));
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
        Params *params = const_cast<Params *>(static_cast<const Params *>(param));
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
        TRACE_BLOCK
	    {
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
        Params *params = const_cast<Params *>(static_cast<const Params *>(param));
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

HttpController *SSEController::getInstance() { return &sseController; }

SSEController sseController;