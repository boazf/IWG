#ifndef IndexView_h
#define IndexView_h

#include <HtmlFillerView.h>

class IndexView : public HtmlFillerView
{
public:
    IndexView(const char *_viewName, const char *_viewFile);     
    bool redirect(EthClient &client, const String &_id);

protected:
    int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];

private:
    static int id;
};

extern IndexView indexView;

#endif // IndexView_h