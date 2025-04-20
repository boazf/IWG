#ifndef View_h
#define View_h

#include <HttpController.h>

class View : public HttpController
{
public:
    View() : 
        buff(NULL),
        buffSize(0)
    {}

    virtual bool Get(HttpClientContext &context, const String id);
    virtual bool Post(HttpClientContext &context, const String id);
    virtual bool Put(HttpClientContext &context, const String id);
    virtual bool Delete(HttpClientContext &context, const String id);

protected:
    virtual bool open(byte *buff, int buffSize);
    virtual void close();
    virtual bool redirect(EthClient &client, const String &id);
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) = 0;
    virtual CONTENT_TYPE getContentType() = 0;
    virtual long getViewSize() = 0;
    virtual int read() = 0;

protected:
    byte *buff;
    int buffSize;
};

#endif // View_h