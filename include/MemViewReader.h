#ifndef MemViewReader_h
#define MemViewReader_h

#include <ViewReader.h>

class MemViewReader : public ViewReader
{
public:
    MemViewReader(const byte *mem, size_t size, CONTENT_TYPE contentType) : 
        mem(mem), 
        size(size), 
        contentType(contentType), 
        offset(0) 
    {}

    virtual bool open(byte *buff, int buffSize) { this->buff = buff; this->buffSize = buffSize; return true; };
    virtual void close() {};
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) {return false; };
    virtual CONTENT_TYPE getContentType() { return contentType; };
    virtual long getViewSize() { return size; }
    virtual int read() { return read(buff, buffSize); }
    virtual int read(int offset) { return read(buff + offset, buffSize - offset); }

    
protected:
    const byte *mem;
    const size_t size;
    CONTENT_TYPE contentType;
    size_t offset;

private:
    int read(byte *readBuff, size_t readSize)
    {
        if (offset >= size)
            return -1;

        readSize = min<size_t>(readSize, size - offset);
        memcpy(readBuff, mem + offset, readSize);
        offset += readSize;

        return readSize;
    }
};
#endif // MemViewReader_h
