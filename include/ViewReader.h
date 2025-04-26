#ifndef ViewReader_h
#define ViewReader_h

#include <Arduino.h>
#include <EthernetUtil.h>
#include <HttpHeaders.h>

class ViewReader {
public:
    virtual ~ViewReader() {}
    virtual bool open(byte *buff, int buffSize) { this->buff = buff; this->buffSize = buffSize; return true; };
    virtual void close() {};
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) = 0;
    virtual CONTENT_TYPE getContentType() = 0;
    virtual long getViewSize() = 0;
    virtual int read() = 0;
    virtual int read(int offset) = 0;

protected:
    byte *buff;
    int buffSize;
};

#endif // ViewReader_h