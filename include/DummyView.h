#ifndef DummyView_h
#define DummyView_h

#include <View.h>

class DummyViewReader : public ViewReader
{
public:
    DummyViewReader()
    {
    }

protected:
    int read()
    {
        return -1;
    }

    int read(int offset)
    {
        return -1;
    }

    long getViewSize()
    {
        return 0;
    }

    bool getLastModifiedTime(String &lastModifiedTimeStr)
    {
        return false;
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

class DummyView : public View
{
public:
    DummyView(const char *viewFilePath) :
        View(new DummyViewReader())
    {
    }

    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new DummyView(""); }

protected:
};
#endif // DummyView_h