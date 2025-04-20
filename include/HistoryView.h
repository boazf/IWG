#ifndef HistoryView_h
#define HistoryView_h

#include <FileView.h>
#include <Lock.h>

class HistoryView : public FileView
{
public:
    HistoryView(const char *viewFile) : 
        FileView(viewFile)
    {
    }

    virtual bool open(byte *buff, int buffSize);
    virtual void close();
    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new HistoryView("/HISTORY.HTM"); }
    static const String getPath() { return "/HISTORY"; }

private:
    static CriticalSection cs;
};
#endif // HistoryView_h