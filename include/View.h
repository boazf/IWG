#ifndef View_h
#define View_h

#include <Arduino.h>
#include <SDUtil.h>
#include <EthernetUtil.h>

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
        if (file)
            file.close();
        buff = NULL;
        buffSize = 0;
    }

    const String viewPath;
    const String viewFilePath;
    virtual bool open(byte *buff, int buffSize);
    virtual void close();
    virtual bool redirect(EthClient &client, const String &id);
    bool getLastModifiedTime(String &lastModifiedtimeStr);
    virtual CONTENT_TYPE getContentType();
    virtual long getViewSize();
    virtual int read();
    virtual bool post(EthClient &client, const String &resource, const String &id);

protected:
    bool openWWWROOT(SdFile &dir);

protected:
    SdFile file;
    int offset;
    byte *buff;
    int buffSize;
};

#endif // View_h