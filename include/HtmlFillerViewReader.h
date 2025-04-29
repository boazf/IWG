#ifndef HtmlFillerViewReader_h
#define HtmlFillerViewReader_h

#include <ViewReader.h>

typedef void (*ViewFiller)(String &fill);
typedef int (*GetFillers)(const ViewFiller *&fillers);

class HtmlFillerViewReader : public ViewReader
{
public:
    HtmlFillerViewReader(ViewReader *viewReader, GetFillers getFillers) :
        viewReader(viewReader),
        getFillers(getFillers)
    {
    }

    virtual ~HtmlFillerViewReader()
    {
        delete viewReader;
    }

    virtual void close() { viewReader->close(); };
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) { return viewReader->getLastModifiedTime(lastModifiedTimeStr); };
    virtual CONTENT_TYPE getContentType() { return viewReader->getContentType(); };
    virtual long getViewSize() { return viewReader->getViewSize(); };
    virtual bool open(byte *buff, int buffSize);
    virtual int read();
    virtual int read(int offset) { return -1; }

protected:
    GetFillers getFillers;
    ViewReader *viewReader;

private:
    size_t viewHandler(byte *buff, size_t buffSize);
    bool DoFill(int nFill, String &fill);

private:
    int offset;
};

#endif // HtmlFillerViewReader_h