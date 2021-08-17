#ifndef HistoryView_h
#define HistoryView_h

#include <View.h>

class HistoryView : public View
{
public:
    HistoryView(const char *viewName, const char *viewFile) : 
        View(viewName, viewFile)
    {
    }

    bool open(byte *buff, int buffSize);
};

class HistoryViewCreator : public ViewCreator
{
public:
    HistoryViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }

    View *createView()
    {
        return new HistoryView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern HistoryViewCreator historyViewCreator;

#endif // HistoryView_h