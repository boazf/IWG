#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <Common.h>

class Config
{
public:
    static byte mac[6];
#ifndef USE_WIFI
    static long routerInitTimeSec;
    static long timeUpdatePeriodMin;
#endif
    static long dnsAvailTimeSec;
    static byte ip[4];
    static byte gateway[4];
    static byte mask[4];
    static long timeZone;
    static long DST;
    static const char *timeServer;
    static byte modemRelay;
    static byte routerRelay;
    static bool singleDevice;
    static const char *deviceName;
    static time_t skipRouterTime;
    static long hardResetPeriodDays;
    static long hardResetTime;
    static const char *otaServer;
#ifdef USE_WIFI
    static const char *ssid;
    static const char *password;
#endif


public:
    static void Init();

private:
    static const char *configFileName;

private:
    static bool ParseString(const String &configValue, void *str);
    static bool ParseLong(const String &configValue, void *parsedTimeZone);
    static bool ParseByte(const String &strByte, byte *value);
    static bool ParseByte(const String &strByte, void *value);
    static bool ParseByteArray(const String &strValue, char separator, byte *bytes, int arrayLen);
    static bool ParseMACAddress(const String &configValue, void *parsedMACAddress);
    static bool ParseIPAddress(const String &configValue, void *parsedIPAddress);
    static bool ParseBoolean(const String &configValue, void *parsedBoolean);
    static bool ParseTime(const String &configValue, void *parsedTime);
};

void InitConfig();

#endif // Config_h