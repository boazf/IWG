#ifndef RecoveryController_h
#define RecoveryController_h

#include <Controller.h>

class RecoveryController : public Controller
{
public:
    RecoveryController() : Controller("RECOVERY")
    {
    }

    bool Get(EthernetClient &client, String &resource);
    bool Post(EthernetClient &client, String &resource, size_t contentLength, String contentType);
    bool Put(EthernetClient &client, String &resource);
    bool Delete(EthernetClient &client, String &resource);
};

extern RecoveryController recoveryController;

#endif // RecoveryController_h