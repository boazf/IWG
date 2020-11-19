#ifndef SDUtil_h
#define SDUtil_h

#include <Arduino.h>
#include <SD.h>

void InitSD();

extern Sd2Card card;
extern SdVolume vol;

#endif // SDUtil_h