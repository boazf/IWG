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

    // Scan the list of clients and delete the client if there is already a client with the same ID.
    // In case index page is invoked with no ID, a temporary clientInfo instance is created with the ID
    // and added to the clients list. Then the request is replied with a redirect to the index page with the ID in the URL.
    // So now it is time to delete this temporary clientInfo instance and replace it with a real clientInfo instance.
    // The temporary clientInfo instance is required so that the SSE controller will recognise the redirected call as a valid 
    // call with a valid ID.
    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        const Params *params = static_cast<const Params *>(param);
        String id = params->id;
        if (clientInfo.id.equals(id))
        {
            // If the client already exists, delete it.
            params->controller->DeleteClient(clientInfo, true);
            return false;
        }

        return true;
    }, &params);
#ifdef DEBUG_HTTP_SERVER
    Tracef("Adding SSE client: id=%s, IP=%s, port=%d, object=%lx\n", id.c_str(), client.remoteIP().toString().c_str(), client.remotePort(), (ulong)&client);
#endif
    // Add the client to the list of clients.
    clients.Insert(ClientInfo(id, client));

    // Send response to the client to acknowledge the SSE request.
    HttpHeaders httpHeaders(client);
    httpHeaders.sendStreamHeaderSection();
    // Set the keep-alive flag to true to keep the connection open for SSE.
    context.keepAlive = true;

    // Notify the client about the current state.
    NotifyState(id);

    return true;
}

// SSE controller doesn't implement Post or Put methods, so they return false.
bool SSEController::Post(HttpClientContext &context, const String id)
{
    return false;
}

bool SSEController::Put(HttpClientContext &context, const String id)
{
    return false;
}

// This method is called when the index page is exited, so it deletes the client with the given ID. 
// This call is triggered by the beforeunload event in the browser.
// Not all browsers call the beforeunload event. So there is also a background task that periodically deletes unused clients.
bool SSEController::Delete(HttpClientContext &context, const String id)
{
    struct Params
    {
        SSEController *controller;
        const String &id;
    } params = { this, id };

    // Scan the list of clients and delete the client with the given ID.
    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        const Params *params = static_cast<const Params *>(param);
        String id = params->id;
        if (clientInfo.id.equals(id))
        {
            // If the client with the given ID is found, delete it.
            params->controller->DeleteClient(clientInfo, true);
            return false;
        }

        return true;
    }, &params);

    // Send a response to the client to acknowledge the deletion.
    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}

void SSEController::NotifyState(const String &id)
{
    UpdateStateLastRecoveryTime();

    // Prepare the event data to be sent to the clients.
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

    // Scan the list of clients and send the event data to the requires client(s).
    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        const Params *params = static_cast<const Params *>(param);
        String id = params->id;
        // If an ID is provided, only send the event to the client with that ID.
        if (!id.equals("") && !id.equals(clientInfo.id))
            return true;
        EthClient client = clientInfo.client;
        // If the client is not connected, skip it.
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
            // Send the event data to the client.`
            client.print(params->event);
            client.println();
#ifdef USE_WIFI
            client.flush();
#endif
        }

        // If an ID is provided, stop scanning after sending the event to the specified client.
        if (!id.equals(""))
            return false;
            
        // Continue scanning for other clients if no ID is provided.
        return true;
    }, &params);

    if (state.recoveryType == RecoveryTypes::RouterSingleDevice)
        state.recoveryType = RecoveryTypes::Router;
}

// This method is called periodically to delete unused clients.
// It scans the list of clients and deletes those that are not connected.
void SSEController::DeleteUnusedClients()
{
    ClientsList clientsToDelete;

    // Scan the list of clients and add the unused clients to the clientsToDelete list.
    // It is not possible to delete clients while scanning the list, so we first collect the clients to delete in a separate list.
    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        ClientsList *clientsToDelete = const_cast<ClientsList *>(static_cast<const ClientsList *>(param));
        EthClient client = clientInfo.client;
        if (!client.connected())
            clientsToDelete->Insert(clientInfo);
        return true;
    }, &clientsToDelete);

    // Now delete the clients that were collected in the clientsToDelete list.
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
    // Initialize the state of the controller.
    state.autoRecovery = recoveryControl.GetAutoRecovery();
    state.recoveryType = recoveryControl.GetRecoveryState();
    state.modemState = GetModemPowerState();
    state.routerState = GetRouterPowerState();
    UpdateStateLastRecoveryTime();
    // Register observers for various state changes.
    recoveryControl.addRecoveryStateChangedObserver(OnRecoveryStateChanged, this);
    recoveryControl.addAutoRecoveryStateChangedObserver(OnAutoRecoveryStateChanged, this);
    recoveryControl.addModemPowerStateChangedObserver(OnModemPowerStateChanged, this);
    recoveryControl.addRouterPowerStateChangedObserver(OnRouterPowerStateChanged, this);
    // Start a background task to periodically delete unused clients.
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
    4 * 1024,
    this,
    tskIDLE_PRIORITY,
    NULL);
}

