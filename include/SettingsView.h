#ifndef SettingsView_h
#define SettingsView_h

#include <HtmlFillerView.h>

class SettingsView : public HtmlFillerView
{
public:
    SettingsView(const char *_viewName, const char *_viewFile);     
    bool post(EthernetClient &client, const String &resource, const String &id);
    
protected:
    int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];

private:
    bool parseBool(const String &val);
    IPAddress parseIPAddress(const String &val);
    int parseInt(const String &val);
    void SetConfigValue(const String &pair, bool &autoRecovery, bool &limitCycles);
};

extern SettingsView settingsView;

#endif // SettingsView_h