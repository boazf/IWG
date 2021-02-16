#ifndef HtmlFillerView_h
#define HtmlFillerView_h

#include <View.h>

typedef void (*ViewFiller)(String &fill);

class HtmlFillerView : public View
{
public:
    HtmlFillerView(const char *viewPath, const char *viewFilePath) :
        View(viewPath, viewFilePath)
    {
    }

    int read();

protected:
    virtual int getFillers(const ViewFiller *&fillers) = 0;

private:
    int viewHandler(byte *buff, int buffSize);
    bool DoFill(int nFill, String &fill);
};

#endif // HtmlFillerView_h