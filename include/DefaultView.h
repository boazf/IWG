#ifndef DefaultView_h
#define DefaultView_h

#include <DummyView.h>

class DefaultView : public DummyView
{
public:
    DefaultView(const char *viewPath, const char *viewFilePath) :
        DummyView(viewPath, viewFilePath)
    {
    }

    bool redirect(EthClient &client, const String &id)
    {
        client.println("HTTP/1.1 302 Found");
        client.println("Location: /index");
        client.println("Content-Length: 0");
        client.println("Connection: close");
        client.println();
    #ifdef USE_WIFI
        client.flush();
    #endif
        return true;
    }
};

extern DefaultView defaultView;

#endif // DefaultView_h