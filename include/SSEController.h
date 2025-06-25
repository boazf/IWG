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

class SSEController : public HttpController
{
public:
    SSEController()
    {
    }

    bool Get(HttpClientContext &context, const String id);
    bool Post(HttpClientContext &context, const String id);
    bool Put(HttpClientContext &context, const String id);
    bool Delete(HttpClientContext &context, const String id);
    void Init();
    bool DeleteClient(EthClient &client, bool stopClient);
    bool IsValidId(const String &id);
    void AddClient(const String &id);
    bool isSingleton() { return true; }
    static HttpController *getInstance();

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