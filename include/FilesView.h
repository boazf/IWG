#ifndef FilesView_h
#define FilesView_h

#include <FileView.h>

class FilesView : public FileView
{
public:
    FilesView(const char *_viewFile);
    virtual bool Post(HttpClientContext &context, const String id);
    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new FilesView("/FILES.HTM"); }
    static const String getPath() { return "/FILES"; }
};

#endif // FilesView_h
