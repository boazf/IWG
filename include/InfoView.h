#ifndef INFOVIEW_H
#define INFOVIEW_H
#include "HtmlFillerView.h"

class InfoView : public HtmlFillerView
{
public:
    InfoView(const char *_viewFile);
    static std::shared_ptr<HttpController> getInstance() { return std::make_shared<InfoView>("/INFO.HTM"); }

protected:
    static int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];
};

#endif // INFOVIEW_H