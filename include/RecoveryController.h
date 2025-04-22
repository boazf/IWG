#ifndef RecoveryController_h
#define RecoveryController_h

#include <HttpController.h>

class RecoveryController : public HttpController
{
public:
    RecoveryController()
    {
    }

    bool Get(HttpClientContext &context, const String id);
    bool Post(HttpClientContext &context, const String id);
    bool Put(HttpClientContext &context, const String id);
    bool Delete(HttpClientContext &context, const String id);
    bool isSingleton() { return true; }
    static HttpController *getInstance();
};

extern RecoveryController recoveryController;

#endif // RecoveryController_h