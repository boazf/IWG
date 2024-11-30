#ifndef VERSION_CONTROLLER_H
#define VERSION_CONTROLLER_H
#include <Version.h>
#include <Controller.h>
#include <HttpUpdate.h>
#include <map>
#include <Trace.h>

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

    bool sendVersionInfo(EthClient &client)
    {
        
        String version = Version::getOtaVersion();
        String versionJson = "{ \"Version\" : \"" + version + "\" }";

        client.println("HTTP/1.1 200 OK");
        client.print("Content-Length: ");
        client.println(versionJson.length());
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Server: Arduino");
        client.println();
        client.print(versionJson.c_str());
        return true;
    }

private:
    bool updateVersion(EthClient &client)
    {
        BaseType_t ret = xTaskCreate(
            [](void *param)
            {
                static EthClient *notificationClient;
                notificationClient = (EthClient *)param;
                Version::onStart([]()
                {
                    notify(*notificationClient, NotificationType::start);
                });
                Version::onProgress([](int sent, int total)
                {
                    notify(*notificationClient, NotificationType::progress, sent, total);
                });
                Version::onEnd([]()
                {
                    notify(*notificationClient, NotificationType::end);
                    delay(1000);
                });
                Version::onError([](int error, const String &message)
                {
                    notify(*notificationClient, NotificationType::error, error, message);
                });

                EthClient client;
                Version::UpdateResult res = Version::updateFirmware();
                client.stop();
                if (res == Version::UpdateResult::noAvailUpdate)
                    notify(*notificationClient, NotificationType::error, 0, "No available update");
                notificationClient->stop();
                Tracef("Update task stack high watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
                vTaskDelete(NULL);
            },
            "UpdateFirmware",
            4*1024,
            &client,
            tskIDLE_PRIORITY,
            NULL);
        if (ret != pdPASS)
            return false;

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/event-stream");
        client.println("Connection: keep-alive"); 
        client.println("Access-Control-Allow-Origin: *"); 
        client.println("Cache-Control: no-cache");
        client.println();

        return false;
    }

    static String notificationJsonHead(NotificationType notificationType)
    {
        return String("{ \"type\": \"") + notificationTypesStrings.at(notificationType) + "\"";
    }

    static void notify(EthClient &client, NotificationType notificationType)
    {
        notify(client, notificationJsonHead(notificationType) + " }");
    }

    static void notify(EthClient &client, NotificationType notificationType, int sent, int total)
    {
        notify(client, notificationJsonHead(notificationType) + ", \"sent\": " + sent + ", \"total\": " + total + " }");
    }

    static void notify(EthClient &client, NotificationType notificationType, int error, const String &message)
    {
        notify(client, notificationJsonHead(notificationType) + ", \"code\": " + error + ", \"message\": \"" + message + "\" }");
    }

    static void notify(EthClient &client, const String &json)
    {
        client.println(String("data:") + json + "\n");
    }

    static const std::map<VersionController::NotificationType, String> notificationTypesStrings;
};

extern VersionController versionController;

#endif // VERSION_CONTROLLER_H

