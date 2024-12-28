#ifndef VERSION_CONTROLLER_H
#define VERSION_CONTROLLER_H
#include <Controller.h>
#include <map>

class VersionController : public Controller
{
public:
    VersionController() : Controller("VERSION")
    {
    }

    bool Get(EthClient &client, String &resource)
    {
        if (resource.equals("info"))
            return sendVersionInfo(client);
        else if (resource.equals("update"))
            return updateVersion(client);
        else
            return false;
    }

    bool Post(EthClient &client, String &resource, size_t contentLength, String contentType)
    {
        return false;
    }

    bool Put(EthClient &client, String &resource)
    {
        return false;
    }

    bool Delete(EthClient &client, String &resource)
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
    bool sendVersionInfo(EthClient &client);
    bool updateVersion(EthClient &client);
    static String notificationJsonHead(NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType, int sent, int total);
    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message);
    static void notify(EthClient &client, const String &json);
    static const std::map<VersionController::NotificationType, String> notificationTypesStrings;
};

extern VersionController versionController;

#endif // VERSION_CONTROLLER_H

