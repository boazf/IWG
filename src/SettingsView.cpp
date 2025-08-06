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

#include <Common.h>
#include <SettingsView.h>
#include <Config.h>
#include <AppConfig.h>
#include <EthernetUtil.h>
#include <Version.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

SettingsView::SettingsView(const char *_viewFile) : 
    HtmlFillerView(_viewFile, getFillers)
{
}

ViewFiller SettingsView::fillers[] =
{
    /*  0 */ [](String &fill){ fill = AppConfig::getAutoRecovery() ?  "checked=\"checked\" />" : " />"; },
    /*  1 */ [](String &fill)
    { 
        IPAddress lanAddress = AppConfig::getLANAddr(); 
        fill = IsZeroIPAddress(lanAddress) ? "" : String(lanAddress[0]) + "." + lanAddress[1] + "." + lanAddress[2] + "." + lanAddress[3]; 
        fill = "\"" + fill + "\" />";
    },
    /*  2 */ [](String &fill){ fill = "\"" + AppConfig::getServer1() + "\" />"; },
    /*  3 */ [](String &fill){ fill = "\"" + AppConfig::getServer2() + "\" />"; },
    /*  4 */ [](String &fill){ fill = String("\"") + AppConfig::getRDisconnect() + "\" />"; },
    /*  5 */ [](String &fill){ fill = String("\"") + AppConfig::getMDisconnect() + "\" />"; },
    /*  6 */ [](String &fill){ fill = String("\"") + AppConfig::getRReconnect() + "\" />"; },
    /*  7 */ [](String &fill){ fill = String("\"") + AppConfig::getMReconnect() + "\" />"; },
    /*  8 */ [](String &fill){ fill = String("\"") + AppConfig::getConnectionTestPeriod() + "\" />"; },
    /*  9 */ [](String &fill){ fill = AppConfig::getLimitCycles() ? "checked=\"checked\" />" : " />"; },
    /* 10 */ [](String &fill){ fill = String("\"") + AppConfig::getRecoveryCycles() + "\" />"; },
    /* 11 */ [](String &fill){ fill = String("\"") + AppConfig::getMaxHistory() + "\" />"; },
    /* 12 */ [](String &fill){ fill = AppConfig::getLimitCycles() ? "'True'" : "'False'"; },
    /* 13 */ [](String &fill){ fill = AppConfig::getDST() ? "checked=\"checked\" />" : " />"; },
    /* 14 */ [](String &fill){ fill = Config::singleDevice ? "none" : "visible"; },
    /* 15 */ [](String &fill){ fill = Config::deviceName; },
    /* 16 */ [](String &fill){ fill = AppConfig::getPeriodicallyRestartRouter() ? "checked=\"checked\" />" : " />"; },
    /* 17 */ [](String &fill){ fill = AppConfig::getPeriodicallyRestartModem() ? "checked=\"checked\" />" : " />"; },
    /* 18 */ [](String &fill){ time_t t = AppConfig::getPeriodicRestartTime(); int h = t / 3600; int m =  (t % 3600) / 60; char buff[8]; sprintf(buff, "\"%02u:%02u\"%c", h, m, '\0'); fill = String(buff); },
    /* 19 */ [](String &fill){ fill = Version::getCurrentVersion(); },
    /* 20 */ [](String &fill){ fill = appBase(); },
};

int SettingsView::getFillers(const ViewFiller *&_fillers)
{
    _fillers = fillers;
    return NELEMS(fillers);
}

bool SettingsView::parseBool(const String &val)
{
    return val.equals("true");
}

IPAddress SettingsView::parseIPAddress(const String &val)
{
    int b1, b2, b3, b4;
    if (val.equals(""))
        b1 = b2 = b3 = b4 = 0;
    else
        sscanf(val.c_str(), "%d.%d.%d.%d", &b1, &b2, &b3, &b4);

    return IPAddress(b1, b2, b3, b4);
}

int SettingsView::parseInt(const String &val)
{
    int ret;

    sscanf(val.c_str(), "%d", &ret);

    return ret;
}

time_t SettingsView::parseTime(const String &val)
{
    int h, m;
    // Parse the time in the format "HH%3AMM" (where %3A is URL-encoded ':').
    if (sscanf(val.c_str(), "%d%%3A%d", &h, &m) != 2)
        return -1;

    return (h * 60 + m) * 60;
}

#define X(a) { #a, settingsKeys::a },
SettingsView::SettingsMap SettingsView::settingsMap = 
{
    SETTINGS_KEYS
};
#undef X

