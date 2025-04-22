#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H
#include <Common.h>
#include <HttpController.h>
#include <PwrCntl.h>
#include <map>

class SystemController : public HttpController
{
public:
    SystemController()
    {
    }

    bool Get(HttpClientContext &context, const String id)
    {
        if (id.equals("info"))
            return sendVersionInfo(context);
        else if (id.equals("update"))
            return updateVersion(context);
        else
            if (id.equals("reboot"))
                HardReset();
                
        return false;
    }

    bool Post(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool Put(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool Delete(HttpClientContext &context, const String id)
    {
        return false;
    }

    bool isSingleton() { return true; }
    static HttpController *getInstance();

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
    static bool sendVersionInfo(HttpClientContext &context);
    static bool updateVersion(HttpClientContext &context);
    static String notificationJsonHead(NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType);
    static void notify(EthClient &client, NotificationType notificationType, int sent, int total);
    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message);
    static void notify(EthClient &client, const String &json);
    static const std::map<SystemController::NotificationType, String> notificationTypesStrings;
};

extern SystemController systemController;

#endif // SYSTEM_CONTROLLER_H

