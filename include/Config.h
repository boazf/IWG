#ifndef Config_h
#define Config_h

#include <Arduino.h>
#include <Common.h>
#include <SD.h>

class Config
{
public:
#ifndef ESP32
    static byte mac[6];
#endif
    static byte ip[4];
    static byte gateway[4];
    static byte mask[4];
    static long timeZone;
    static const char *timeServer;
    static byte modemRelay;
    static byte routerRelay;
#ifdef ESP32
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
};

void InitConfig();

#endif // Config_h