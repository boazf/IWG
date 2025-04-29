#ifndef HtmlFillerView_h
#define HtmlFillerView_h

#include <View.h>
#include <HtmlFillerViewReader.h>
#include <FileViewReader.h>
#include <MemViewReader.h>

class 
HtmlFillerView : public View
{
public:
    HtmlFillerView(const char *viewFilePath, GetFillers getFillers) :
        View(new HtmlFillerViewReader(new FileViewReader(viewFilePath), getFillers))
    {
    }

    HtmlFillerView(const byte *mem, size_t size, CONTENT_TYPE contentType, GetFillers getFillers) :
    View(new HtmlFillerViewReader(new MemViewReader(mem, size, contentType), getFillers))
    {
    }

    static const String appBase()
    {
        IPAddress myIP = Eth.localIP(); 
        return String("'http://") + myIP[0] + "." + myIP[1]+ "." + myIP[2] + "." + myIP[3] + "/'";
    }
};

#endif // HtmlFillerView_h