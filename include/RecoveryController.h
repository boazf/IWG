#ifndef RecoveryController_h
#define RecoveryController_h

#include <Controller.h>

class RecoveryController : public Controller
{
public:
    RecoveryController() : Controller("RECOVERY")
    {
    }

    bool Get(EthClient &client, String &resource, ControllerContext &context);
    bool Post(EthClient &client, String &resource, ControllerContext &context);
    bool Put(EthClient &client, String &resource, ControllerContext &context);
    bool Delete(EthClient &client, String &resource, ControllerContext &context);
};

extern RecoveryController recoveryController;

#endif // RecoveryController_h