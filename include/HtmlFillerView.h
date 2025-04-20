#ifndef HtmlFillerView_h
#define HtmlFillerView_h

#include <FileView.h>

typedef void (*ViewFiller)(String &fill);

class HtmlFillerView : public FileView
{
public:
    HtmlFillerView(const char *viewFilePath) :
        FileView(viewFilePath)
    {
    }

    bool open(byte *buff, int buffSize);
    int read();

protected:
    virtual int getFillers(const ViewFiller *&fillers) = 0;
    static const String appBase()
    {
        IPAddress myIP = Eth.localIP(); 
        return String("'http://") + myIP[0] + "." + myIP[1]+ "." + myIP[2] + "." + myIP[3] + "/'";
    }

private:
    size_t viewHandler(byte *buff, size_t buffSize);
    bool DoFill(int nFill, String &fill);

private:
    int offset;
};

#endif // HtmlFillerView_h