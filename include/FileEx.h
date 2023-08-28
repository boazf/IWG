#ifndef FileEx_h
#define FileEx_h

#include <Arduino.h>
#include <SD.h>

class FileEx
{
public:
    FileEx() {}
    FileEx(File _file) { file = _file; }
    FileEx operator =(File _file) { file = _file; return *this; }
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    int read();
    int peek();
    void flush();
    size_t read(uint8_t* buf, size_t size);
    size_t readBytes(char *buffer, size_t length);
    bool seek(uint32_t pos, SeekMode mode);
    bool seek(uint32_t pos);
    size_t position() const;
    size_t size() const;
    void close();
    operator bool() const;
    time_t getLastWrite();
    const char* path() const;
    const char* name() const;

    boolean isDirectory(void);
    FileEx openNextFile(const char* mode = FILE_READ);
    void rewindDirectory(void);

protected:
    File file;
};

#endif // FileEx_h
