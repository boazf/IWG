#ifdef ESP32
#ifndef FilesController_h
#define FilesController_h

#include <Controller.h>

class FilesController : public Controller
{
public:
    FilesController() : Controller("FILES")
    {
    }

    bool Get(EthernetClient &client, String &resource);
    bool Post(EthernetClient &client, String &resource, size_t contentLength, String contentType);
    bool Put(EthernetClient &client, String &resource);
    bool Delete(EthernetClient &client, String &resource);

private:
    void normilizePath(String &path);
    void parseUploadHeaders(const String &header, String &boundary, String &fileName);
};

extern FilesController filesController;

#endif // FilesController_h
#endif // ESP32