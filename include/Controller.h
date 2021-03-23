#ifndef Controller_h
#define Controller_h

#include <Arduino.h>
#include <EthernetUtil.h>

class Controller
{
public:
    Controller(const char *_name) :
        name(_name)
    {
    }

    virtual bool Get(EthClient &client, String &resource) = 0;
    virtual bool Post(EthClient &client, String &resource, size_t contentLength, String contentType) = 0;
    virtual bool Put(EthClient &client, String &resource) = 0;
    virtual bool Delete(EthClient &client, String &resource) = 0;

    const String name;
};

#endif // Controller_h