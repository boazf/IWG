#ifndef HTTPServer_h
#define HTTPServer_h

#include <EthernetUtil.h>
#include <View.h>
#include <Controller.h>
#include <LinkedList.h>
#include <HttpHeaders.h>
#include <array>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

#define N_COLLECTED_HEADERS 3
#define IF_MODIFIED_SINCE_HEADER_NAME "If-Modified-Since"
#define CONTENT_LENGTH_HEADER_NAME "Content-Length"
#define CONTENT_TYPE_HEADER_NAME "Content-Type"

typedef std::array<HttpHeaders::Header, N_COLLECTED_HEADERS> CollectedHeaders;
#define GET_HEADER_BY_NAME(headerName) \
    std::find_if( \
        std::begin(collectedHeaders), \
        std::end(collectedHeaders), \
        [](const HttpHeaders::Header &header) {return header.name.equals(headerName); })

class ClientContext
{
public:
    ClientContext(EthClient &client);

    bool parseRequestHeaderSection();
    HTTP_REQ_TYPE getRequestType() const { return requestType; }
    EthClient &getClient() { return client; }
    uint16_t getRemotePort() const { return remotePort; }
    String getLastModified() const { return GET_HEADER_BY_NAME(IF_MODIFIED_SINCE_HEADER_NAME)->value; }
    size_t getContentLength() const { return atoi(GET_HEADER_BY_NAME(CONTENT_LENGTH_HEADER_NAME)->value.c_str()); }
    String getContentType() const { return GET_HEADER_BY_NAME(CONTENT_TYPE_HEADER_NAME)->value; }
    String getResource() const { return resource; }
    bool keepAlive;

private:
#define N_COLLECTED_HEADERS 3
    CollectedHeaders collectedHeaders;
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
    typedef LinkedList<ViewCreator *> ViewCreatorsList; 
    static ViewCreatorsList viewCreators;
    typedef LinkedList<Controller *> ControllersList;
    static ControllersList controllers;
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h