#include <Arduino.h>
#include <Config.h>
#include <SDUtil.h>

// Default MAC address
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
#ifndef USE_WIFI
byte Config::mac[6] =  { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
#endif
byte Config::ip[4] = { 0, 0, 0, 0 };
byte Config::gateway[4] = { 0, 0, 0, 0, };
byte Config::mask[4] = { 0, 0, 0, 0 };
long Config::timeZone = 0;
const char *Config::timeServer = "time.nist.gov";
const char *Config::configFileName = "config.txt";
byte Config::modemRelay = 0;
byte Config::routerRelay = 0;
#ifdef USE_WIFI
const char *Config::ssid; // = "Your SSID";
const char *Config::password; // = "Your password";
#endif

bool Config::ParseString(const String &configValue, void *str)
{
  size_t len = strlen(configValue.c_str());
  *(char **)str = (char *)malloc(len + 1);
  strcpy(*(char **)str, configValue.c_str());
  return true;
}

bool Config::ParseLong(const String &configValue, void *parsedLong)
{
  return sscanf(configValue.c_str(), "%ld", (long *)parsedLong) == 1;
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
  return ParseByteArray(configValue, ',', (byte *)parsedMACAddress, 6);
}

bool Config::ParseIPAddress(const String &configValue, void *parsedIPAddress)
{
  return ParseByteArray(configValue, '.', (byte *)parsedIPAddress, 4);
}

bool Config::ParseByte(const String &configValue, void *parsedByte)
{
  return ParseByte(configValue, (byte *)parsedByte);
}

void Config::Init()
{
#ifdef ESP32
  AutoSD autoSD;
#endif  
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
  #ifndef USE_WIFI
    { String("MACAddress"), ParseMACAddress, mac },
  #endif
    { String("IP"), ParseIPAddress, ip },
    { String("Gateway"), ParseIPAddress, gateway },
    { String("Subnet"), ParseIPAddress, mask },
    { String("ModemRelay"), ParseByte, &modemRelay },
    { String("RouterRelay"), ParseByte, &routerRelay} 
  #ifdef USE_WIFI
    , { String("SSID"), ParseString, &ssid },
    { String("Password"), ParseString, &password }
  #endif
  };

  SdFile config;

#ifdef ESP32
  String configFilePath = String("/") + configFileName;
#ifdef DEBUG_CONFIG
  Trace("Config file: ");
  Traceln(configFilePath);
#endif
  config = SD.open(configFilePath);
#else
  SdFile root;

  root.openRoot(vol);

  config.open(root, configFileName, O_READ);
#endif

#ifdef ESP32
  if (!config)
#else
  if (!config.isOpen())
#endif
  {
#ifdef DEBUG_CONFIG
    Traceln("Failed to open configuration file");
#endif
    return;
  }

  bool eof = false;
  do
  {
    bool eol = false;
    String configLine = "";
    do
    {
      int16_t nextChar = config.read();
      if (nextChar == -1)
      {
        eol = true;
        eof = true;
      }
      else if ((char)nextChar == '\n')
      {
        eol = true;
      }
      else if ((char) nextChar == '\r')
      {
          continue;
      }
      else
      {
        configLine += (char)nextChar;
      }
    } while (!eol);
    if (configLine.equals(""))
    {
      continue;
    }
#ifdef DEBUG_CONFIG
    Trace("Config line:");
    Traceln(configLine.c_str());
#endif
    int sepIndex = configLine.indexOf('=');
    if (sepIndex == -1)
    {
#ifdef DEBUG_CONFIG
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
      Trace("Unrecognized configuration variable: ");
      Traceln(configName);
#endif
      continue;
    }
    if (!parsers[i].parser(configValue, parsers[i].param))
    {
#ifdef DEBUG_CONFIG
      Trace("Failed to parse configuration value for configuration line: ");
      Traceln(configLine.c_str());
#endif
      continue;
    }

  } while (!eof);

#ifndef ESP32
  root.close();
#endif
  config.close();
}

void InitConfig()
{
    Config::Init();
}