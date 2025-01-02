#ifndef Controller_h
#define Controller_h

#include <Arduino.h>
#include <EthernetUtil.h>

class ControllerContext
{
public:
    ControllerContext(size_t contentLength, const String &contentType) :
        keepAlive(false),
        contentLength(contentLength),
        contentType(contentType)
    {}
    bool keepAlive;

    size_t getContentLength() { return contentLength; }
    const String &getContentType() { return contentType; }

private:
    size_t contentLength;
    String contentType;
};

class Controller
{
public:
    Controller(const char *_name) :
        name(_name)
    {
    }

    virtual bool Get(EthClient &client, String &resource, ControllerContext &context) = 0;
    virtual bool Post(EthClient &client, String &resource, ControllerContext &context) = 0;
    virtual bool Put(EthClient &client, String &resource, ControllerContext &context) = 0;
    virtual bool Delete(EthClient &client, String &resource, ControllerContext &context) = 0;

    const String name;
};

#endif // Controller_h