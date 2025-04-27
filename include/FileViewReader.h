#ifndef FileViewReader_h
#define FileViewReader_h

#include <SDUtil.h>
#include <ViewReader.h>

class FileViewReader : public ViewReader
{
public:
    FileViewReader(const String viewFilePath) :
        viewFilePath(viewFilePath)
    {        
    }
    
    virtual ~FileViewReader()
    {
        close();
    }

protected:
    virtual bool open(byte *buff, int buffSize);
    virtual bool open(byte *buff, int buffSize, SdFile file);
    virtual void close();
    virtual int read();
    int read(int offset);
    virtual long getViewSize();
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr);
    virtual CONTENT_TYPE getContentType();

private:
    String viewFilePath;
    SdFile file;
};

#endif // FileViewReader