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

#ifndef AppConfig_h
#define AppConfig_h

#include <Arduino.h>
#include <IPAddress.h>
#include <EEPROM.h>
#include <time.h>
#include <Observers.h>

#define MAX_SERVER_NAME 64
#define APP_CONFIG_EEPROM_START_ADDR 0

/// @brief This class is used to notify observers when the application configuration changes.
class AppConfigChangedParam
{
public:
    AppConfigChangedParam()
    {
    }
};

/// @brief This structure holds the application configuration parameters that are stored in EEPROM.
typedef struct AppConfigStore_
{
    int mDisconnect;    // Seconds to wait while the modem power is disconnected and then reconnected
    int rDisconnect;    // Seconds to wait while the router power is disconnected and then reconnected
    int mReconnect;     // Maximum seconds to wait for connectivity to resume after modem power is reconnected
    int rReconnect;     // Maximum seconds to wait for connectivity to resume after router power is reconnected
    bool limitCycles;   // If true, the number of recovery cycles is limited to the value of recoveryCycles
    int recoveryCycles; // Number of recovery cycles to perform before giving up
    char server1[MAX_SERVER_NAME + 1]; // Name of the first server to ping in order to verify connectivity
    char server2[MAX_SERVER_NAME + 1]; // Name of the second (fall back) server to ping in order to verify connectivity
    byte lanAddress[4]; // LAN address of the device, used for local network connectivity checks
    time_t connectionTestPeriod; // Period in seconds to wait between connectivity tests
    bool autoRecovery; // If true, the application will automatically recover from connectivity issues
    // If false, the application will not attempt to recover from connectivity issues and will require manual intervention
    // to restore connectivity. However, the application will still attempt to ping the servers to verify connectivity and 
    // will update the user interface and other components of the application with the current connectivity status.
    int maxHistory; // Maximum number of history entries to keep in the history log
    bool DST; // If true, the application will assume Daylight Saving Time (DST) when calculating the current time
    bool initialized; // If true, the application configuration has been initialized and is valid.
    bool periodicallyRestartRouter; // If true, the router will be periodically restarted to maintain connectivity
    bool periodicallyRestartModem;  // If true, the modem will be periodically restarted to maintain connectivity
    // This is useful for devices that have a tendency to lose connectivity after a period of time
    time_t periodicRestartTime; // The time of day when the next periodic restart will occur.

} AppConfigStore;


