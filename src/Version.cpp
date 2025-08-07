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

#include <Version.h>
#include <Config.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <HttpUpdate.h>

const String Version::unknownVersion = "Unknown";

// The current version of the application.
// This version is reported as the currently install version 
// and also used for OTA updates.
// This version should be updated with each release.
#define APP_VERSION "1.0.28"

String Version::getCurrentVersion()
{
    return APP_VERSION;
}

String Version::getOtaVersion()
{
    // Create an HTTP client to fetch the OTA version.
    // The client will automatically stop when it goes out of scope.
    AutoStopClient client;
    // Create an instance of HttpClientEx to handle the HTTP request.
    HttpClientEx http(client);
    // Set the http agent for the request.
    http.begin(getUpdateURL());
    // Perform a GET request to fetch the OTA version.
    http.get();
    // Check the response status code.
    // If the response code is less than 0, it indicates an error.
    // If the response code is 304, it means no update is available, so return the current version.
    int code = http.responseStatusCode();
    if (code < 0)
        return unknownVersion;
    if (code == 304)
        return getCurrentVersion();

    // Check the headers for the "X-Version" header to get the OTA version.
    HttpClientEx::Headers headers[] = {{"X-Version"}};
    http.collectHeaders(headers, NELEMS(headers));

    if (headers[0].value.isEmpty())
        // If the "X-Version" header is not found or is empty, return the unknown version.
        return unknownVersion;

    // If the "X-Version" header is found, return its value.
    return headers[0].value;
}

Version::UpdateResult Version::updateFirmware()
{
    // Create an HTTP client to fetch the OTA version.
    // The client will automatically stop when it goes out of scope.
    AutoStopClient client;
    // Do the OTA update using the HttpUpdate class.
    HttpUpdateResult ret = httpUpdate.update(client, getUpdateURL(), getCurrentVersion());
    // Convert the HttpUpdateResult to Version::UpdateResult.
    switch(ret)
    {
        case HttpUpdateResult::HTTP_UPDATE_FAILED:
            return UpdateResult::error;
        case HttpUpdateResult::HTTP_UPDATE_NO_UPDATES:
            return UpdateResult::noAvailUpdate;
        case HttpUpdateResult::HTTP_UPDATE_OK:
            return UpdateResult::done;
    }

    return UpdateResult::unknown;
}


void Version::onStart(OnStart onStartCallback)
{
    // Set the callback for when the update process starts.
    httpUpdate.onStart(onStartCallback);
}

void Version::onProgress(OnProgress onProgressCallback)
{
    // Set the callback for when the update process progresses.
    httpUpdate.onProgress(onProgressCallback);
}

void Version::onEnd(OnEnd onEndCallback)
{
    // Set the callback for when the update process ends.
    httpUpdate.onEnd(onEndCallback);
}

void Version::onError(OnError onErrorCallback)
{
    // Set the callback for when an error occurs during the update process.
    static OnError otaOnError;
    otaOnError = onErrorCallback;
    // HttpUpdate error callback has no error message, only error code.
    // However, we can use the getLastErrorString() method to get the error message.
    httpUpdate.onError([](int errCode)
    {
        // Add the error message to the error callback.
        String errMessage = httpUpdate.getLastErrorString();
        otaOnError(errCode, errMessage);
    });
}

String Version::getOtaDriveBaseUrl()
{
    return String("http://") + Config::otaServer + "/deviceapi/";
}

String Version::getChipId()
{
    String chipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
    chipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
    return chipIdHex;
}

String Version::getBaseParams()
{
    return String("k=") + Config::otaApiKey + "&v=" + getCurrentVersion() + "&s=" + getChipId();
}

String Version::getUpdateURL()
{
    return getOtaDriveBaseUrl() + "update?" + getBaseParams();
}
