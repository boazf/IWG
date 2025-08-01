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

#ifndef SSEController_h
#define SSEController_h

#include <Arduino.h>
#include <HttpController.h>
#include <RecoveryControl.h>

/// @brief This class contains information about clients connected to the SSEController. 
struct ClientInfo
{
public:
    /// @brief Constructor for the ClientInfo class.
    /// @param _id The unique identifier for the client.
    /// @param _client The EthClient object associated with the client.
    ClientInfo(const String &_id, EthClient &_client) :
        id(_id),
        client(_client)
    {
    }

    /// @brief Constructor for the ClientInfo class (overloaded). This constructor is used when the index page is called with no ID in the URL.
    /// So an ID an ID is generated and an HTTP redirect reply is sent. When the index page will ber called again, this time with the ID in the URL,
    /// then a clientInfo will be added to the class. This instance in the list of clients is required so that SSEController will identify the ID as valid
    /// ID.
    /// @param _id The unique identifier for the client.
    ClientInfo(const String &_id) :
        id(_id)
    {
    }

    /// @brief Copy constructor for the ClientInfo class.
    /// @param clientInfo The ClientInfo object to copy.
    ClientInfo(const ClientInfo &clientInfo)
    {
        *this = clientInfo;
    }

    /// @brief Assignment operator for the ClientInfo class.
    /// @param clientInfo The ClientInfo object to copy.
    /// @return A reference to this object. 
    ClientInfo &operator=(const ClientInfo &clientInfo)
    {
        *const_cast<String *>(&id) = clientInfo.id;
        client = clientInfo.client;
        return *this;
    }

    /// @brief Equality operator for the ClientInfo class.
    /// @param c The ClientInfo object to compare.
    /// @return True if the objects are equal, false otherwise.
    bool operator==(const ClientInfo &c) const
    {
        return id == c.id;
    }

    /// @brief the unique identifier for the client.
    const String id;
    /// @brief the EthClient object associated with the client.
    EthClient client;
};

#undef ON_RECOVERY_STATE_CHANGED
#undef ON_AUTO_RECOVERY_STATE_CHANGED
#undef ON_MODEM_POWER_STATE_CHANGED
#undef ON_ROUTER_POWER_STATE_CHANGED
#define ON_RECOVERY_STATE_CHANGED(fnName) ON_EVENT(SSEController, RecoveryStateChangedParams, fnName)
#define ON_AUTO_RECOVERY_STATE_CHANGED(fnName) ON_EVENT(SSEController, AutoRecoveryStateChangedParams, fnName)
#define ON_MODEM_POWER_STATE_CHANGED(fnName) ON_EVENT(SSEController, PowerStateChangedParams, fnName)
#define ON_ROUTER_POWER_STATE_CHANGED(fnName) ON_EVENT(SSEController, PowerStateChangedParams, fnName)

/// @brief This class is used to manage Server-Sent Events (SSE) connections and notifications.
/// It allows clients to subscribe to events and receive updates in real-time.
class SSEController : public HttpController
{
public:
    SSEController()
    {
    }

