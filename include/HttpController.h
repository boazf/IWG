#ifndef HttpController_h
#define HttpController_h

#include <Arduino.h>
#include <HttpClientContext.h>

class HttpController
{
public:
    virtual ~HttpController() {}
    virtual bool Get(HttpClientContext &client, const String id = "") = 0;
    virtual bool Post(HttpClientContext &client, const String id = "") = 0;
    virtual bool Put(HttpClientContext &client, const String id = "") = 0;
    virtual bool Delete(HttpClientContext &client, const String id = "") = 0;
    virtual bool isSingleton() = 0;
};

typedef HttpController *(*GetControllerInstance)();

#endif // HttpController_h