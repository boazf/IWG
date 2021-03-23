#ifdef ESP32
#include <Common.h>
#include <FileEx.h>

size_t FileEx::write(uint8_t byte)
{
    Lock lock(csSpi);
    return file.write(byte);
}

size_t FileEx::write(const uint8_t *buf, size_t size)
{
    Lock lock(csSpi);
    return file.write(buf, size);
}

int FileEx::available()
{
    Lock lock(csSpi);
    return file.available();
}

int FileEx::read()
{
    Lock lock(csSpi);
    return file.read();
}

int FileEx::peek()
{
    Lock lock(csSpi);
    return file.peek();
}

void FileEx::flush()
{
    Lock lock(csSpi);
    file.flush();
}

size_t FileEx::read(uint8_t* buf, size_t size)
{
    Lock lock(csSpi);
    return file.read(buf, size);
}

size_t FileEx::readBytes(char *buffer, size_t length)
{
    return read((uint8_t*)buffer, length);
}

bool FileEx::seek(uint32_t pos, SeekMode mode)
{
    Lock lock(csSpi);
    return file.seek(pos, mode);
}

bool FileEx::seek(uint32_t pos)
{
    return seek(pos, SeekSet);
}

size_t FileEx::position() const
{
    Lock lock(csSpi);
    return file.position();
}

size_t FileEx::size() const
{
    Lock lock(csSpi);
    return file.size();
}

void FileEx::close()
{
    Lock lock(csSpi);
    file.close();
}

FileEx::operator bool() const
{
    Lock lock(csSpi);
    return !!file;
}

time_t FileEx::getLastWrite()
{
    Lock lock(csSpi);
    return file.getLastWrite();
}

const char* FileEx::name() const
{
    Lock lock(csSpi);
    return file.name();
}

boolean FileEx::isDirectory(void)
{
    Lock lock(csSpi);
    return file.isDirectory();
}

FileEx FileEx::openNextFile(const char* mode)
{
    Lock lock(csSpi);
    return file.openNextFile(mode);
}

void FileEx::rewindDirectory(void)
{
    Lock lock(csSpi);
    file.rewindDirectory();
}
#endif // ESP32