#ifndef SDUtil_h
#define SDUtil_h

#include <Arduino.h>
#include <SD.h>

#ifdef ESP32

#include <Lock.h>

class AutoSD
{
public:
  AutoSD();
  ~AutoSD();

private:
  static int count;
  static CriticalSection cs;
};
#endif

void InitSD();

#ifdef ESP32
#define SdFile File
#else
extern Sd2Card card;
extern SdVolume vol;
#endif

#endif // SDUtil_h