#ifndef DefaultView_h
#define DefaultView_h

#include <DummyView.h>
#include <HttpHeaders.h>

class DefaultView : public DummyView
{
public:
    DefaultView(const char *viewPath, const char *viewFilePath) :
        DummyView(viewPath, viewFilePath)
    {
    }

    bool redirect(EthClient &client, const String &id)
    {
        HttpHeaders::Header additionalHeaders[] = { {"Location", "/index"} };
        HttpHeaders headers(client);
        headers.sendHeaderSection(302, true, additionalHeaders, NELEMS(additionalHeaders));
        return true;
    }
};

class DefaultViewCreator : public ViewCreator
{
public:
    DefaultViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }

    View *createView()
    {
        return new DefaultView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern DefaultViewCreator defaultViewCreator;

#endif // DefaultView_h