void SSEController::UpdateStateLastRecoveryTime()
{
    time_t lastRecovery = recoveryControl.GetLastRecovery();
    // We should show the last recovery time only if we are not in a recovery state.
    state.showLastRecovery = lastRecovery != INT32_MAX && recoveryControl.GetRecoveryState() == RecoveryTypes::NoRecovery;
    if (state.showLastRecovery)
    {
        // Calculate the time since the last recovery.
        time_t timeSinceLastRecovery = t_now - lastRecovery;
        state.seconds = timeSinceLastRecovery % 60;
        state.minutes = (timeSinceLastRecovery / 60) % 60;
        state.hours = (timeSinceLastRecovery / 3600) % 24;
        state.days = (timeSinceLastRecovery / 3600 / 24);
    }
}

void SSEController::OnRecoveryStateChanged(const RecoveryStateChangedParams &params)
{
    state.recoveryType = params.m_recoveryType;
    UpdateStateLastRecoveryTime();
    // Notify all the clients about the recovery state change.
    NotifyState("");
}

void SSEController::OnAutoRecoveryStateChanged(const AutoRecoveryStateChangedParams &params)
{
#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Trace("AutoRecoveryStateChanged: ");
        Traceln(params.m_autoRecovery);
    }
#endif
    state.autoRecovery = params.m_autoRecovery;
    // Notify all the clients about the auto-recovery state change.
    NotifyState("");
}

void SSEController::OnModemPowerStateChanged(const PowerStateChangedParams &params)
{
    state.modemState = params.m_state;
    // Notify all the clients about the modem power state change.
    NotifyState("");
}

/// @brief Supposed to be called when the router power state changes. However, this function is not used in the current implementation.
/// When the router power is turned off, the browser will not receive any updates. So what happens is a notification is sent
/// to the browser that a router recovery is about to start. The browser will then simulate turning off the router power
/// by turning the router switch to off, it will wait the specified configured time, and then it will turn the router switch back on.
/// This is all done in the browser. So no need to send power change notifications, they will not be received by the browser anyhow.
/// @param params The parameters containing the new power state of the router.
/// @param context The context pointer.
void SSEController::OnRouterPowerStateChanged(const PowerStateChangedParams &params)
{
    state.routerState = params.m_state;
    NotifyState("");
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
            // Since we do not continue to scan the list after finding the client,
            // we can safely delete the client here.
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
    // If the client is not connected then there is no need to stop it.
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
            // Stop the client connection.
            client.stop();
        }
    }
#ifdef DEBUG_HTTP_SERVER
    else
        Tracef("Deleting client info id=%s\n", clientInfo.id);
#endif
    // Delete the client info from the list of clients.
    clients.Delete(clientInfo);
}

bool SSEController::IsValidId(const String &id)
{
    struct Params
    {
        String id;
        bool isValid;
    } params = { id, false };

    // Scan the list of clients and check if one of the clients has the given ID.
    clients.ScanNodes([](const ClientInfo &clientInfo, const void *param)->bool
    {
        Params *params = const_cast<Params *>(static_cast<const Params *>(param));
        params->isValid = clientInfo.id.equals(params->id);
        // If the ID is found, stop scanning the list. Otherwise, continue scanning.
        return !params->isValid;
    }, &params);

    return params.isValid;
}

void SSEController::AddClient(const String &id)
{
    // If the ID is already valid, do not add it again.
    if (IsValidId(id))
        return;

    // Add a new client with the given ID to the list of clients.
    clients.Insert(ClientInfo(id));
}

/// @brief Pointer to the global SSEController instance.
static std::shared_ptr<SSEController> pSSEController = std::make_shared<SSEController>();

/// This method returns a pointer to the global SSEController instance.
std::shared_ptr<HttpController> SSEController::getInstance() { return pSSEController; }

/// Global instance of the SSEController.
SSEController &sseController = *pSSEController;