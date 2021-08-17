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

class DummyViewCreator : public ViewCreator
{
public:
    DummyViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }

    View *createView()
    {
        return new DummyView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern DummyViewCreator dummyViewCreator;

#endif // DummyView_h