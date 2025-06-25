#include <Arduino.h>
#include <Common.h>
#include <HTTPServer.h>
#include <LinkedList.h>
#include <AutoPtr.h>
#include <DirectFileView.h>
#include <SSEController.h>

HttpClientContext::HttpClientContext(EthClient &client) :
    keepAlive(false),
    client(client), 
    requestType(HTTP_REQ_TYPE::HTTP_UNKNOWN),
    collectedHeaders{IF_MODIFIED_SINCE_HEADER_NAME, CONTENT_LENGTH_HEADER_NAME, CONTENT_TYPE_HEADER_NAME}
{
    remotePort = client.remotePort();
}

bool HttpClientContext::parseRequestHeaderSection()
{
    HttpHeaders headers(client);

    bool res = headers.parseRequestHeaderSection(requestType, resource, collectedHeaders.data(), collectedHeaders.size());
#ifdef DEBUG_HTTP_SERVER
    Tracef("%d %s\n", remotePort, headers.getRequestLine().c_str());
#endif        
#ifdef DEBUG_HTTP_SERVER
    if (!res)
        Tracef("%d Bad HTTP request: \"%s\"\n", remotePort, headers.getLastParsedLine().c_str());
#endif        

    return res;
}

HTTPServer::ControllersDataList HTTPServer::controllersData;

void HTTPServer::AddController(const String path, GetControllerInstance getControllerInstance)
{
    HttpControllerCreatorData creatorData(path, getControllerInstance);
    controllersData.Insert(creatorData);
}

bool HTTPServer::GetController(HttpClientContext *context, HttpController *&controller, String &id)
{
    String resource = context->getResource();

    struct Params
    {
        String id;
        String resource;
        HttpController *controller;
    } params = {"", resource, NULL };

    controllersData.ScanNodes([](HttpControllerCreatorData const &creatorData, const void *param)->bool
    {
        Params *params = const_cast<Params *>(static_cast<const Params *>(param));
        String path = creatorData.getPath();
        String resource = params->resource;
        resource.toUpperCase();

        if (path.equals("/"))
        {
            if (resource.equals("/"))
            {
                params->controller = creatorData.getInstanceGetter()();
                return false;
            }
            return true;
        }
        if (resource.startsWith(path))
        {
            if (!resource.equals(path))
            {
                params->id = params->resource.substring(path.length());
                if (params->id[0] != '/')
                    return false;
                params->id = params->id.substring(1);
            }
            params->controller = creatorData.getInstanceGetter()();
            return false;
        }

        return true;
    }, &params);

    id = params.id;
    controller = params.controller;
    if (controller == NULL)
        controller = new DirectFileView(resource.c_str());
    return controller != NULL;
}

void HTTPServer::NotModified(EthClient &client)
{
    HttpHeaders headers(client);
    headers.sendHeaderSection(304);
}

void HTTPServer::PageNotFound(EthClient &client)
{
    HttpHeaders headers(client);
    headers.sendHeaderSection(404);
}

void HTTPServer::ServiceRequest(HttpClientContext *context)
{
    HttpController *controller;
    String id;

    if (!GetController(context, controller, id))
    {
        PageNotFound(context->getClient());
        return;
    }

    HTTP_REQ_TYPE requestType = context->getRequestType();

    bool ret = false;

    switch(requestType)
    {
    case HTTP_REQ_TYPE::HTTP_GET:
        ret = controller->Get(*context, id);
        break;

    case HTTP_REQ_TYPE::HTTP_POST:
        ret = controller->Post(*context, id);
        break;

    case HTTP_REQ_TYPE::HTTP_PUT:
        ret = controller->Put(*context, id);
        break;

    case HTTP_REQ_TYPE::HTTP_DELETE:
        ret = controller->Delete(*context, id);
        break;

    default:
        break;
    }

    if (!ret)
        PageNotFound(context->getClient());

    if (!controller->isSingleton())
        delete controller;
}

#ifndef USE_WIFI
EthServer HTTPServer::server(80);
#else
#define MAX_CLIENTS 12
WiFiServer HTTPServer::server(80, MAX_CLIENTS);
#endif

void HTTPServer::Init()
{
    server.begin();
#ifdef DEBUG_HTTP_SERVER
    Traceln("HTTP Server has started");
#endif
}

#define TASK_CREATE_MAX_RETRIES 20

void HTTPServer::ServeClient()
{
    // listen for incoming clients
    EthClient client = server.accept();
    if (!client.connected())
        return;
#ifdef DEBUG_HTTP_SERVER
#ifndef USE_WIFI
    Tracef("New client: IP=%s, port=%d, socket: %d\n", client.remoteIP().toString().c_str(), client.remotePort(), client.getSocketNumber());
#else
    Tracef("New client: IP=%s, port=%d\n", client.remoteIP().toString().c_str(), client.remotePort());
#endif
#endif
    HttpClientContext *context = new HttpClientContext(client);
    BaseType_t ret;
    for (int i = 0; i <= TASK_CREATE_MAX_RETRIES; i++)
    {
        ret = xTaskCreate(RequestTask, "HTTPRequest", 4*1024, context, tskIDLE_PRIORITY + 1, NULL);
        if (ret == pdPASS)
        {
#ifdef DEBUG_HTTP_SERVER
            if (i > 0)
                Tracef("%d Succeeded to create request task after %d retries\n", client.remotePort(), i);
#endif
            break;
        }
#ifdef DEBUG_HTTP_SERVER
        if (i == 0)
            Tracef("%d Failed to create request task, error = %d will attempt again\n", client.remotePort(), ret);
#endif
        delay(1000); // Wait before reattempting creating the task
    }

    if (ret != pdPASS)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("%d Failed to create request task after %d attempts, error = %d\n", client.remotePort(), TASK_CREATE_MAX_RETRIES, ret);
#endif
        // Drain the client
        while(client.available())
        {
            uint8_t buff[128];
            client.read(buff, NELEMS(buff));
        }
        // Send reply
        HttpHeaders headers(client);
        headers.sendHeaderSection(500);
        client.stop();
        delete context;
    }
}

void HTTPServer::RequestTask(HttpClientContext *context)
{
    EthClient client = context->getClient();
    unsigned long t0 = millis();
    while (!client.available() && millis() - t0 < 3000);
    if (!client.available())
        return;
    if (!context->parseRequestHeaderSection())
    {
        HttpHeaders headers(context->getClient());
        headers.sendHeaderSection(400);
        return;
    }
    ServiceRequest(context);
}

void HTTPServer::RequestTask(void *params)
{
    HttpClientContext *context = static_cast<HttpClientContext *>(params);
    RequestTask(context);
    if (!context->keepAlive)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("%d Stopping client\n", context->getRemotePort());
#endif
        context->getClient().stop();
    }
#ifdef DEBUG_HTTP_SERVER
    else
        Tracef("%d Keeping alive\n", context->getRemotePort());
#endif
    delete context;
#ifdef DEBUG_HTTP_SERVER
    Tracef("%d Task stack high watermark: %d\n", context->getRemotePort(), uxTaskGetStackHighWaterMark(NULL));
#endif
    vTaskDelete(NULL);
}

void InitHTTPServer()
{
    HTTPServer::Init();
}

void DoHTTPService()
{
    HTTPServer::ServeClient();
}