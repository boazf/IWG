#ifndef VERSION_H
#define VERSION_H
#include <Arduino.h>

class Version
{
public:
    static String getCurrentVersion();
    static String getOtaVersion();

    typedef enum class _UpdateResult
    {
        error,
        noAvailUpdate,
        done,
        unknown
    } UpdateResult;

    static UpdateResult updateFirmware();

    typedef void (*OnStart)();
    typedef void (*OnEnd)();
    typedef void (*OnProgress)(int, int);
    typedef void (*OnError)(int, const String&);

    static void onStart(OnStart onStartCallback);
    static void onProgress(OnProgress onProgressCallback);
    static void onEnd(OnEnd onEndCallback);
    static void onError(OnError onErrorCallback);
    
private:
    static String getOtaDriveBaseUrl();
    static String getBaseParams();
    static String getChipId();

private:
    static const String unknownVersion;
    static const char *apiKey;
};

#endif // VERSION_H