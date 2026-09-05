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
    /// Get the OTA version information.
    String version = Version::getOtaVersion();
    // Construct the JSON response.
    String versionJson = "{ \"Version\" : \"" + version + "\" }";

    // Prepare the HTTP headers for the response.
    HttpHeaders::Header additionalHeaders[] = {{CONTENT_TYPE::JSON}, {"Access-Control-Allow-Origin", "*"}};
    HttpHeaders headers(context.getClient());
    // Send the HTTP response headers.
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), versionJson.length());

    // Send the JSON response body.
    context.getClient().print(versionJson.c_str());

    return true;
}

bool SystemController::updateVersion(HttpClientContext &context)
{
    // This variable is used to ensure that only one update process runs at a time.
    static std::atomic<bool> busy(false);

    // If an update is already in progress, we return false to indicate that the request was not handled.
    bool free = !busy.exchange(true);

    if (free)
    {
        // Create a task to handle the firmware update process.
        // The task will run in the background and notify the client about the update progress.
        // The task will also handle any errors that may occur during the update process.
        BaseType_t ret = xTaskCreate(
            [](void *param)
            {
                // Place the notification client in a static variable so that it can be accessed by the notify lambda function.
                static EthClient notificationClient;
                notificationClient = *static_cast<EthClient *>(param);
                Version::onStart([]()
                {
                    // Notify the client that the update process has started.
                    notify(notificationClient, NotificationType::start);
                });
                Version::onProgress([](int sent, int total)
                {
                    // Notify the client about the update progress.
                    notify(notificationClient, NotificationType::progress, sent, total);
                });
                Version::onEnd([]()
                {
                    // Notify the client that the update process has ended.
                    notify(notificationClient, NotificationType::end);
                    delay(1000);
                });
                Version::onError([](int error, const String &message)
                {
                    // Notify the client about an error that occurred during the update process.
                    notify(notificationClient, NotificationType::error, error, message);
                });

                // Perform the firmware update.
                // This will download the firmware from the OTA server and apply it.
                // The updateFirmware method will handle the actual update process.
                // It will also notify the client about the update progress.
                // If there is no available update, it will notify the client about it.
                // If the update is successful, the system will reboot.
                Version::UpdateResult res = Version::updateFirmware();
                if (res == Version::UpdateResult::noAvailUpdate)
                    notify(notificationClient, NotificationType::error, 0, "No available update");
    #ifdef DEBUG_HTTP_SERVER
                Tracef("%d Stopping client\n", notificationClient.remotePort());
    #endif
                // Stop the client connection after the update process is completed.
                notificationClient.stop();
    #ifdef DEBUG_HTTP_SERVER
                Tracef("Update task stack high watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
    #endif
                // Allow further requests to be processed by setting the busy flag to false.
                // If the update result is done, we do not free the busy flag,
                // because the system will reboot and the busy flag will be reset.
                // If the update result is error or no available update, we free the busy flag.
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
            // If the task creation failed, we notify the client about the error and free the busy flag.
            busy.exchange(false);
            return false;
        }
    }

    // We sent a response to the client indicating that the update process has started
    // and we keep the connection alive in order to send notifications.
    EthClient client = context.getClient();
    HttpHeaders headers(client);
    headers.sendStreamHeaderSection();
    // Set the keep-alive flag to true if the free flag is true to keep the connection open for notifications.
    // Otherwise, we set it to false to close the connection after the response is sent.
    // The connection is stopped in the update task after the update process is completed.
    context.keepAlive = free;

    if (!free)
    {
        // If there is another update process already running, we notify the client about it.
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

static std::shared_ptr<HttpController> systemController = std::make_shared<SystemController>();

std::shared_ptr<HttpController> SystemController::getInstance() { return systemController; }

#define X(a) {SystemController::NotificationType::a, #a},
const std::map<SystemController::NotificationType, String> SystemController::notificationTypesStrings =
{
    NOTIFICATION_TYPES
};
#undef X