void SettingsView::SetConfigValue(const String &pair, SettingsValuesSetMap &settingsValuesSetMap)
{
    if (pair.equals(""))
        return;

    // Parse the key-value pair from the form data.
    int eqIndex = pair.indexOf('=');
    String var = pair.substring(0, eqIndex);
    String val = pair.substring(eqIndex + 1);

    // Find the corresponding settings key in the settingsMap.
    // If the key is not found, log an error and return.
    SettingsMap::const_iterator i = settingsMap.find(var.c_str());
    if (i == settingsMap.end())
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Unknown settings key variable: %s\n", var.c_str());
#endif
        return;
    }

    // Set the configuration value based on the settings key.
    // If the setting was already set by the user, skip setting it again.
    // This is to avoid overwriting a value that was already set in the form.
    switch (i->second)
    {
    case settingsKeys::enableAutoRecovery:
        if (!settingsValuesSetMap[settingsKeys::enableAutoRecovery])
            AppConfig::setAutoRecovery(parseBool(val));
        settingsValuesSetMap[settingsKeys::enableAutoRecovery] = true;
        break;
    case settingsKeys::lanAddressForConnectionTesting:
        AppConfig::setLANAddr(parseIPAddress(val));
        break;
    case settingsKeys::serverForConnectionTesting:
        AppConfig::setServer1(val);
        break;
    case settingsKeys::server2ForConnectionTesting:
        AppConfig::setServer2(val);
        break;
    case settingsKeys::routerDisconnectTime:
        AppConfig::setRDisconnect(parseInt(val));
        break;
    case settingsKeys::modemDisconnectTime:
        AppConfig::setMDisconnect(parseInt(val));
        break;
    case settingsKeys::connectionTestPeriod:
        AppConfig::setConnectionTestPeriod(parseInt(val));
        break;
    case settingsKeys::routerReconnectTime:
        AppConfig::setRReconnect(parseInt(val));
        break;
    case settingsKeys::modemReconnectTime:
        AppConfig::setMReconnect(parseInt(val));
        break;
    case settingsKeys::limitRecoveryCycles:
        if (!settingsValuesSetMap[settingsKeys::limitRecoveryCycles])
            AppConfig::setLimitCycles(parseBool(val));
        settingsValuesSetMap[settingsKeys::limitRecoveryCycles] = true;
        break;
    case settingsKeys::recoveryCycles:
        AppConfig::setRecoveryCycles(parseInt(val));
        break;
    case settingsKeys::daylightSavingTime:
        if (!settingsValuesSetMap[settingsKeys::daylightSavingTime])
            AppConfig::setDST(parseBool(val));
        settingsValuesSetMap[settingsKeys::daylightSavingTime] = true;
        break;
    case settingsKeys::maxHistoryRecords:
        AppConfig::setMaxHistory(parseInt(val));
        break;
    case settingsKeys::periodicallyRestartRouter:
        if (!settingsValuesSetMap[settingsKeys::periodicallyRestartRouter])
            AppConfig::setPeriodicallyRestartRouter(parseBool(val));
        settingsValuesSetMap[settingsKeys::periodicallyRestartRouter] = true;
        break;
    case settingsKeys::periodicallyRestartModem:
        if (!settingsValuesSetMap[settingsKeys::periodicallyRestartModem])
            AppConfig::setPeriodicallyRestartModem(parseBool(val));
        settingsValuesSetMap[settingsKeys::periodicallyRestartModem] = true;
        break;
    case settingsKeys::periodicRestartTime:
        AppConfig::setPeriodicRestartTime(parseTime(val));
        break;
    }
}

bool SettingsView::Post(HttpClientContext &context, const String id)
{
    String pair = "";

    SettingsValuesSetMap settingsValuesSetMap =
    {
        { settingsKeys::enableAutoRecovery, false },
        { settingsKeys::limitRecoveryCycles, false },
        { settingsKeys::daylightSavingTime, false },
        { settingsKeys::periodicallyRestartRouter, false },
        { settingsKeys::periodicallyRestartModem, false }
    };

    EthClient client = context.getClient();

    // Read the form data from the message body.
    // The form data is expected to be in the format "key1=value1&key2=value2&...".
    // We read until we reach the end of the message body.
    while(client.available())
    {
        char c = client.read();
        if (c != '&')
        {
            // If the character is not an '&', append it to the current pair.
            pair += c;
            continue;
        }

        // If we reach an '&', it means we have a complete key-value pair.
        // Set the configuration value for the current pair.
        SetConfigValue(pair, settingsValuesSetMap);
        pair = "";
    }

    // If there is a remaining pair after reading the message body, set its value.
    SetConfigValue(pair, settingsValuesSetMap);
    // Commit the changes to the AppConfig.
    // This will save the configuration values to EEPROM.
    // It is important to call this after all settings have been set for efficiency.
    AppConfig::commit();

    // Send a response to the client indicating that the settings have been saved.
    // We send a 302 Found status code to redirect the client to the index page.
    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Location", "/index"} };
    HttpHeaders headers(client);
    headers.sendHeaderSection(302, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}
