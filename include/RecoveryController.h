#ifndef RecoveryController_h
#define RecoveryController_h

#include <Controller.h>

class RecoveryController : public Controller
{
public:
    RecoveryController() : Controller("RECOVERY")
    {
    }

    bool Get(EthClient &client, String &resource);
    bool Post(EthClient &client, String &resource, size_t contentLength, String contentType);
    bool Put(EthClient &client, String &resource);
    bool Delete(EthClient &client, String &resource);
};

extern RecoveryController recoveryController;

#endif // RecoveryController_h