/// @brief This class manages the application configuration stored in EEPROM.
/// It provides methods to initialize, get, set, and commit configuration values.
/// It holds the user defined configuration parameters for the application set by the GUI.
class AppConfig
{
public:
    /// @brief If the EEPROM configuration is already initialized, it will read the values from EEPROM to the `AppConfigStore`.
    /// If the EEPROM configuration is not initialized, it will set the default values in the EEPROM and then read them into the `AppConfigStore`.
    static void init();
    /// @brief Retrieves the modem disconnect time in seconds.
    /// @return The modem disconnect time in seconds.
    static int getMDisconnect();
    /// @brief Sets the modem disconnect time in seconds
    /// @param value The value to set
    static void setMDisconnect(int value);
    /// @brief Retrieves the router disconnect time in seconds
    /// @return The modem disconnect time in seconds
    static int getRDisconnect();
    /// @brief Sets the router disconnect time in seconds
    /// @param value The value to set
    static void setRDisconnect(int value);
    /// @brief Retrieves the modem reconnect time in seconds
    /// @return The modem reconnect time in seconds
    static int getMReconnect();
    /// @brief sets the modem reconnect time in seconds
    /// @param value The value to set
    static void setMReconnect(int value);
    /// @brief Retrieves the router reconnect time in seconds
    /// @return The router reconnect time in seconds
    static int getRReconnect();
    /// @brief Sets the router reconnect time in seconds
    /// @param value The value to set
    static void setRReconnect(int value);
    /// @brief Retrieves whether the number of recovery cycles is limited
    /// @return True if the number of recovery cycles is limited, false otherwise
    static bool getLimitCycles();
    /// @brief Sets whether the number of recovery cycles is limited
    /// @param value True to limit the number of recovery cycles, false otherwise
    static void setLimitCycles(bool value);
    /// @brief Retrieves the number of recovery cycles to perform before giving up
    /// @return The number of recovery cycles
    static int getRecoveryCycles();
    /// @brief Sets the number of recovery cycles to perform before giving up
    /// @param value The number of recovery cycles to set
    static void setRecoveryCycles(int value);
    /// @brief Retrieves the first server name used for connectivity checks
    /// @return The first server name as a String
    static String getServer1();
    /// @brief Sets the first server name used for connectivity checks
    /// @param value The server name to set as a String
    static void setServer1(const String &value);
    /// @brief Retrieves the second server name used for connectivity checks
    /// This is a fallback server that is used if the first server is not reachable.
    /// @return The second server name as a String
    static String getServer2();
    /// @brief Sets the second server name used for connectivity checks
    /// @param value The server name to set as a String
    static void setServer2(const String &value);
    /// @brief Retrieves the LAN address of the device
    /// This address is used for local network connectivity checks.
    /// @return The LAN address as an IPAddress object
    static IPAddress getLANAddr();
    /// @brief Sets the LAN address of the device
    /// @param value The LAN address to set as an IPAddress object
    static void setLANAddr(const IPAddress &value);
    /// @brief Retrieves the period in seconds to wait between connectivity tests
    /// @return The connection test period in seconds
    static time_t getConnectionTestPeriod();
    /// @brief Sets the period in seconds to wait between connectivity tests
    /// @param value The connection test period to set in seconds
    static void setConnectionTestPeriod(time_t value);
    /// @brief Retrieves whether the application will automatically recover from connectivity issues
    /// @return True if auto recovery is enabled, false otherwise
    static bool getAutoRecovery();
    /// @brief Sets whether the application will automatically recover from connectivity issues
    /// @param value True to enable auto recovery, false to disable it
    static void setAutoRecovery(bool value);
    /// @brief Retrieves the maximum number of history entries to keep in the history log
    /// @return The maximum number of history entries
    static int getMaxHistory();
    /// @brief Sets the maximum number of history entries to keep in the history log
    /// @param value The maximum number of history entries to set
    static void setMaxHistory(int value);
    /// @brief Retrieves whether the application assumes Daylight Saving Time (DST)
    /// @return True if DST is enabled, false otherwise
    static bool getDST();
    /// @brief Sets whether the application assumes Daylight Saving Time (DST)
    /// @param value True to enable DST, false to disable it
    static void setDST(bool value);
    /// @brief Retrieves whether the router will be periodically restarted to maintain connectivity
    /// @return True if the router will be periodically restarted, false otherwise
    static bool getPeriodicallyRestartRouter();
    /// @brief Sets whether the router will be periodically restarted to maintain connectivity
    /// @param value True to enable periodic router restarts, false to disable it
    static void setPeriodicallyRestartRouter(bool value);
    /// @brief Retrieves whether the modem will be periodically restarted to maintain connectivity
    /// @return True if the modem will be periodically restarted, false otherwise
    static bool getPeriodicallyRestartModem();
    /// @brief Sets whether the modem will be periodically restarted to maintain connectivity
    /// @param value True to enable periodic modem restarts, false to disable it
    static void setPeriodicallyRestartModem(bool value);
    /// @brief Retrieves the time of day when the periodic restart will occur
    /// @return The periodic restart time as a time_t value
    static time_t getPeriodicRestartTime();
    /// @brief Sets the time of day when the periodic restart will occur
    /// @param value The periodic restart time to set as a time_t value
    static void setPeriodicRestartTime(time_t value);
    /// @brief Retrieves the observer obejct that notifies when the application configuration changes.
    /// This observer can be used to listen for changes in the application configuration.
    /// @return The observer object to enlist for application configuration changes
    static Observers<AppConfigChangedParam> &getAppConfigChanged()
    {
        return appConfigChanged;
    }
    /// @brief Commits the application configuration to EEPROM.
    /// This method writes the current configuration values to EEPROM and notifies observers of the change.
    static void commit();

private:
    static AppConfigStore store;
    static Observers<AppConfigChangedParam> appConfigChanged;

private:
    /// @brief This method checks if the application configuration has been initialized.
    /// @return True if the configuration is initialized, false otherwise. 
    static bool isInitialized();
    /// @brief This method sets the application configuration as initialized or uninitialized.
    /// @param isInitialized True to mark the configuration as initialized, false as uninitialized.
    static void setInitialized(bool isInitialized);
    /// @brief Reads the value of the modem disconnect time from EEPROM.
    /// @return The modem disconnect time in seconds.
    static int internalGetMDisconnect();
    /// @brief Writes the value of the modem disconnect time to EEPROM.
    /// @param value The modem disconnect time in seconds to set.
    static void internalSetMDisconnect(int value);
    /// @brief Reads the value of the router disconnect time from EEPROM.
    /// @return The router disconnect time in seconds.
    static int internalGetRDisconnect();
    /// @brief Writes the value of the router disconnect time to EEPROM.
    /// @param value The router disconnect time in seconds to set.
    static void internalSetRDisconnect(int value);
    /// @brief Reads the value of the modem reconnect time from EEPROM.
    /// @return The modem reconnect time in seconds.
    static int internalGetMReconnect();
    /// @brief Writes the value of the modem reconnect time to EEPROM.
    /// @param value The modem reconnect time in seconds to set.
    static void internalSetMReconnect(int value);
    /// @brief Reads the value of the router reconnect time from EEPROM.
    /// @return The router reconnect time in seconds.
    static int internalGetRReconnect();
    /// @brief Writes the value of the router reconnect time to EEPROM.
    /// @param value The router reconnect time in seconds to set.
    static void internalSetRReconnect(int value);
    /// @brief Reads whether the number of recovery cycles is limited from EEPROM.
    /// @return True if the number of recovery cycles is limited, false otherwise.
    static bool internalGetLimitCycles();
    /// @brief Writes whether the number of recovery cycles is limited to EEPROM.
    /// @param value True to limit the number of recovery cycles, false otherwise.
    static void internalSetLimitCycles(bool value);
    /// @brief Reads the number of recovery cycles to perform before giving up from EEPROM.
    /// @return The number of recovery cycles.
    static int internalGetRecoveryCycles();
    /// @brief Writes the number of recovery cycles to perform before giving up to EEPROM.
    /// @param value The number of recovery cycles to set.
    static void internalSetRecoveryCycles(int value);
    /// @brief Reads the first server name used for connectivity checks from EEPROM.
    /// @return The first server name as a String.
    static String internalGetServer1();
    /// @brief Writes the first server name used for connectivity checks to EEPROM.
    /// @param value The server name to set as a String.
    static void internalSetServer1(const String &value);
    /// @brief Reads the second server name used for connectivity checks from EEPROM.
    /// @return The second server name as a String.
    static String internalGetServer2();
    /// @brief Writes the second server name used for connectivity checks to EEPROM.
    /// @param value The server name to set as a String.
    static void internalSetServer2(const String &value);
    /// @brief Reads the LAN address of the device with which to check connectivity from EEPROM.
    /// @return The LAN address as an IPAddress object.
    static IPAddress internalGetLANAddr();
    /// @brief Writes the LAN address of the device with which to check connectivity to EEPROM.
    /// @param value The LAN address to set as an IPAddress object.
    static void internalSetLANAddr(const IPAddress &value);
    /// @brief Reads the period in seconds to wait between connectivity tests from EEPROM.
    /// @return The connection test period in seconds.
    static time_t internalGetConnectionTestPeriod();
    /// @brief Writes the period in seconds to wait between connectivity tests to EEPROM.
    /// @param value The connection test period to set in seconds.
    static void internalSetConnectionTestPeriod(time_t value);
    /// @brief Reads whether the application will automatically recover from connectivity issues from EEPROM.
    /// @return True if auto recovery is enabled, false otherwise.
    static bool internalGetAutoRecovery();
    /// @brief Writes whether the application will automatically recover from connectivity issues to EEPROM.
    /// @param value True to enable auto recovery, false to disable it.
    static void internalSetAutoRecovery(bool value);
    /// @brief Reads the maximum number of history entries to keep in the history log from EEPROM.
    /// @return The maximum number of history entries.
    static int internalGetMaxHistory();
    /// @brief Writes the maximum number of history entries to keep in the history log to EEPROM.
    /// @param value The maximum number of history entries to set.
    static void internalSetMaxHistory(int value);
    /// @brief Reads whether the application assumes Daylight Saving Time (DST) from EEPROM.
    /// @return True if DST is enabled, false otherwise.
    static bool internalGetDST();
    /// @brief Writes whether the application assumes Daylight Saving Time (DST) to EEPROM. 
    /// @param value True to enable DST, false to disable it.
    static void internalSetDST(bool value);
    /// @brief Reads whether the router will be periodically restarted to maintain connectivity from EEPROM.
    /// @return True if the router will be periodically restarted, false otherwise.
    static bool internalGetPeriodicallyRestartRouter();
    /// @brief Writes whether the router will be periodically restarted to maintain connectivity to EEPROM.
    /// @param value True to enable periodic router restarts, false to disable it.
    static void internalSetPeriodicallyRestartRouter(bool value);
    /// @brief Reads whether the modem will be periodically restarted to maintain connectivity from EEPROM.
    /// @return True if the modem will be periodically restarted, false otherwise.
    static bool internalGetPeriodicallyRestartModem();
    /// @brief Writes whether the modem will be periodically restarted to maintain connectivity to EEPROM.
    /// @param value True to enable periodic modem restarts, false to disable it.
    static void internalSetPeriodicallyRestartModem(bool value);
    /// @brief Reads the time of day when the periodic restart will occur from EEPROM.
    /// @return The periodic restart time as a time_t value.
    static time_t internalGetPeriodicRestartTime();
    /// @brief Writes the time of day when the periodic restart will occur to EEPROM.
    /// @param value The periodic restart time to set as a time_t value.
    static void internalSetPeriodicRestartTime(time_t value);
    /// @brief Reads a field from EEPROM at the specified offset and returns the value.
    /// @tparam T The type of the field to read.
    /// @param offset The offset in bytes from the APP_CONFIG_EEPROM_START_ADDR where the field is located.
    /// @param value A reference to a variable where the read value will be stored.
    /// @return The value read from EEPROM.
    template<typename T>
    static T &getField(int offset, T &value)
    {
        return EEPROM.get<T>(APP_CONFIG_EEPROM_START_ADDR + offset, value);
    }

    /// @brief Writes a field to EEPROM at the specified offset.
    /// @tparam T The type of the field to write.
    /// @param offset The offset in bytes from the APP_CONFIG_EEPROM_START_ADDR where the field is located.
    /// @param value The value to write to EEPROM.
    template<typename T>
    static void putField(int offset, const T &value)
    {
        EEPROM.put<T>(APP_CONFIG_EEPROM_START_ADDR + offset, value);
    }
};

/// @brief Initializes the application configuration by reading the values from EEPROM.
/// If the EEPROM configuration is not initialized, it will set the default values.
void InitAppConfig();

#endif // AppConfig_h