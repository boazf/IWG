#include <SystemController.h>
#include <Version.h>
#include <HttpUpdate.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif
#include <HttpHeaders.h>

bool SystemController::sendVersionInfo(HttpClientContext &context)
{
    
    String version = Version::getOtaVersion();
    String versionJson = "{ \"Version\" : \"" + version + "\" }";

    HttpHeaders::Header additionalHeaders[] = {{CONTENT_TYPE::JSON}, {"Access-Control-Allow-Origin", "*"}};
    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), versionJson.length());

    context.getClient().print(versionJson.c_str());

    return true;
}

bool SystemController::updateVersion(HttpClientContext &context)
{
    BaseType_t ret = xTaskCreate(
        [](void *param)
        {
            static EthClient notificationClient;
            notificationClient = *(EthClient *)param;
            Version::onStart([]()
            {
                notify(notificationClient, NotificationType::start);
            });
            Version::onProgress([](int sent, int total)
            {
                notify(notificationClient, NotificationType::progress, sent, total);
            });
            Version::onEnd([]()
            {
                notify(notificationClient, NotificationType::end);
                delay(1000);
            });
            Version::onError([](int error, const String &message)
            {
                notify(notificationClient, NotificationType::error, error, message);
            });

            Version::UpdateResult res = Version::updateFirmware();
            if (res == Version::UpdateResult::noAvailUpdate)
                notify(notificationClient, NotificationType::error, 0, "No available update");
#ifdef DEBUG_HTTP_SERVER
            Tracef("%d Stopping client\n", notificationClient.remotePort());
#endif
            notificationClient.stop();
#ifdef DEBUG_HTTP_SERVER
            Tracef("Update task stack high watermark: %d\n", uxTaskGetStackHighWaterMark(NULL));
#endif
            vTaskDelete(NULL);
        },
        "UpdateFirmware",
        4*1024,
        &context.getClient(),
        tskIDLE_PRIORITY,
        NULL);
    if (ret != pdPASS)
        return false;

    HttpHeaders headers(context.getClient());
    headers.sendStreamHeaderSection();
    context.keepAlive = true;

    return true;
}

String SystemController::notificationJsonHead(NotificationType notificationType)
{
    return String("{ \"type\": \"") + notificationTypesStrings.at(notificationType) + "\"";
}

void SystemController::notify(EthClient &client, NotificationType notificationType)
{
    notify(client, notificationJsonHead(notificationType) + " }");
}

void SystemController::notify(EthClient &client, NotificationType notificationType, int sent, int total)
{
    notify(client, notificationJsonHead(notificationType) + ", \"sent\": " + sent + ", \"total\": " + total + " }");
}

void SystemController::notify(EthClient &client, NotificationType notificationType, int error, const String &message)
{
    notify(client, notificationJsonHead(notificationType) + ", \"code\": " + error + ", \"message\": \"" + message + "\" }");
}

void SystemController::notify(EthClient &client, const String &json)
{
    client.println(String("data:") + json + "\n");
}

HttpController *SystemController::getInstance() { return &systemController; }

SystemController systemController;

#define X(a) {SystemController::NotificationType::a, #a},
const std::map<SystemController::NotificationType, String> SystemController::notificationTypesStrings =
{
    NOTIFICATION_TYPES
};
#undef X
