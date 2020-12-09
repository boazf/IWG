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

    virtual bool Get(EthernetClient &client, String &resource) = 0;
    virtual bool Post(EthernetClient &client, String &resource) = 0;

    const String name;
};

#endif // Controller_h