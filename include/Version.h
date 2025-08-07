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

#ifndef VERSION_H
#define VERSION_H
#include <Arduino.h>

/// @brief Version class provides methods to manage firmware versioning and OTA updates.
class Version
{
public:
    /// @brief Get the current installed firmware version.
    /// @return A String representing the current installed firmware version.
    static String getCurrentVersion();
    /// @brief Get the OTA version available for update.
    /// @return A String representing the OTA version available for update, or "Unknown" if not available.
    static String getOtaVersion();

    typedef enum class _UpdateResult
    {
        error,          ///< An error occurred during the update process.
        noAvailUpdate,  ///< No available update was found.
        done,           ///< The update process completed successfully.
        unknown         ///< The update result is unknown.
    } UpdateResult;

    /// @brief Update the firmware from the OTA server.
    /// @return The result of the update process.
    static UpdateResult updateFirmware();

    /// @brief Callback type for when the update process starts.
    typedef void (*OnStart)();
    /// @brief Callback type for when the update process ends.
    typedef void (*OnEnd)();
    /// @brief Callback type for reporting update progress.
    /// @param sent The number of bytes sent so far.
    /// @param total The total number of bytes to be sent.
    typedef void (*OnProgress)(int sent, int total);
    /// @brief Callback type for reporting errors during the update process.
    /// @param error The error code.
    /// @param message A String containing the error message.
    typedef void (*OnError)(int error, const String& message);

    /// @brief Set the callback for when the update process starts.
    /// @param onStartCallback The callback function to be called when the update starts.
    static void onStart(OnStart onStartCallback);
    /// @brief Set the callback for when the update process ends.
    /// @param onEndCallback The callback function to be called when the update ends.
    static void onProgress(OnProgress onProgressCallback);
    /// @brief Set the callback for reporting when the update process ends.
    /// @param onEndCallback The callback function to be called when the update ends.
    static void onEnd(OnEnd onEndCallback);
    /// @brief Set the callback for reporting errors during the update process.
    /// @param onErrorCallback The callback function to be called when an error occurs.
    static void onError(OnError onErrorCallback);
    
private:
    /// @brief Get the base URL for the OTA drive.
    /// @return A String containing the base URL for the OTA drive.
    static String getOtaDriveBaseUrl();
    /// @brief Get the base parameters for the OTA update.
    /// @return A String containing the base parameters for the OTA update.
    static String getBaseParams();
    /// @brief Get the chip ID for the OTA update.
    /// @return A String containing the chip ID for the OTA update.
    static String getChipId();
    /// @brief  Get the update URL for the OTA update.
    /// @return A String containing the update URL for the OTA update.
    static String getUpdateURL();

private:
    /// @brief A string representing unknown version. this string is used when the version cannot be determined.
    static const String unknownVersion;
};

#endif // VERSION_H