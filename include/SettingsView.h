#ifndef SettingsView_h
#define SettingsView_h

#include <View.h>

class SettingsView : public View
{
public:
    SettingsView(const char *_viewName, const char *_viewFile);     
    bool post(EthernetClient &client, const String &resource, const String &id);
    
private:
    bool DoFill(int nFill, String &fill);
};

extern SettingsView settingsView;

#endif // SettingsView_h