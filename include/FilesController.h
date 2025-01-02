#ifndef FilesController_h
#define FilesController_h

#include <Controller.h>

class FilesController : public Controller
{
public:
    FilesController() : Controller("FILES")
    {
    }

    bool Get(EthClient &client, String &resource, ControllerContext &context);
    bool Post(EthClient &client, String &resource, ControllerContext &context);
    bool Put(EthClient &client, String &resource, ControllerContext &context);
    bool Delete(EthClient &client, String &resource, ControllerContext &context);

private:
    void normalizePath(String &path);
    void parseUploadHeaders(const String &header, String &boundary, String &fileName);
};

extern FilesController filesController;

#endif // FilesController_h
