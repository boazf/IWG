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

AutoSD::AutoSD()
{
  if (count++ == 0)
    SD.begin();
}

AutoSD::~AutoSD()
{
  if (--count == 0)
    SD.end();
}
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
    Serial.println("Failed to initialize SD card")
#endif
    ;
  SdFile::dateTimeCallback(getFatDateTime);
  if (!vol.init(card))
#ifdef DEBUG_SD
    Serial.println("Failed to initialize SD volume")
#endif
    ;
#endif // ESP32
}

