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
    SettingsView(const char *_viewName, const char *_viewFile);     
    bool post(EthClient &client, const String &resource, const String &id);
    
protected:
    int getFillers(const ViewFiller *&fillers);

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