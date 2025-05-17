#ifndef FileEx_h
#define FileEx_h

#include <Arduino.h>
#include <SD.h>

class FileEx : public File
{
public:
    FileEx() {}
    FileEx(File file) : File(file) {}
    FileEx& operator=(const File& file);
    size_t write(uint8_t) override;
    size_t write(const uint8_t *buf, size_t size) override;
    int available() override;
    int read() override;
    int peek() override;
    void flush() override;
    size_t read(uint8_t* buf, size_t size);
    size_t readBytes(char *buffer, size_t length) override;
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
};

#endif // FileEx_h
