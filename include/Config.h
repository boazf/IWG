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

/// @brief This structure holds the configuration values extracted from the configuration file.
class Config
{
public:
    /// @brief The mac address from the configuration file.
    /// In case of a wired device this is the MAC address that will be assigned to the Ethernet interface.
    /// In case of a WiFi device, this is the MAC address of the router or access point that the device should monitor.
    /// If this value is not specified in the configuration file, the default value will be used and the device will connect to
    /// any router or access point.
    /// The default value is { 0, 0, 0, 0, 0, 0 } for WiFi devices and { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 } for wired devices.
    static byte mac[6];
#ifndef USE_WIFI
    /// @brief The time to wait in seconds from startup until the device is ready to connect to the router.
    /// It was observed that some routers assigned a non-valid IP address to the device if the device attempts
    /// to connect to the router too early, so this delay is needed to allow the router to be ready.
    static long routerInitTimeSec;
    /// @brief The time in minutes between updates of the time from the time server.
    /// This is not needed for WiFi devices.
    static long timeUpdatePeriodMin;
#endif
    /// @brief The time in seconds that the DNS server should be available after the device starts up.
    static long dnsAvailTimeSec;
    /// @brief The IP address of the device in the local network.
    /// This is the IP address that the device will use to connect to the router. If it remains in the default value of (all zeroes)
    /// it will use DHCP to get an IP address from the router.
    static byte ip[4];
    /// @brief The gateway IP address.
    /// This is the IP address of the router in the local network. If it remains in the default value of (all zeroes)
    /// it will use the default gateway address provided by the router via DHCP.
    static byte gateway[4];
    /// @brief The subnet mask of the local network.
    /// This is the subnet mask of the local network. If it remains in the default value of (all zeroes)
    /// it will use the default subnet mask provided by the router via DHCP.
    static byte mask[4];
    /// @brief The time zone offset in seconds from UTC.
    /// This is the time zone offset that the device will use to calculate the local time. The value is in seconds and can be positive or negative.
    /// For example, for UTC+2 it will be 2 * 60 * 60 (2 hours in seconds), and for UTC-5 it will be -5 * 60 * 60 (5 hours in seconds).
    static long timeZone;
    /// @brief The Daylight Saving Time (DST) offset in seconds.
    static long DST;
    /// @brief The time server to use for getting the current time.
    /// This is the domain name or IP address of the time server that the device will use to
    static const char *timeServer;
    /// @brief The GPIO pin number for the modem relay.
    static byte modemRelay;
    /// @brief The GPIO pin number for the router relay.
    static byte routerRelay;
    /// @brief If true, the device is a single device (e.g., router only) and not a dual device (router and modem).
    /// If false, the device is a dual device (router and modem).
    static bool singleDevice;
    /// @brief The name of the device.
    /// This is the name that will be used to identify the device in the GUI and in the logs.
    /// It can be set to any string value, but it is recommended to use a meaningful name.
    static const char *deviceName;
    /// @brief The time in seconds to skip the router recovery after router recovery.
    /// This is useful in case of dual devices (router and modem). When the last recovery was
    /// a router recovery and took place within this router skip time, and now connectivity is lost again,
    /// the device will not attempt to recover the router again, but will try to recover the modem first.
    static time_t skipRouterTime;
    /// @brief The period in days to perform a hard reset of the device.
    static long hardResetPeriodDays;
    /// @brief The time of day to perform a hard reset of the device.
    static long hardResetTime;
    /// @brief The OTA server to use for over-the-air updates.
    static const char *otaServer;
#ifdef USE_WIFI
    /// @brief The SSID of the WiFi network to connect to.
    static const char *ssid;
    /// @brief The password of the WiFi network to connect to.
    static const char *password;
    /// @brief The host name of the device in the WiFi network. It registers the name in the mDNS service
    /// so that it can be accessed by the host name in the local network.
    static const char *hostName;
#endif


public:
    /// @brief Initializes the configuration parameters by reading the configuration file from the SD card.
    static void Init();

private:
    /// @brief The name of the configuration file on the SD card.
    /// This file contains the configuration parameters for the device.
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

/// @brief Initializes the configuration parameters.
/// This function is a wrapper for the Config::Init() function to be used in the main application.
void InitConfig();

#endif // Config_h