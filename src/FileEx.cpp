#include <Common.h>
#include <FileEx.h>

FileEx &FileEx::operator=(const File& file) 
{
    *((File*)(this)) = file;
    return *this;
}

size_t FileEx::write(uint8_t byte)
{
    Lock lock(csSpi);
    return File::write(byte);
}

size_t FileEx::write(const uint8_t *buf, size_t size)
{
    Lock lock(csSpi);
    return File::write(buf, size);
}

int FileEx::available()
{
    Lock lock(csSpi);
    return File::available();
}

int FileEx::read()
{
    Lock lock(csSpi);
    return File::read();
}

int FileEx::peek()
{
    Lock lock(csSpi);
    return File::peek();
}

void FileEx::flush()
{
    Lock lock(csSpi);
    File::flush();
}

size_t FileEx::read(uint8_t* buf, size_t size)
{
    Lock lock(csSpi);
    return File::read(buf, size);
}

size_t FileEx::readBytes(char *buffer, size_t length)
{
    return read((uint8_t*)buffer, length);
}

bool FileEx::seek(uint32_t pos, SeekMode mode)
{
    Lock lock(csSpi);
    return File::seek(pos, mode);
}

bool FileEx::seek(uint32_t pos)
{
    return seek(pos, SeekSet);
}

size_t FileEx::position() const
{
    Lock lock(csSpi);
    return File::position();
}

size_t FileEx::size() const
{
    Lock lock(csSpi);
    return File::size();
}

void FileEx::close()
{
    Lock lock(csSpi);
    File::close();
}

FileEx::operator bool() const
{
    Lock lock(csSpi);
    return !!File::operator bool();
}

time_t FileEx::getLastWrite()
{
    Lock lock(csSpi);
    return File::getLastWrite();
}

const char* FileEx::path() const
{
    Lock lock(csSpi);
    return File::path();
}

const char* FileEx::name() const
{
    Lock lock(csSpi);
    return File::name();
}

boolean FileEx::isDirectory(void)
{
    Lock lock(csSpi);
    return File::isDirectory();
}

FileEx FileEx::openNextFile(const char* mode)
{
    Lock lock(csSpi);
    return File::openNextFile(mode);
}

void FileEx::rewindDirectory(void)
{
    Lock lock(csSpi);
    File::rewindDirectory();
}
