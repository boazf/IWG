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

#include <SystemController.h>
#include <Version.h>
#include <HttpUpdate.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif
#include <HttpHeaders.h>
#include <atomic>

bool SystemController::sendVersionInfo(HttpClientContext &context)
{
    
    String version = Version::getOtaVersion();
    String versionJson = "{ \"Version\" : \"" + version + "\" }";

    HttpHeaders::Header additionalHeaders[] = {{CONTENT_TYPE::JSON}, {"Access-Control-Allow-Origin", "*"}};
    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), versionJson.length());

    context.getClient().print(versionJson.c_str());

    return true;
}

bool SystemController::updateVersion(HttpClientContext &context)
{
    static std::atomic<bool> busy(false);

    bool free = !busy.exchange(true);

    if (free)
    {
        BaseType_t ret = xTaskCreate(
            [](void *param)
            {
                static EthClient notificationClient;
                notificationClient = *static_cast<EthClient *>(param);
                Version::onStart([]()
                {
                    notify(notificationClient, NotificationType::start);
                });
                Version::onProgress([](int sent, int total)
                {
                    notify(notificationClient, NotificationType::progress, sent, total);
                });
                Version::onEnd([]()
                {
                    notify(notificationClient, NotificationType::end);
                    delay(1000);
                });
                Version::onError([](int error, const String &message)
                {
                    notify(notificationClient, NotificationType::error, error, message);
                });

                Version::UpdateResult res = Version::updateFirmware();
                if (res == Version::UpdateResult::noAvailUpdate)
                    notify(notificationClient, NotificationType::error, 0, "No available update");
    #ifdef DEBUG_HTTP_SERVER
                Tracef("%d Stopping client\n", notificationClient.remotePort());
    #endif
                notificationClient.stop();
    #ifdef DEBUG_HTTP_SERVER
                Tracef("Update task stack high watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
    #endif
                busy.exchange(res == Version::UpdateResult::done);
                vTaskDelete(NULL);
            },
            "UpdateFirmware",
            4*1024,
            &context.getClient(),
            tskIDLE_PRIORITY,
            NULL);
        if (ret != pdPASS)
        {
            busy.exchange(false);
            return false;
        }
    }

    EthClient client = context.getClient();
    HttpHeaders headers(client);
    headers.sendStreamHeaderSection();
    context.keepAlive = free;

    if (!free)
    {
        notify(client, NotificationType::error, 0, "An update already started by another client!");
    }

    return true;
}

String SystemController::notificationJsonHead(NotificationType notificationType)
{
    return String("{ \"type\": \"") + notificationTypesStrings.at(notificationType) + "\"";
}

void SystemController::notify(EthClient &client, NotificationType notificationType)
{
    notify(client, notificationJsonHead(notificationType) + " }");
}

void SystemController::notify(EthClient &client, NotificationType notificationType, int sent, int total)
{
    notify(client, notificationJsonHead(notificationType) + ", \"sent\": " + sent + ", \"total\": " + total + " }");
}

void SystemController::notify(EthClient &client, NotificationType notificationType, int error, const String &message)
{
    notify(client, notificationJsonHead(notificationType) + ", \"code\": " + error + ", \"message\": \"" + message + "\" }");
}

void SystemController::notify(EthClient &client, const String &json)
{
    client.println(String("data:") + json + "\n");
}

HttpController *SystemController::getInstance() { return &systemController; }

SystemController systemController;

#define X(a) {SystemController::NotificationType::a, #a},
const std::map<SystemController::NotificationType, String> SystemController::notificationTypesStrings =
{
    NOTIFICATION_TYPES
};
#undef X
