#include <VersionController.h>
#include <Version.h>
#include <HttpUpdate.h>
#include <Trace.h>

bool VersionController::sendVersionInfo(EthClient &client)
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

bool VersionController::updateVersion(EthClient &client)
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

String VersionController::notificationJsonHead(NotificationType notificationType)
{
    return String("{ \"type\": \"") + notificationTypesStrings.at(notificationType) + "\"";
}

void VersionController::notify(EthClient &client, NotificationType notificationType)
{
    notify(client, notificationJsonHead(notificationType) + " }");
}

void VersionController::notify(EthClient &client, NotificationType notificationType, int sent, int total)
{
    notify(client, notificationJsonHead(notificationType) + ", \"sent\": " + sent + ", \"total\": " + total + " }");
}

void VersionController::notify(EthClient &client, NotificationType notificationType, int error, const String &message)
{
    notify(client, notificationJsonHead(notificationType) + ", \"code\": " + error + ", \"message\": \"" + message + "\" }");
}

void VersionController::notify(EthClient &client, const String &json)
{
    client.println(String("data:") + json + "\n");
}


VersionController versionController;

#define X(a) {VersionController::NotificationType::a, #a},
const std::map<VersionController::NotificationType, String> VersionController::notificationTypesStrings =
{
    NOTIFICATION_TYPES
};
#undef X
