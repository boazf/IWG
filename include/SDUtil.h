#ifndef SDUtil_h
#define SDUtil_h

#include <Arduino.h>
#include <SD.h>

void InitSD();

#ifdef ESP32
#define SdFile File
#else
extern Sd2Card card;
extern SdVolume vol;
#endif

#endif // SDUtil_h