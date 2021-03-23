#ifdef ESP32
#ifndef FilesView_h
#define FilesView_h

#include <EthernetUtil.h>
#include <View.h>

class FilesView : public View
{
public:
    FilesView(const char *_viewName, const char *_viewFile);     
    bool post(EthClient &client, const String &resource, const String &id);
};

extern FilesView filesView;

#endif // FilesView_h
#endif // ESP32