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

#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <Common.h>

class Config
{
public:
    static byte mac[6];
#ifndef USE_WIFI
    static long routerInitTimeSec;
    static long timeUpdatePeriodMin;
#endif
    static long dnsAvailTimeSec;
    static byte ip[4];
    static byte gateway[4];
    static byte mask[4];
    static long timeZone;
    static long DST;
    static const char *timeServer;
    static byte modemRelay;
    static byte routerRelay;
    static bool singleDevice;
    static const char *deviceName;
    static time_t skipRouterTime;
    static long hardResetPeriodDays;
    static long hardResetTime;
    static const char *otaServer;
#ifdef USE_WIFI
    static const char *ssid;
    static const char *password;
    static const char *hostName;
#endif


public:
    static void Init();

private:
    static const char *configFileName;

private:
    static bool ParseString(const String &configValue, void *str);
    static bool ParseLong(const String &configValue, void *parsedTimeZone);
    static bool ParseByte(const String &strByte, byte *value);
    static bool ParseByte(const String &strByte, void *value);
    static bool ParseByteArray(const String &strValue, char separator, byte *bytes, int arrayLen);
    static bool ParseMACAddress(const String &configValue, void *parsedMACAddress);
    static bool ParseIPAddress(const String &configValue, void *parsedIPAddress);
    static bool ParseBoolean(const String &configValue, void *parsedBoolean);
    static bool ParseTime(const String &configValue, void *parsedTime);
};

void InitConfig();

#endif // Config_h