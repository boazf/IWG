#include <Arduino.h>
#include <SD.h>
#include <SDUtil.h>
#include <assert.h>
#include <TimeUtil.h>
#include <Common.h>

#ifndef ESP32
Sd2Card card;
SdVolume vol;
#endif

#ifdef ESP32
int AutoSD::count = 0;;
CriticalSection AutoSD::cs;

AutoSD::AutoSD()
{
  Lock lock(cs);
  if (count++ == 0)
    SD.begin();
}

AutoSD::~AutoSD()
{
  Lock lock(cs);
  if (--count == 0)
    SD.end();
}

#undef SD

bool SDExClass::begin(uint8_t ssPin, SPIClass &spi, uint32_t frequency, const char * mountpoint, uint8_t max_files)
{
  Lock lock(csSpi);
  return SD.begin(ssPin, spi, frequency, mountpoint, max_files);
}

void SDExClass::end()
{
  Lock lock(csSpi);
  SD.end(); 
}

sdcard_type_t SDExClass::cardType()
{
  Lock lock(csSpi);
  return SD.cardType();
}

uint64_t SDExClass::cardSize()
{
  Lock lock(csSpi);
  return SD.cardSize();
}

uint64_t SDExClass::totalBytes()
{
  Lock lock(csSpi);
  return SD.totalBytes();
}

uint64_t SDExClass::usedBytes()
{
  Lock lock(csSpi);
  return SD.usedBytes(); 
}

File SDExClass::open(const char* path, const char* mode)
{
  Lock lock(csSpi);
  return SD.open(path, mode);
}

File SDExClass::open(const String& path, const char* mode)
{
  return open(path.c_str(), mode);  
}

bool SDExClass::exists(const char* path)
{
  Lock lock(csSpi);
  return SD.exists(path);
}

bool SDExClass::exists(const String& path)
{
  return exists(path.c_str());
}

bool SDExClass::remove(const char* path)
{
  Lock lock(csSpi);
  return SD.remove(path);
}
bool SDExClass::remove(const String& path)
{
  return remove(path.c_str());
}

bool SDExClass::rename(const char* pathFrom, const char* pathTo)
{
  Lock lock(csSpi);
  return SD.rename(pathFrom, pathTo);
}

bool SDExClass::rename(const String& pathFrom, const String& pathTo)
{
  return rename(pathFrom.c_str(), pathTo.c_str());
}
bool SDExClass::mkdir(const char *path)
{
  Lock lock(csSpi);
  return SD.mkdir(path);
}

bool SDExClass::mkdir(const String &path)
{
  return mkdir(path.c_str());
}

bool SDExClass::rmdir(const char *path)
{
  Lock lock(csSpi);
  return SD.rmdir(path);
}

bool SDExClass::rmdir(const String &path)
{
  return rmdir(path.c_str());
}

SDExClass SDEx;
#endif

void InitSD()
{
#ifndef ESP32
  // change this to match your SD shield or module;
  // Arduino Ethernet shield: pin 4
  // Adafruit SD shields and modules: pin 10
  // Sparkfun SD shield: pin 8
  // MKRZero SD: SDCARD_SS_PIN
  const int chipSelect = 4;

  if (!card.init(SPI_HALF_SPEED, chipSelect))
#ifdef DEBUG_SD
    Traceln("Failed to initialize SD card")
#endif
    ;
  SdFile::dateTimeCallback(getFatDateTime);
  if (!vol.init(card))
#ifdef DEBUG_SD
    Traceln("Failed to initialize SD volume")
#endif
    ;
#endif // ESP32
}

