#ifndef HTTPServer_h
#define HTTPServer_h

#include <EthernetUtil.h>
#include <FileView.h>
#include <HttpController.h>
#include <LinkedList.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

class HTTPServer
{
public:
    static void Init();
    static void ServeClient();
    static void AddController(const String path, GetControllerInstance getControllerInstance);

public:
    static void NotModified(EthClient &client);
    static void PageNotFound(EthClient &client);

private:
    static bool GetController(HttpClientContext *context, HttpController *&controller, String &id);
    static void ServiceRequest(HttpClientContext *context);
    static void RequestTask(void *params);
    static void RequestTask(HttpClientContext *context);

private:
    static EthServer server;
    class HttpControllerCreatorData{
    public:    
        HttpControllerCreatorData(const String path, GetControllerInstance instanceGetter) :
            path(path),
            instanceGetter(instanceGetter)
        {}

        HttpControllerCreatorData(const HttpControllerCreatorData &other) : 
            path(other.path),
            instanceGetter(other.instanceGetter)
        {}

        const String getPath() const { return path; }
        const GetControllerInstance getInstanceGetter() const { return instanceGetter; }
    private:
        const String path;
        GetControllerInstance instanceGetter;
    };

    typedef LinkedList<HttpControllerCreatorData> ControllersDataList;
    static ControllersDataList controllersData;
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h