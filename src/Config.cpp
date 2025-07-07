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

#include <Arduino.h>
#include <Config.h>
#include <SDUtil.h>
#include <PwrCntl.h>
#ifdef DEBUG_CONFIG
#include <Trace.h>
#endif

// This file contains classes and methods to read the content of the configuration file CONFIG.TXT
// from the SD card and parse it into the configuration parameters.

// Default MAC address
byte Config::mac[6] = 
#ifndef USE_WIFI
  /// @brief Default MAC address for wired devices.
  { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
  /// @brief The time to wait in seconds from startup until the device is ready to connect to the router.
  /// it was observed that some routers assigned a non-valid IP address to the device if the device attemps
  /// to connect to the router too early, so this delay is needed to allow the router to be ready.
  /// This is not needed for WiFi devices.
  long Config::routerInitTimeSec = 10;
  /// @brief The time in minutes between updates of the time from the time server.
  /// This is not needed for WiFi devices.
  long Config::timeUpdatePeriodMin = 20;
#else
  /// @brief Default MAC address for WiFi devices. In WiFi devices, the MAC address in the configuration should be set to the MAC address
  /// of the router or access point that it should monitor. If this value is not specified in the configuration file,
  /// the default value will be used and any device will be connected. This is usually useful when there is only one router
  /// to connect to.
  {0, 0, 0, 0, 0, 0};
#endif
/// @brief time in seconds that the DNS server should be available after the device starts up.
long Config::dnsAvailTimeSec = 5 * 60; // 5 Minutes
/// @brief The IP address of the device in the local network.
/// This is the IP address that the device will use to connect to the router. If it remains in the default value of (all zeroes)
/// it will use DHCP to get an IP address from the router.
byte Config::ip[4] = { 0, 0, 0, 0 };
/// @brief The gateway IP address.
/// This is the IP address of the router in the local network. If it remains in the default value of (all zeroes)
/// it will use the default gateway address provided by the router via DHCP.
byte Config::gateway[4] = { 0, 0, 0, 0, };
/// @brief The subnet mask of the local network.
/// This is the subnet mask that the device will use to connect to the router. If it remains in the default value of (all zeroes)
/// it will use the default subnet mask provided by the router via DHCP.
byte Config::mask[4] = { 0, 0, 0, 0 };
/// @brief The time zone offset in seconds from UTC.
/// This is the time zone offset that the device will use to calculate the local time. The value is in seconds and can be positive or negative.
/// For example, for UTC+2 it will be 2 * 60 * 60 (2 hours in seconds), and for UTC-5 it will be -5 * 60 * 60 (5 hours in seconds).
long Config::timeZone = 0;
/// @brief The Daylight Saving Time (DST) offset in seconds.
long Config::DST = 0;
/// @brief The time server to use for getting the current time.
/// This is the domain name or IP address of the time server that the device will use to
const char *Config::timeServer = "time.nist.gov";
/// @brief The name of the configuration file on the SD card.
const char *Config::configFileName = "config.txt";
/// @brief The GPIO pin number for the modem relay.
byte Config::modemRelay = 26;
/// @brief The GPIO pin number for the router relay.
byte Config::routerRelay = 25;
/// @brief If true, the device is a single device (e.g., router only) and not a dual device (router and modem).
bool Config::singleDevice = false; // Default is two devices: router and modem
/// @brief The name of the device.
/// This is the name that will be used to identify the device in the GUI and in the logs.
/// It can be set to any string value, but it is recommended to use a meaningful name.
const char *Config::deviceName = "Router";
/// @brief The time in seconds to skip the router recovery after router recovery.
/// This is usefull in case of dual devices (router and modem). When last recovery was 
/// a router recovery and took place within this router skip time, and now connectivity is lost again,
/// the device will not attempt to recover the router again, but will try to recover the modem first.
/// If modem recovery did not resume connectivity, then it will attempt to recover the router again in 
/// the next recovery cycle.
time_t Config::skipRouterTime = 60 * 60; // One hour
/// @brief The period in days to perform a hard reset of the device.
long Config::hardResetPeriodDays = 3; // 3 days
/// @brief The time of day to perform a hard reset of the device.
long Config::hardResetTime = 3 * 60 * 60; // 3AM
/// @brief The OTA server to use for over-the-air updates.
const char *Config::otaServer = "otadrive.com";
#ifdef USE_WIFI
/// @brief The SSID of the WiFi network to connect to.
const char *Config::ssid /* = "Your SSID" */;
/// @brief The password of the WiFi network to connect to.
const char *Config::password /* = "Your password" */;
/// @brief The host name of the device in the WiFi network. It registers the name in the mDNS service
/// so that it can be accessed by the host name in the local network.
const char *Config::hostName = "InternetRecoveryBox";
#endif

/// @brief Parses a string value from the configuration file.
/// This function allocates memory for the string and copies the value from the configuration file into it.
/// @param configValue The string value from the configuration file.
/// @param strBuffer A pointer to a buffer where the parsed string will be stored.
/// The buffer is allocated by this function and is filled with the parsed string.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseString(const String &configValue, void *strBuffer)
{
  char **str = static_cast<char **>(strBuffer);

  size_t len = strlen(configValue.c_str());
  *str = static_cast<char *>(malloc(len + 1));
  strcpy(*str, configValue.c_str());
  return true;
}

/// @brief Parses a long value from the configuration file.
/// This function reads a long integer from the string and stores it in the provided pointer.
/// @param configValue The string value from the configuration file.
/// @param parsedLong A pointer to a long variable where the parsed value will be stored.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseLong(const String &configValue, void *parsedLong)
{
  return sscanf(configValue.c_str(), "%ld", parsedLong) == 1;
}

/// @brief Parses a byte value from the configuration file.
/// @param strByte The string value representing a byte, which can be in decimal or hexadecimal format.
/// The string can start with "0x" for hexadecimal values.
/// @param value A pointer to a byte variable where the parsed value will be stored.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseByte(const String &strByte, byte *value)
{
  const char *format = strByte.startsWith("0x") ? "%hhx" : "%hhd";

  return sscanf(strByte.c_str(), format, value) == 1;
}

/// @brief Parses a series of byte values from the configuration file.
/// @param strValue The string value containing byte values separated by a specified separator.
/// The string can contain multiple byte values, each represented in decimal or hexadecimal format.
/// @param separator The character that separates the byte values in the string.
/// @param bytes A pointer to an array of bytes where the parsed values will be stored.
/// @param arrayLen The length of the byte array.
/// The function will parse `arrayLen` number of byte values from the string.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseByteArray(const String &strValue, char separator, byte *bytes, int arrayLen)
{
  int index = 0;
  for(int i = 0; i < arrayLen - 1; i++)
  {
    int separatorIndex = strValue.indexOf(separator, index);
    if (separatorIndex == -1)
      return false;
    String byte = strValue.substring(index, separatorIndex);
    if (!ParseByte(byte, bytes + i))
      return false;
    index = separatorIndex + 1;
  }

  return ParseByte(strValue.substring(index), bytes + arrayLen - 1);
}

/// @brief Parses a MAC address from the configuration file.
/// The MAC address is expected to be in the format of six byte values separated by commas.
/// @param configValue The string value from the configuration file representing the MAC address.
/// @param parsedMACAddress A pointer to a byte array where the parsed MAC address will be stored.
/// The array should be at least 6 bytes long to hold the MAC address.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseMACAddress(const String &configValue, void *parsedMACAddress)
{
  return ParseByteArray(configValue, ',', static_cast<byte *>(parsedMACAddress), 6);
}

/// @brief Parses an IP address from the configuration file.
/// The IP address is expected to be in the format of four byte values separated by dots.
/// @param configValue The string value from the configuration file representing the IP address.
/// @param parsedIPAddress A pointer to a byte array where the parsed IP address will be stored
/// The array should be at least 4 bytes long to hold the IP address.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseIPAddress(const String &configValue, void *parsedIPAddress)
{
  return ParseByteArray(configValue, '.', static_cast<byte *>(parsedIPAddress), 4);
}

/// @brief Parses a byte value from the configuration file.
/// This function is a wrapper for the ParseByte function that takes a void pointer as an argument
/// and casts it to a byte pointer before calling ParseByte.
/// @param configValue The string value from the configuration file representing the byte value.
/// @param parsedByte A pointer to a byte variable where the parsed value will be stored.
/// @return True if the parsing was successful, false otherwise.
bool Config::ParseByte(const String &configValue, void *parsedByte)
{
  return ParseByte(configValue, static_cast<byte *>(parsedByte));
}

/// @brief Parses a boolean value from the configuration file.
/// The boolean value is expected to be either "true" or "false".
/// @param configValue The string value from the configuration file representing the boolean value.
/// @param parsedBoolean A pointer to a boolean variable where the parsed value will be stored.
/// The variable will be set to true if the value is "true", and false if the value is "false".
/// @note The function does not handle case sensitivity, so the values must be exactly "true" or "false".
/// If the value is not recognized, the function returns false.
/// @return True if the parsing was successful (the value is either "true" or "false"), false otherwise.
bool Config::ParseBoolean(const String &configValue, void *parsedBoolean)
{
  bool *parsedBooleanBuff = static_cast<bool *>(parsedBoolean);
  
  if (configValue == "true")
  {
    *parsedBooleanBuff = true;
    return true;
  } else if (configValue == "false")
  {
    *parsedBooleanBuff = false;
    return true;
  } else 
  {
    return false;
  }
}

/// @brief Parses a time value from the configuration file.
/// The time value is expected to be in the format "HH:MM", where HH is the hour (0-23) and MM is the minute (0-59).
/// The function converts the time into seconds since midnight and stores it in the provided pointer.
/// @param configValue The string value from the configuration file representing the time.
/// @param parsedTime A pointer to an unsigned long variable where the parsed time in seconds will be stored.
/// The variable will contain the number of seconds since midnight (00:00:00) after parsing.
/// The value will be calculated as (HH * 3600 + MM * 60).
/// @return True if the parsing was successful (the format is correct and the values are within valid ranges), false otherwise.
bool Config::ParseTime(const String &configValue, void *parsedTime)
{
  u_int h, m;

  if (sscanf(configValue.c_str(), "%d:%d", &h, &m) != 2)
    return false;

  if (h > 23 || m > 60)
    return false;

  *(static_cast<unsigned long *>(parsedTime)) = (h * 60 + m) * 60;

  return true;
}

/// @brief Initializes the configuration parameters by reading the configuration file from the SD card.
/// This function opens the configuration file, reads its content line by line, and parses each line
void Config::Init()
{
  // Initialize the SD card
  AutoSD autoSD;
  /// @brief A configuration line parser definition
  struct ConfigLineParser
  {
    /// @brief The name of the configuration variable to parse.
    String configVarName;
    /// @brief A function pointer to the parser function that will parse the configuration value.
    bool (*parser)(const String &configValue, void *parsedValue);
    /// @brief A pointer to the parameter where the parsed value will be stored.
    void *param;
  };

  /// @brief An array of configuration line parsers.
  /// Each entry in the array defines a configuration variable, its parser function, and the parameter
  /// where the parsed value will be stored.
  ConfigLineParser parsers[] =
  {
    { String("TimeServer"), ParseString, &timeServer},
    { String("TimeZone"), ParseLong, &timeZone },
    { String("DST"), ParseLong, &DST },
    { String("DNSAvailTimeSec"), ParseLong, &dnsAvailTimeSec },
    { String("MACAddress"), ParseMACAddress, mac },
  #ifndef USE_WIFI
    { String("RouterInitTimeSec"), ParseLong, &routerInitTimeSec },
    { String("TimeUpdatePeriodMin"), ParseLong, &timeUpdatePeriodMin },
  #endif
    { String("IP"), ParseIPAddress, ip },
    { String("Gateway"), ParseIPAddress, gateway },
    { String("Subnet"), ParseIPAddress, mask },
    { String("ModemRelay"), ParseByte, &modemRelay },
    { String("RouterRelay"), ParseByte, &routerRelay },
    { String("SingleDevice"), ParseBoolean, &singleDevice },
    { String("DeviceName"), ParseString, &deviceName },
    { String("SkipRouterTime"), ParseLong, &skipRouterTime },
    { String("HardResetPeriodDays"), ParseLong, &hardResetPeriodDays },
    { String("HardResetTime"), ParseTime, &hardResetTime },
    { String("OTAServer"), ParseString, &otaServer },
  #ifdef USE_WIFI
    { String("SSID"), ParseString, &ssid },
    { String("Password"), ParseString, &password },
    { String("HostName"), ParseString, &hostName },
  #endif
  };

  // Open the configuration file
  String configFilePath = String("/") + configFileName;
#ifdef DEBUG_CONFIG
  TRACE_BLOCK
  {
    Trace("Config file: ");
    Traceln(configFilePath);
  }
#endif

  SdFile config = SD.open(configFilePath);

  // Check if the configuration file was opened successfully
  if (!config)
  {
#ifdef DEBUG_CONFIG
    Traceln("Failed to open configuration file");
#endif
    // If the configuration file cannot be opened, reset the device.
    HardReset(0);
    return;
  }

  // Read the configuration file line by line
  bool eof = false;
  do
  {
    bool eol = false;
    String configLine = "";
    bool skipLine = false; // If the line is empty or starts with a comment, skip it.
    // Read characters from the configuration file until the end of the line or end of file
    do
    {
      // Read the next character from the configuration file
      // The read() method returns -1 if the end of the file is reached.
      int16_t nextChar = config.read();
      switch(nextChar)
      {
      case -1:
        // If the end of the file is reached, set eof and eol flags to true
        // and break the loop to stop reading further.
        eol = true;
        eof = true;
        break;

      case '\n':
        // If a newline character is encountered, set the eol flag to true
        // and break the loop to stop reading further.
        eol = true;
        break;

      case '\r':
        // If a carriage return character is encountered, skip it.
        // This is to handle different line endings (CRLF vs LF).
        continue;

      case ';': // Comment
        // If a semicolon is encountered, it indicates the start of a comment.
        // Set the skipLine flag to true to stop reading further characters in this line.
        skipLine = configLine.isEmpty();
      default:
        // For any other character, in case this is not a comment line, append it to the configLine string.
        if (!skipLine)
          configLine += (char)nextChar;
        break;
      }
    } while (!eol); // Continue reading until the end of the line or end of file.
    // If the line is empty or starts with a comment, skip it.
    if (configLine.equals(""))
    {
      continue;
    }
#ifdef DEBUG_CONFIG
    // If debugging is enabled, print the configuration line to the trace output.
    TRACE_BLOCK
    {
      Trace("Config line:");
      Traceln(configLine.c_str());
    }
#endif
    // Find the index of the first '=' character in the configuration line.
    // This character separates the configuration variable name from its value.
    int sepIndex = configLine.indexOf('=');
    if (sepIndex == -1)
    {
      // If no '=' character is found, it means the line is not a valid configuration line.
      // Log an error message and continue to the next line.
#ifdef DEBUG_CONFIG
      LOCK_TRACE;
      Trace("Invalid configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }
    // Extract the configuration variable name and value from the line.
    // The variable name is the substring before the '=' character, and the value is the substring
    String configName = configLine.substring(0, sepIndex);
    // Extract the configuration value, which is the substring after the '=' character.
    // The value can contain spaces, so it is taken as the rest of the line.
    String configValue = configLine.substring(sepIndex + 1);
    
    if (configName.equals("") || configValue.equals(""))
    {
      // If either the configuration variable name or value is empty, it is not a valid configuration line.
      // Log an error message and continue to the next line.
#ifdef DEBUG_CONFIG
      LOCK_TRACE;
      Trace("Invalid configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }

    // Find the parser for the configuration variable name in the parsers array.
    size_t i = 0;
    for (; i < NELEMS(parsers); i++)
      if (parsers[i].configVarName.equals(configName))
        break;
    if (i == NELEMS(parsers))
    {
      // If no parser is found for the configuration variable name, it means the variable is not recognized.
      // Log an error message and continue to the next line.
#ifdef DEBUG_CONFIG
      LOCK_TRACE;
      Trace("Unrecognized configuration variable: ");
      Traceln(configName);
#endif
      continue;
    }
    // If a parser is found, call the parser function with the configuration value and the parameter where the parsed value will be stored.
    // The parser function will parse the value and store it in the provided parameter.
    if (!parsers[i].parser(configValue, parsers[i].param))
    {
      // If the parser function returns false, it means the parsing failed.
      // Log an error message and continue to the next line.
#ifdef DEBUG_CONFIG
      LOCK_TRACE;
      Trace("Failed to parse configuration value for configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }

  } while (!eof); // Continue reading lines until the end of the file is reached.

  // Close the configuration file after reading all lines.
  config.close();
}

void InitConfig()
{
    Config::Init();
}