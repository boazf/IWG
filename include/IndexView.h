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

class IndexViewCreator : public ViewCreator
{
public:
    IndexViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }

    View *createView()
    {
        return new IndexView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern IndexViewCreator indexViewCreator;

#endif // IndexView_h