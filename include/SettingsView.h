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

class SettingsView : public HtmlFillerView
{
    typedef std::map<settingsKeys, bool> SettingsValuesSetMap;
    typedef std::map<const std::string, settingsKeys> SettingsMap;

public:
    SettingsView(const char *_viewFile);     
    virtual bool Post(HttpClientContext &context, const String id);
    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new SettingsView("/SETTINGS.HTM"); }
    
protected:
    static int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];
    static SettingsMap settingsMap;

private:
    bool parseBool(const String &val);
    IPAddress parseIPAddress(const String &val);
    int parseInt(const String &val);
    time_t parseTime(const String &val);
    void SetConfigValue(const String &pair, SettingsValuesSetMap &settingsValuesSetMap);
};
#endif // SettingsView_h