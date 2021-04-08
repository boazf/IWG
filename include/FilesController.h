#ifndef FilesController_h
#define FilesController_h

#include <Controller.h>

class FilesController : public Controller
{
public:
    FilesController() : Controller("FILES")
    {
    }

    bool Get(EthClient &client, String &resource);
    bool Post(EthClient &client, String &resource, size_t contentLength, String contentType);
    bool Put(EthClient &client, String &resource);
    bool Delete(EthClient &client, String &resource);

private:
    void normilizePath(String &path);
    void parseUploadHeaders(const String &header, String &boundary, String &fileName);
};

extern FilesController filesController;

#endif // FilesController_h
