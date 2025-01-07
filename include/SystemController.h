#ifndef VERSION_CONTROLLER_H
#define VERSION_CONTROLLER_H
#include <Common.h>
#include <Controller.h>
#include <map>

class VersionController : public Controller
{
public:
    VersionController() : Controller("VERSION")
    {
    }

    bool Get(EthClient &client, String &resource, ControllerContext &context)
    {
        if (resource.equals("info"))
            return sendVersionInfo(client, context);
        else if (resource.equals("update"))
            return updateVersion(client, context);
        else
            return false;
    }

    bool Post(EthClient &client, String &resource, ControllerContext &context)
    {
        return false;
    }

    bool Put(EthClient &client, String &resource, ControllerContext &context)
    {
        return false;
    }

    bool Delete(EthClient &client, String &resource, ControllerContext &context)
    {
        return false;
    }


private:
    #define NOTIFICATION_TYPES \
        X(start) \
        X(progress) \
        X(end) \
        X(error)

    #define X(a) a,
    typedef enum class _NotificationType
    {
        NOTIFICATION_TYPES
    } NotificationType;
    #undef X

private:
    bool sendVersionInfo(EthClient &client, ControllerContext &context);
    bool updateVersion(EthClient &client, ControllerContext &context);
    static String notificationJsonHead(NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType, int sent, int total);
    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message);
    static void notify(EthClient &client, const String &json);
    static const std::map<VersionController::NotificationType, String> notificationTypesStrings;
};

extern VersionController versionController;

#endif // VERSION_CONTROLLER_H

