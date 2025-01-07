#include <Version.h>
#include <Config.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <HttpUpdate.h>

const String Version::unknownVersion = "Unknown";

#ifdef USE_WIFI
#define API_KEY "85563019-0120-4fc4-bd4b-5baff9c556a9"
#else
#define API_KEY "d54ef031-6f61-4f00-bcdc-c803c3fa8ce8"
#endif

#define APP_VERSION "1.0.16"

const char *Version::apiKey = API_KEY;

String Version::getCurrentVersion()
{
    return APP_VERSION;
}

String Version::getOtaVersion()
{
    AutoStopClient client;
    HttpClientEx http(client);
    String url = getOtaDriveBaseUrl() + "update?" + getBaseParams();
    http.begin(url);
    http.get();
    int code = http.responseStatusCode();
    if (code == 304)
        return getCurrentVersion();

    HttpClientEx::Headers headers[] = {{"X-Version"}};
    http.collectHeaders(headers, NELEMS(headers));

    if (headers[0].value.isEmpty())
        return unknownVersion;

    return headers[0].value;
}

Version::UpdateResult Version::updateFirmware()
{
    AutoStopClient client;
    HttpUpdateResult ret = httpUpdate.update(client, getOtaDriveBaseUrl() + "update?" + getBaseParams(), getCurrentVersion());
    switch(ret)
    {
        case HttpUpdateResult::HTTP_UPDATE_FAILED:
            return UpdateResult::error;
        case HttpUpdateResult::HTTP_UPDATE_NO_UPDATES:
            return UpdateResult::noAvailUpdate;
        case HttpUpdateResult::HTTP_UPDATE_OK:
            return UpdateResult::done;
    }

    return UpdateResult::unknown;
}


void Version::onStart(OnStart onStartCallback)
{
    httpUpdate.onStart(onStartCallback);
}

void Version::onProgress(OnProgress onProgressCallback)
{
    httpUpdate.onProgress(onProgressCallback);
}

void Version::onEnd(OnEnd onEndCallback)
{
    httpUpdate.onEnd(onEndCallback);
}

void Version::onError(OnError onErrorCallback)
{
    static OnError otaOnError;
    otaOnError = onErrorCallback;
    httpUpdate.onError([](int errCode)
    {
        String errMessage = httpUpdate.getLastErrorString();
        otaOnError(errCode, errMessage);
    });
}

String Version::getOtaDriveBaseUrl()
{
    return String("http://") + Config::otaServer + "/deviceapi/";
}

String Version::getChipId()
{
    String chipIdHex = String((uint32_t)(ESP.getEfuseMac() >> 32), HEX);
    chipIdHex += String((uint32_t)ESP.getEfuseMac(), HEX);
    return chipIdHex;
}

String Version::getBaseParams()
{
    return String("k=") + apiKey + "&v=" + getCurrentVersion() + "&s=" + getChipId();
}
