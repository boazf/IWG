#include <Arduino.h>
#include <Config.h>
#include <SDUtil.h>
#include <PwrCntl.h>
#ifdef DEBUG_CONFIG
#include <Trace.h>
#endif

// Default MAC address
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte Config::mac[6] = 
#ifndef USE_WIFI
  { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
long Config::routerInitTimeSec = 10;
long Config::timeUpdatePeriodMin = 20;
#else
  {0, 0, 0, 0, 0, 0};
#endif
long Config::dnsAvailTimeSec = 5 * 60; // 5 Minutes
byte Config::ip[4] = { 0, 0, 0, 0 };
byte Config::gateway[4] = { 0, 0, 0, 0, };
byte Config::mask[4] = { 0, 0, 0, 0 };
long Config::timeZone = 0;
long Config::DST = 0;
const char *Config::timeServer = "time.nist.gov";
const char *Config::configFileName = "config.txt";
byte Config::modemRelay = 26;
byte Config::routerRelay = 25;
bool Config::singleDevice = false; // Default is two devices: router and modem
const char *Config::deviceName = "Router";
time_t Config::skipRouterTime = 60 * 60; // One hour
long Config::hardResetPeriodDays = 3; // 3 days
long Config::hardResetTime = 3 * 60 * 60; // 3AM
const char *Config::otaServer = "otadrive.com";
#ifdef USE_WIFI
const char *Config::ssid /* = "Your SSID" */;
const char *Config::password /* = "Your password" */;
const char *Config::hostName = "InternetRecoveryBox";
#endif

bool Config::ParseString(const String &configValue, void *strBuffer)
{
  char **str = static_cast<char **>(strBuffer);

  size_t len = strlen(configValue.c_str());
  *str = static_cast<char *>(malloc(len + 1));
  strcpy(*str, configValue.c_str());
  return true;
}

bool Config::ParseLong(const String &configValue, void *parsedLong)
{
  return sscanf(configValue.c_str(), "%ld", parsedLong) == 1;
}
  
bool Config::ParseByte(const String &strByte, byte *value)
{
  const char *format = strByte.startsWith("0x") ? "%hhx" : "%hhd";

  return sscanf(strByte.c_str(), format, value) == 1;
}

bool Config::ParseByteArray(const String &strValue, char separator, byte *bytes, int arrayLen)
{
  int index = 0;
  for(int i = 0; i < arrayLen - 1; i++)
  {
    int separatorIndex = strValue.indexOf(separator, index);
    if (separatorIndex == -1)
      return false;
    String byte = strValue.substring(index, separatorIndex);
    if (!ParseByte(byte, bytes + i))
      return false;
    index = separatorIndex + 1;
  }

  return ParseByte(strValue.substring(index), bytes + arrayLen - 1);
}

bool Config::ParseMACAddress(const String &configValue, void *parsedMACAddress)
{
  return ParseByteArray(configValue, ',', static_cast<byte *>(parsedMACAddress), 6);
}

bool Config::ParseIPAddress(const String &configValue, void *parsedIPAddress)
{
  return ParseByteArray(configValue, '.', static_cast<byte *>(parsedIPAddress), 4);
}

bool Config::ParseByte(const String &configValue, void *parsedByte)
{
  return ParseByte(configValue, static_cast<byte *>(parsedByte));
}

bool Config::ParseBoolean(const String &configValue, void *parsedBoolean)
{
  bool *parsedBooleanBuff = static_cast<bool *>(parsedBoolean);
  
  if (configValue == "true")
  {
    *parsedBooleanBuff = true;
    return true;
  } else if (configValue == "false")
  {
    *parsedBooleanBuff = false;
    return true;
  } else 
  {
    return false;
  }
}

bool Config::ParseTime(const String &configValue, void *parsedTime)
{
  u_int h, m;

  if (sscanf(configValue.c_str(), "%d:%d", &h, &m) != 2)
    return false;

  if (h > 23 || m > 60)
    return false;

  *(static_cast<unsigned long *>(parsedTime)) = (h * 60 + m) * 60;

  return true;
}


void Config::Init()
{
  AutoSD autoSD;
  struct ConfigLineParser
  {
    String configVarName;
    bool (*parser)(const String &configValue, void *parsedValue);
    void *param;
  };

  ConfigLineParser parsers[] =
  {
    { String("TimeServer"), ParseString, &timeServer},
    { String("TimeZone"), ParseLong, &timeZone },
    { String("DST"), ParseLong, &DST },
    { String("DNSAvailTimeSec"), ParseLong, &dnsAvailTimeSec },
    { String("MACAddress"), ParseMACAddress, mac },
  #ifndef USE_WIFI
    { String("RouterInitTimeSec"), ParseLong, &routerInitTimeSec },
    { String("TimeUpdatePeriodMin"), ParseLong, &timeUpdatePeriodMin },
  #endif
    { String("IP"), ParseIPAddress, ip },
    { String("Gateway"), ParseIPAddress, gateway },
    { String("Subnet"), ParseIPAddress, mask },
    { String("ModemRelay"), ParseByte, &modemRelay },
    { String("RouterRelay"), ParseByte, &routerRelay },
    { String("SingleDevice"), ParseBoolean, &singleDevice },
    { String("DeviceName"), ParseString, &deviceName },
    { String("SkipRouterTime"), ParseLong, &skipRouterTime },
    { String("HardResetPeriodDays"), ParseLong, &hardResetPeriodDays },
    { String("HardResetTime"), ParseTime, &hardResetTime },
    { String("OTAServer"), ParseString, &otaServer },
  #ifdef USE_WIFI
    { String("SSID"), ParseString, &ssid },
    { String("Password"), ParseString, &password },
    { String("HostName"), ParseString, &hostName },
  #endif
  };

  String configFilePath = String("/") + configFileName;
#ifdef DEBUG_CONFIG
  {
    LOCK_TRACE();
    Trace("Config file: ");
    Traceln(configFilePath);
  }
#endif

  SdFile config = SD.open(configFilePath);

  if (!config)
  {
#ifdef DEBUG_CONFIG
    Traceln("Failed to open configuration file");
#endif
    HardReset(0);
    return;
  }

  bool eof = false;
  do
  {
    bool eol = false;
    String configLine = "";
    bool skipLine = false;
    do
    {
      int16_t nextChar = config.read();
      switch(nextChar)
      {
      case -1:
        eol = true;
        eof = true;
        break;

      case '\n':
        eol = true;
        break;

      case '\r':
        continue;

      case ';': // Comment
        skipLine = configLine.isEmpty();
      default:
        if (!skipLine)
          configLine += (char)nextChar;
        break;
      }
    } while (!eol);
    if (configLine.equals(""))
    {
      continue;
    }
#ifdef DEBUG_CONFIG
    {
      LOCK_TRACE();
      Trace("Config line:");
      Traceln(configLine.c_str());
    }
#endif
    int sepIndex = configLine.indexOf('=');
    if (sepIndex == -1)
    {
#ifdef DEBUG_CONFIG
      LOCK_TRACE();
      Trace("Invalid configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }
    String configName = configLine.substring(0, sepIndex);
    String configValue = configLine.substring(sepIndex + 1);
    if (configName.equals("") || configValue.equals(""))
    {
#ifdef DEBUG_CONFIG
      LOCK_TRACE();
      Trace("Invalid configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }

    size_t i = 0;
    for (; i < NELEMS(parsers); i++)
      if (parsers[i].configVarName.equals(configName))
        break;
    if (i == NELEMS(parsers))
    {
#ifdef DEBUG_CONFIG
      LOCK_TRACE();
      Trace("Unrecognized configuration variable: ");
      Traceln(configName);
#endif
      continue;
    }
    if (!parsers[i].parser(configValue, parsers[i].param))
    {
#ifdef DEBUG_CONFIG
      LOCK_TRACE();
      Trace("Failed to parse configuration value for configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }

  } while (!eof);

  config.close();
}

void InitConfig()
{
    Config::Init();
}