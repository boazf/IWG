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

#ifndef SettingsView_h
#define SettingsView_h

#include <HtmlFillerView.h>
#include <map>

#define SETTINGS_KEYS \
    X(enableAutoRecovery) \
    X(lanAddressForConnectionTesting) \
    X(serverForConnectionTesting) \
    X(server2ForConnectionTesting) \
    X(periodicallyRestartRouter) \
    X(periodicallyRestartModem) \
    X(periodicRestartTime) \
    X(routerDisconnectTime) \
    X(modemDisconnectTime) \
    X(connectionTestPeriod) \
    X(routerReconnectTime) \
    X(modemReconnectTime) \
    X(limitRecoveryCycles) \
    X(recoveryCycles) \
    X(daylightSavingTime) \
    X(maxHistoryRecords)

#define X(a) a,
enum class settingsKeys
{
    SETTINGS_KEYS
};
#undef X

// This class represents a view for settings, allowing users to configure various parameters of the application.
// It inherits from HtmlFillerView to provide a web-based interface for settings management.
// The view is designed to handle GET and POST requests, allowing users to retrieve and update settings.
// It uses a form to collect user input and applies the changes to the application configuration.
// This class implements a POST request handler to process form submissions.
// The GET request handler is inherited from HtmlFillerView, with HtmlFillerViewReader handling the view rendering.
class SettingsView : public HtmlFillerView
{
    // Type definition of a map to hold the settings keys and their value state.
    //If a map entry is true, it means the setting was set by the user.
    // Some settings appear twice in the sent form, only the first one should be evaluated and set the value.
    // The second one should be ignored. So this map holds a boolean indicating whether the form value was already set.
    typedef std::map<settingsKeys, bool> SettingsValuesSetMap;
    // Type definition of a map to convert from key setting string to settingsKeys enum.
    typedef std::map<const std::string, settingsKeys> SettingsMap;

public:
    /// @brief Constructor for SettingsView.
    SettingsView(const char *_viewFile);    
    /// @brief This method is called to handle a GET request.
    /// @param context The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being requested. 
    virtual bool Post(HttpClientContext &context, const String id);
    /// @brief Returns a pointer to a new instance of SettingsView.
    /// @note This method is static and can be used to create a new instance of SettingsView.
    /// @return A pointer to a new SettingsView instance.
    static std::shared_ptr<HttpController> getInstance() { return std::make_shared<SettingsView>("/SETTINGS.HTM"); }

protected:
    /// @brief Get the fillers for the view.
    /// @param fillers The fillers to populate.
    /// @return The number of fillers added.
    static int getFillers(const ViewFiller *&fillers);

private:
    /// @brief A static array that holds the view fillers.
    static ViewFiller fillers[];
    /// @brief A static map that holds the settings keys and their corresponding enum values.
    static SettingsMap settingsMap;

private:
    // Helper methods to parse the form values.
    /// @brief Parses a boolean value from a string.
    /// @param val The string value to parse.
    /// @return True if the value is "true", false otherwise.
    bool parseBool(const String &val);
    /// @brief Parses an IP address from a string.
    /// @param val The string value to parse.
    /// @return The parsed IPAddress object.
    IPAddress parseIPAddress(const String &val);
    /// @brief Parses an integer value from a string.
    /// @param val The string value to parse.
    /// @return The parsed integer value.
    int parseInt(const String &val);
    /// @brief Parses a time value from a string.
    /// @param val The string value to parse.
    /// @return The parsed time_t value.
    time_t parseTime(const String &val);
    /// @brief Sets the configuration value for a specific setting.
    /// @param pair The key-value pair from the form data.
    /// @param settingsValuesSetMap A map that holds a boolean indicating whether the setting was already set.
    void SetConfigValue(const String &pair, SettingsValuesSetMap &settingsValuesSetMap);
};
#endif // SettingsView_h