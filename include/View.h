#ifndef View_h
#define View_h

#include <Arduino.h>
#include <SD.h>
#include <Ethernet.h>

enum CONTENT_TYPE
{
    CT_UNKNOWN,
    JAVASCRIPT,
    ICON,
    HTML,
    CSS,
    JPEG,
    EOT,
    SVG,
    TTF,
    WOFF,
    WOFF2
};

typedef void (*ViewFiller)(String &fill);

struct View
{
public:
    View(const String _viewPath, const String _viewFilePath) :
        viewPath(_viewPath),
        viewFilePath(_viewFilePath),
        offset(0),
        buff(NULL),
        buffSize(0)
    {
    }

    virtual ~View()
    {
        if (file.isOpen())
            file.close();
        buff = NULL;
        buffSize = 0;
    }

    const String viewPath;
    const String viewFilePath;
    int viewHandler(byte *buff, int buffSize);
    virtual bool open(byte *buff, int buffSize);
    virtual void close();
    virtual bool redirect(EthernetClient &client, const String &id);
    bool getLastModifiedTime(String &lastModifiedtimeStr);
    virtual CONTENT_TYPE getContentType();
    virtual long getViewSize();
    virtual int read();
    virtual bool post(EthernetClient &client, const String &resource, const String &id);

protected:
    bool openWWWROOT(SdFile &dir);

protected:
    SdFile file;
    int offset;
    byte *buff;
    int buffSize;

private:
    virtual bool DoFill(int nFill, String &fill);
};

#endif // View_h