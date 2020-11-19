#ifndef DummyView_h
#define DummyView_h

#include <View.h>

class DummyView : public View
{
public:
    DummyView(const char *viewPath, const char *viewFilePath) :
        View(viewPath, viewFilePath)
    {
    }

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

extern DummyView dummyView;

#endif // DummyView_h