#include <Common.h>
#include <SettingsView.h>
#include <AppConfig.h>
#include <EthernetUtil.h>

SettingsView::SettingsView(const char *_viewName, const char *_viewFile) : 
    View(_viewName, _viewFile)
{
}

static ViewFiller fillers[] = 
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
    /* 12 */ [](String &fill){ fill = AppConfig::getLimitCycles() ? "'True'" : "'False'"; }
};

bool SettingsView::DoFill(int nFill, String &fill)
{
    if ((size_t)nFill >= NELEMS(fillers))
        return false;
    fillers[nFill](fill);
    return true;
}

bool parseBool(String &val)
{
    return val.equals("true");
}

IPAddress parseIPAddress(String &val)
{
    int b1, b2, b3, b4;
    if (val.equals(""))
        b1 = b2 = b3 = b4 = 0;
    else
        sscanf(val.c_str(), "%d.%d.%d.%d", &b1, &b2, &b3, &b4);

    return IPAddress(b1, b2, b3, b4);
}

int parseInt(String &val)
{
    int ret;

    sscanf(val.c_str(), "%d", &ret);

    return ret;
}

static void SetConfigValue(String &pair, bool &autoRecovery, bool &limitCycles)
{
    if (pair.equals(""))
        return;

    int eqIndex = pair.indexOf('=');
    String var = pair.substring(0, eqIndex);
    String val = pair.substring(eqIndex + 1);

    if (var.equals("EnableAutoRecovery"))
    {
        if (!autoRecovery)
            AppConfig::setAutoRecovery(parseBool(val));
        autoRecovery = true;
    }
    else if (var.equals("LANAddressForConnectionTesting"))
        AppConfig::setLANAddr(parseIPAddress(val));
    else if (var.equals("ServerForConnectionTesting"))
        AppConfig::setServer1(val);
    else if (var.equals("Server2ForConnectionTesting"))
        AppConfig::setServer2(val);
    else if (var.equals("RouterDisconnectTime"))
        AppConfig::setRDisconnect(parseInt(val));
    else if (var.equals("ModemDisconnectTime"))
        AppConfig::setMDisconnect(parseInt(val));
    else if (var.equals("ConnectionTestPeriod"))
        AppConfig::setConnectionTestPeriod(parseInt(val));
    else if (var.equals("RouterReconnectTime"))
        AppConfig::setRReconnect(parseInt(val));
    else if (var.equals("ModemReconnectTime"))
        AppConfig::setMReconnect(parseInt(val));
    else if (var.equals("LimitRecoveryCycles"))
    {
        if (!limitCycles)
            AppConfig::setLimitCycles(parseBool(val));
        limitCycles = true;
    }
    else if (var.equals("RecoveryCycles"))
        AppConfig::setRecoveryCycles(parseInt(val));
    else if (var.equals("MaxHistoryRecords"))
        AppConfig::setMaxHistory(parseInt(val));
    AppConfig::commit();
}

bool SettingsView::post(EthernetClient &client, const String &resource, const String &id)
{
    String pair = "";
    bool autoRecovery = false;
    bool limitCycles = false;

    while(client.available())
    {
        char c = client.read();
        if (c != '&')
        {
            pair += c;
            continue;
        }

        SetConfigValue(pair, autoRecovery, limitCycles);
        pair = "";
    }

    SetConfigValue(pair, autoRecovery, limitCycles);

    client.println("HTTP/1.1 302 Found");
    client.println("Location: /index");
    client.println("Server: Arduino");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close"); 
    client.println("Content-Length: 0");
    client.println();

    return true;
}

SettingsView settingsView("/SETTINGS", "/SETTINGS.HTM");