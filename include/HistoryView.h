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

extern HistoryView historyView;

#endif // HistoryView_h