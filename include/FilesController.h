#ifndef FilesController_h
#define FilesController_h

#include <HttpController.h>

class FilesController : public HttpController
{
public:
    FilesController()
    {
    }

    bool Get(HttpClientContext &context, const String id);
    bool Post(HttpClientContext &context, const String id);
    bool Put(HttpClientContext &context, const String id);
    bool Delete(HttpClientContext &context, const String id);
    bool isSingleton() { return true; }
    static HttpController *getInstance();
    static const String getPath() { return "/API/FILES"; }

private:
    static void normalizePath(String &path);
    static void parseUploadHeaders(const String &header, String &boundary, String &fileName);
};

extern FilesController filesController;

#endif // FilesController_h
