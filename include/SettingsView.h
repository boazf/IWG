#ifndef SettingsView_h
#define SettingsView_h

#include <HtmlFillerView.h>
#include <map>

enum class settingsKeys
{
    enableAutoRecovery,
    lanAddressForConnectionTesting,
    serverForConnectionTesting,
    server2ForConnectionTesting,
    periodicRestartRouter,
    periodicRestartModem,
    periodicRestartTime,
    routerDisconnectTime,
    modemDisconnectTime,
    connectionTestPeriod,
    routerReconnectTime,
    modemReconnectTime,
    limitRecoveryCycles,
    recoveryCycles,
    daylightSavingTime,
    maxHistoryRecords
};

class SettingsView : public HtmlFillerView
{
    typedef std::map<settingsKeys, bool> SettingsValuesSetMap;

public:
    SettingsView(const char *_viewName, const char *_viewFile);     
    bool post(EthClient &client, const String &resource, const String &id);
    
protected:
    int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];
    static std::map<const std::string, settingsKeys> settingsMap;

private:
    bool parseBool(const String &val);
    IPAddress parseIPAddress(const String &val);
    int parseInt(const String &val);
    time_t parseTime(const String &val);
    void SetConfigValue(const String &pair, SettingsValuesSetMap &settingsValuesSetMap);
};

class SettingsViewCreator : public ViewCreator
{
public:
    SettingsViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }

    View *createView()
    {
        return new SettingsView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern SettingsViewCreator settingsViewCreator;

#endif // SettingsView_h