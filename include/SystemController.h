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

#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H
#include <Common.h>
#include <HttpController.h>
#include <PwrCntl.h>
#include <map>

class SystemController : public HttpController
{
public:
    SystemController()
    {
    }

    bool Get(HttpClientContext &context, const String id)
    {
        if (id.equals("info"))
            return sendVersionInfo(context);
        else if (id.equals("update"))
            return updateVersion(context);
        else
            if (id.equals("reboot"))
                HardReset();
                
        return false;
    }

    bool Post(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool Put(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool Delete(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool isSingleton() { return true; }
    static HttpController *getInstance();

private:
    #define NOTIFICATION_TYPES \
        X(start) \
        X(progress) \
        X(end) \
        X(error)

    #define X(a) a,
    typedef enum class _NotificationType
    {
        NOTIFICATION_TYPES
    } NotificationType;
    #undef X

private:
    static bool sendVersionInfo(HttpClientContext &context);
    static bool updateVersion(HttpClientContext &context);
    static String notificationJsonHead(NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType, int sent, int total);
    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message);
    static void notify(EthClient &client, const String &json);
    static const std::map<SystemController::NotificationType, String> notificationTypesStrings;
};

extern SystemController systemController;

#endif // SYSTEM_CONTROLLER_H

