#ifndef View_h
#define View_h

#include <Arduino.h>
#include <SDUtil.h>
#include <EthernetUtil.h>

enum class CONTENT_TYPE
{
    UNKNOWN,
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
    bool getLastModifiedTime(String &lastModifiedTimeStr);
    virtual CONTENT_TYPE getContentType();
    virtual long getViewSize();
    virtual int read();
    virtual bool post(EthClient &client, const String &resource, const String &id);

protected:
    bool openWWWROOT(SdFile &dir);
    bool open(byte *buff, int buffSize, SdFile file);

protected:
    SdFile file;
    byte *buff;
    int buffSize;
};

class ViewCreator
{
public:
    ViewCreator(const char *_viewPath, const char *_viewFilePath) :
        viewPath(_viewPath),
        viewFilePath(_viewFilePath)
    {
    }

    virtual View *createView() = 0;

    const String viewPath;
    const String viewFilePath;
};

#endif // View_h