#ifndef FileView_h
#define FileView_h

#include <Arduino.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <HttpHeaders.h>
#include <View.h>

struct FileView : public View
{
public:
    FileView(const String viewFilePath) :
        viewFilePath(viewFilePath)
    {
    }

    virtual ~FileView()
    {
        if (file)
            file.close();
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
#endif // FileView_h