    /// @brief Handles GET requests for SSE clients.
    /// @param context The HTTP client context.
    /// @param id The unique identifier for the client.
    /// @return True if the request was handled successfully, false otherwise.
    bool Get(HttpClientContext &context, const String id);
    /// @brief Handles POST requests for SSE clients.
    /// @param context The HTTP client context.
    /// @param id The unique identifier for the client.
    /// @return True if the request was handled successfully, false otherwise.
    bool Post(HttpClientContext &context, const String id);
    /// @brief Handles PUT requests for SSE clients.
    /// @param context The HTTP client context.
    /// @param id The unique identifier for the client.
    /// @return True if the request was handled successfully, false otherwise.
    bool Put(HttpClientContext &context, const String id);
    /// @brief Handles DELETE requests for SSE clients.
    /// @param context The HTTP client context.
    /// @param id The unique identifier for the client.
    /// @return True if the request was handled successfully, false otherwise.
    bool Delete(HttpClientContext &context, const String id);
    /// @brief Initializes the SSEController.
    /// This method sets up the necessary observers for recovery state changes, auto-recovery state changes
    /// modem power state changes, and router power state changes.
    /// It also initializes the state of the controller.
    /// @note This method should be called once at the start of the program to set up the controller.
    void Init();
    /// @brief Deletes a client from the controller.
    /// @param client The EthClient object to delete.
    /// @param stopClient If true, the client will be stopped before deletion.
    /// @return True if the client was deleted successfully, false otherwise.
    bool DeleteClient(EthClient &client, bool stopClient);
    /// @brief Checks if the given ID is valid. That is if it exists in the list of clients.
    /// @param id The ID to check.
    /// @return True if the ID is valid, false otherwise.
    bool IsValidId(const String &id);
    /// @brief Adds a client to the controller. It adds a class with only the ID,
    /// so that when the index page is called with that ID it will be recognized as a valid ID.
    /// @param id The unique identifier for the client.
    void AddClient(const String &id);
    /// @brief Checks if the controller is a singleton.
    /// @return True if the controller is a singleton, false otherwise.
    bool isSingleton() { return true; }
    /// @brief Gets the instance of the SSEController.
    /// @return A pointer to the SSEController instance.
    static HttpController *getInstance();

private:
    /// @brief Handles recovery state changes.
    /// @param params The parameters for the recovery state change.
    /// @param context The context for the HTTP client.
    ON_RECOVERY_STATE_CHANGED(OnRecoveryStateChanged);
    /// @brief Handles auto-recovery state changes.
    /// @param params The parameters for the auto-recovery state change.
    /// @param context The context for the HTTP client.
    ON_AUTO_RECOVERY_STATE_CHANGED(OnAutoRecoveryStateChanged);
    /// @brief Handles modem power state changes.
    /// @param params The parameters for the modem power state change.
    /// @param context The context for the HTTP client.
    ON_MODEM_POWER_STATE_CHANGED(OnModemPowerStateChanged);
    /// @brief Handles router power state changes.
    /// @param params The parameters for the router power state change.
    /// @param context The context for the HTTP client.
    ON_ROUTER_POWER_STATE_CHANGED(OnRouterPowerStateChanged);
    /// @brief Notifies the state of a client.
    /// @param id The unique identifier for the client. If empty, the state will be notified to all clients.
    /// @note This method is used to send updates to the clients about the current state of the controller.
    void NotifyState(const String &id);
    /// @brief Updates the last recovery time in the state.
    void UpdateStateLastRecoveryTime();
    /// @brief Deletes a client from the controller.
    /// @param clientInfo The information of the client to delete.
    /// @param stopClient If true, the client will be stopped before deletion.
    void DeleteClient(const ClientInfo &clientInfo, bool stopClient);
    /// @brief Deletes unused clients from the controller.
    /// This method scans the list of clients and deletes those that are no longer active or have been disconnected.
    /// @note This method is typically called periodically to clean up the list of clients. 
    void DeleteUnusedClients();

private:
    /// @brief A linked list to store the clients connected to the SSEController.
    typedef LinkedList<ClientInfo> ClientsList;
    ClientsList clients;

    /// @brief The state of the SSEController.
    /// This state contains information about the current recovery type, modem state, router state, etc.
    class SSEControllerState
    {
    public:
        bool autoRecovery; // true if auto-recovery is enabled, false otherwise
        RecoveryTypes recoveryType; // the current recovery type
        PowerState modemState; // the current power state of the modem
        PowerState routerState; // the current power state of the router
        bool showLastRecovery; // true if the last recovery time should be shown, false otherwise
        int days; // the number of days since the last recovery
        int hours; // the number of hours since the last recovery
        int minutes; // the number of minutes since the last recovery
        int seconds; // the number of seconds since the last recovery
    };

    /// @brief The current state of the SSEController.
    SSEControllerState state;
};

/// @brief A global instance of the SSEController.
extern SSEController sseController;

#endif // SSEController_h