#ifndef HTTPServer_h
#define HTTPServer_h

#include <SPI.h>
#include <EthernetUtil.h>
#include <View.h>
#include <Controller.h>
#include <LinkedList.h>

typedef struct ClientContext_
{
    ClientContext_(EthClient _client)
    {
        client = _client;
        remotePort = _client.remotePort();
    }
    EthClient client;
    word remotePort;
    String reqLine;
    String request;
    String lastModified;
    size_t contentLength;
    String contentType;
} ClientContext, *PClientContext;

enum HTTP_REQ_TYPE
{
    HTTP_UNKNOWN,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE
};

class HTTPServer
{
public:
    static void Init();
    static void ServeClient();
    static void AddView(View *view);
    static void AddController(Controller *controller);

private:
    static void CheckForNewClients();
    static String RequestResource(String &request);
    static bool DoController(PClientContext context, String &resource, HTTP_REQ_TYPE requestType);
    static bool GetView(const String resource, View *&view, String &id);
    static bool HandlePostRequest(PClientContext context, const String &resource);
    static bool HandleGetRequest(PClientContext context, String &resource);
    static void NotModified(EthClient &client);
    static void PageNotFound(EthClient &client);
    static HTTP_REQ_TYPE RequestType(String &request);
    static bool ProcessLine(PClientContext context);
    static void ServiceRequest(PClientContext context);

private:
    static EthServer server;
    static LinkedList<View *> views;
    static LinkedList<PClientContext> clients;
};

void InitHTTPServer();
void DoHTTPService();

#endif // HTTPServer_h