#ifndef HistoryView_h
#define HistoryView_h

#include <FileView.h>
#include <Lock.h>

class HistoryViewReader : public FileViewReader
{
public:
    HistoryViewReader(const char *viewFile) :
        FileViewReader(viewFile)
    {}
    
    virtual bool open(byte *buff, int buffSize);

private:
    static CriticalSection cs;
};

class HistoryView : public View
{
public:
    HistoryView(const char *viewFile) : 
        View(new HistoryViewReader(viewFile))
    {
    }

    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new HistoryView("/HISTORY.HTM"); }
};
#endif // HistoryView_h