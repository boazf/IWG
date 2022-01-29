#ifndef SDUtil_h
#define SDUtil_h

#include <Arduino.h>
#include <SD.h>

#include <FileEx.h>
#include <Lock.h>

class AutoSD
{
public:
  AutoSD();
  ~AutoSD();
  static void WaitForIdle();

private:
  static int count;
  static CriticalSection cs;
};

#define SD SDEx

class SDExClass
{
public:
    bool begin(uint8_t ssPin=SS, SPIClass &spi=SPI, uint32_t frequency=4000000, const char * mountpoint="/sd", uint8_t max_files=5);
    void end();
    sdcard_type_t cardType();
    uint64_t cardSize();
    uint64_t totalBytes();
    uint64_t usedBytes();
    File open(const char* path, const char* mode = FILE_READ);
    File open(const String& path, const char* mode = FILE_READ);
    bool exists(const char* path);
    bool exists(const String& path);
    bool remove(const char* path);
    bool remove(const String& path);
    bool rename(const char* pathFrom, const char* pathTo);
    bool rename(const String& pathFrom, const String& pathTo);
    bool mkdir(const char *path);
    bool mkdir(const String &path);
    bool rmdir(const char *path);
    bool rmdir(const String &path);
};

extern SDExClass SDEx;

void InitSD();

#define SdFile FileEx

#endif // SDUtil_h