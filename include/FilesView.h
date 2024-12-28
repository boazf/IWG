#ifndef FilesView_h
#define FilesView_h

#include <View.h>

class FilesView : public View
{
public:
    FilesView(const char *_viewName, const char *_viewFile);     
    bool post(EthClient &client, const String &resource, const String &id);
};

class FilesViewCreator : public ViewCreator
{
public:
    FilesViewCreator(const char *_viewPath, const char *_viewFilePath) :
        ViewCreator(_viewPath, _viewFilePath)
    {
    }


    View *createView()
    {
        return new FilesView(viewPath.c_str(), viewFilePath.c_str());
    }
};

extern FilesViewCreator filesViewCreator;

#endif // FilesView_h
