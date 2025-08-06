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

/// @brief SystemController handles system-level HTTP requests.
/// It provides endpoints for system information, updates, and rebooting the system.
/// This controller is a singleton, meaning there is only one instance of it in the application.
/// It inherits from HttpController to handle HTTP requests and responses.
class SystemController : public HttpController
{
public:
    SystemController()
    {
    }

    /// @brief Handles GET requests for system information.
    /// @param context The HTTP client context.
    /// @param id The identifier for the requested resource.
    /// @return True if the request was handled successfully, false otherwise.
    bool Get(HttpClientContext &context, const String id)
    {
        if (id.equals("info"))
            // Send system version information as a JSON response.
            return sendVersionInfo(context);
        else if (id.equals("update"))
            // Handle system firmware update requests.
            return updateVersion(context);
        else if (id.equals("reboot"))
            // Reboot the system.
            HardReset(3000, 15000);

        return true;
    }

    // POST request is unhandler by this controller.
    bool Post(HttpClientContext &context, const String id)
    {
        return false;
    }

    // PUT requests are unhandled by this controller.
    bool Put(HttpClientContext &context, const String id)
    {
        return false;
    }

    // DELETE request is unhandled by this controller.
    bool Delete(HttpClientContext &context, const String id)
    {
        return false;
    }

    /// @brief This controller is a singleton.
    bool isSingleton() { return true; }
    /// @brief Returns a pointer to the global SystemController instance.
    static HttpController *getInstance();

private:
    /// @brief While the firmware is being updated, the contrtoller notifies the client about the update progress.
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
    /// @brief Sends the system version information to the client.
    /// @param context The HTTP client context.
    /// @return True if the response was sent successfully, false otherwise.
    static bool sendVersionInfo(HttpClientContext &context);
    /// @brief Updates the system firmware.
    /// @param context The HTTP client context.
    /// @return True if the update was initiated successfully, false otherwise.
    /// @note This method handles the firmware update process and notifies the client about the progress.
    /// It uses a background task to perform the update and sends notifications to the client during the process.
    static bool updateVersion(HttpClientContext &context);
    /// @brief  Generates a JSON header for the notification.
    /// @param notificationType The type of notification to generate the header for.
    /// @return A JSON string representing the notification header.
    static String notificationJsonHead(NotificationType notificationType);
    /// @brief Notifies the client about the firmware update progress.
    /// @param client The Ethernet client to send the notification to.
    /// @param notificationType The type of notification to send.
    static void notify(EthClient &client, NotificationType notificationType);
    /// @brief Notifies the client about the firmware update progress.
    /// @param client The Ethernet client to send the notification to.
    /// @param notificationType The type of notification to send.
    /// @param sent The number of bytes sent.
    /// @param total The total number of bytes to send.
    static void notify(EthClient &client, NotificationType notificationType, int sent, int total);
    /// @brief Notifies the client about an error that occurred during the firmware update.
    /// @param client The Ethernet client to send the notification to.
    /// @param notificationType The type of notification to send.
    /// @param error The error code.
    /// @param message The error message.
    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message);
    /// @brief Notifies the client with a JSON message.
    /// @param client The Ethernet client to send the notification to.
    /// @param json The JSON message to send.
    /// @note This method sends a JSON message to the client, typically used for notifications during the firmware update process.
    static void notify(EthClient &client, const String &json);
    /// @brief A map that holds the notification types and their corresponding string representations.
    static const std::map<SystemController::NotificationType, String> notificationTypesStrings;
};

/// @brief Global instance of the SystemController.
extern SystemController systemController;

#endif // SYSTEM_CONTROLLER_H

