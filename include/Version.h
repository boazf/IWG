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

class Version
{
public:
    static String getCurrentVersion();
    static String getOtaVersion();

    typedef enum class _UpdateResult
    {
        error,
        noAvailUpdate,
        done,
        unknown
    } UpdateResult;

    static UpdateResult updateFirmware();

    typedef void (*OnStart)();
    typedef void (*OnEnd)();
    typedef void (*OnProgress)(int, int);
    typedef void (*OnError)(int, const String&);

    static void onStart(OnStart onStartCallback);
    static void onProgress(OnProgress onProgressCallback);
    static void onEnd(OnEnd onEndCallback);
    static void onError(OnError onErrorCallback);
    
private:
    static String getOtaDriveBaseUrl();
    static String getBaseParams();
    static String getChipId();

private:
    static const String unknownVersion;
    static const char *apiKey;
};

#endif // VERSION_H