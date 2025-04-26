#ifndef View_h
#define View_h

#include <HttpController.h>
#include <ViewReader.h>

class View : public HttpController
{
public:
    View(ViewReader *viewReader) : 
        viewReader(viewReader)
    {
    }

    virtual ~View()
    {
        delete viewReader;
    }

    virtual bool Get(HttpClientContext &context, const String id);
    virtual bool Post(HttpClientContext &context, const String id);
    virtual bool Put(HttpClientContext &context, const String id);
    virtual bool Delete(HttpClientContext &context, const String id);

protected:
    virtual bool redirect(EthClient &client, const String &id) { return false; }

protected:
    ViewReader *viewReader;
};

#endif // View_h