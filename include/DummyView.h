#ifndef DummyView_h
#define DummyView_h

#include <FileView.h>

class DummyView : public FileView
{
public:
    DummyView(const char *viewFilePath) :
        FileView(viewFilePath)
    {
    }

    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new DummyView(""); }
    static const String getPath() { return "/DUMMY"; }

protected:
    int read()
    {
        return -1;
    }

    long getViewSize()
    {
        return 0;
    }

    CONTENT_TYPE getContentType()
    {
        return CONTENT_TYPE::HTML;
    }

    bool open(byte *buff, int buffSize)
    {
        return true;
    }

    void close()
    {
    }
};
#endif // DummyView_h