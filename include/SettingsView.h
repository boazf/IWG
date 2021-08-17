#ifndef SettingsView_h
#define SettingsView_h

#include <HtmlFillerView.h>

class SettingsView : public HtmlFillerView
{
public:
    SettingsView(const char *_viewName, const char *_viewFile);     
    bool post(EthClient &client, const String &resource, const String &id);
    
protected:
    int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];

private:
    bool parseBool(const String &val);
    IPAddress parseIPAddress(const String &val);
    int parseInt(const String &val);
    void SetConfigValue(const String &pair, bool &autoRecovery, bool &limitCycles, bool &DST);
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