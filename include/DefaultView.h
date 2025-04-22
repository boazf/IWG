#ifndef DefaultView_h
#define DefaultView_h

#include <DummyView.h>
#include <HttpHeaders.h>

class DefaultView : public DummyView
{
public:
    DefaultView(const char *viewFilePath) :
        DummyView(viewFilePath)
    {
    }

    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new DefaultView(""); }

protected:
    bool redirect(EthClient &client, const String &id)
    {
        HttpHeaders::Header additionalHeaders[] = { {"Location", "/index"} };
        HttpHeaders headers(client);
        headers.sendHeaderSection(302, true, additionalHeaders, NELEMS(additionalHeaders));
        return true;
    }

};
#endif // DefaultView_h