#ifndef HTTPServer_h
#define HTTPServer_h

#include <EthernetUtil.h>
#include <View.h>
#include <Controller.h>
#include <LinkedList.h>
#include <HttpHeaders.h>
#include <map>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

class ClientContext
{
public:
    ClientContext(EthClient &client);

    ~ClientContext()
    {
        delete[] collectedHeaders;
    }

    bool parseRequestHeaderSection();

    HTTP_REQ_TYPE getRequestType() { return requestType; }
    EthClient &getClient() { return client; }
    uint16_t getRemotePort() { return remotePort; }
    String getLastModified() { return collectedHeaders[0].value; }
    size_t getContentLength() { return atoi(collectedHeaders[1].value.c_str()); }
    String getContentType() { return collectedHeaders[2].value; }
    String getResource() { return resource; }

private:
    static String collectedHeadersNames[];
    HttpHeaders::Header *collectedHeaders;
    HTTP_REQ_TYPE requestType;
    EthClient client;
    uint16_t remotePort;
    String resource;
};

class HTTPServer
{
public:
    static void Init();
    static void ServeClient();
    static void AddView(ViewCreator *viewCreator);
    static void AddController(Controller *controller);

private:
    static bool DoController(ClientContext *context, String &resource, HTTP_REQ_TYPE requestType);
    static bool GetView(const String resource, View *&view, String &id);
    static bool HandlePostRequest(ClientContext *context, const String &resource);
    static bool HandleGetRequest(ClientContext *context, String &resource);
    static void NotModified(EthClient &client);
    static void PageNotFound(EthClient &client);
    static void ServiceRequest(ClientContext *context);
    static void RequestTask(void *params);
    static void RequestTask(ClientContext *context);

private:
    static EthServer server;
    static LinkedList<ViewCreator *> viewCreators;
    static LinkedList<Controller *> controllers;
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h