#ifndef HttpFillerView_h
#define HttpFillerView_h

#include <View.h>

typedef void (*ViewFiller)(String &fill);

class HttpFillerView : public View
{
public:
    HttpFillerView(const char *viewPath, const char *viewFilePath) :
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

#endif // HttpFillerView_h