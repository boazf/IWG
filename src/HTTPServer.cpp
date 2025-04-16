#include <Arduino.h>
#include <Common.h>
#include <HTTPServer.h>
#include <LinkedList.h>
#include <AutoPtr.h>
#include <FileView.h>
#include <SSEController.h>

ClientContext::ClientContext(EthClient &client) :
    keepAlive(false),
    client(client), 
    requestType(HTTP_REQ_TYPE::HTTP_UNKNOWN),
    collectedHeaders{IF_MODIFIED_SINCE_HEADER_NAME, CONTENT_LENGTH_HEADER_NAME, CONTENT_TYPE_HEADER_NAME}
{
    remotePort = client.remotePort();
}

bool ClientContext::parseRequestHeaderSection()
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

HTTPServer::ViewCreatorsList HTTPServer::viewCreators;
HTTPServer::ControllersList HTTPServer::controllers;

void HTTPServer::AddView(ViewCreator *viewCreator)
{
    viewCreators.Insert(viewCreator);
}

void HTTPServer::AddController(Controller *controller)
{
    controllers.Insert(controller);
}

bool HTTPServer::DoController(ClientContext *context, String &resource, HTTP_REQ_TYPE requestType)
{
    int slashIndex = resource.indexOf('/');
    String id;
    String controllerName;

    if (slashIndex != -1)
    {
        id = resource.substring(slashIndex + 1);
        controllerName = resource.substring(0, slashIndex);
    }
    else
    {
        id = "";
        controllerName = resource;
    }

    controllerName.toUpperCase();

#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Tracef("%d ", context->getClient().remotePort());
        Trace("controller: ");
        Trace(controllerName.c_str());
        Trace(" id=");
        Traceln(id);
    }
#endif

    struct Params
    {
        Controller *controller;
        String name;
    } params = { NULL, controllerName};
    
    controllers.ScanNodes([](Controller *const &controllerInst, const void *param)->bool
    {
        Params *params = (Params *)param;

        if (controllerInst->name.equals(params->name))
        {
            params->controller = controllerInst;
            return false;
        }
        return true;
    }, &params);

    Controller *controller = params.controller;

    if (controller == NULL)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("%d Controller was not found!\n", context->getClient().remotePort());
#endif
        return false;
    }

    ControllerContext controllerContext(context->getContentLength(), context->getContentType());
    bool ret = false;

    switch(requestType)
    {
    case HTTP_REQ_TYPE::HTTP_GET:
        ret = controller->Get(context->getClient(), id, controllerContext);
        break;

    case HTTP_REQ_TYPE::HTTP_POST:
        ret = controller->Post(context->getClient(), id, controllerContext);
        break;

    case HTTP_REQ_TYPE::HTTP_PUT:
        ret = controller->Put(context->getClient(), id, controllerContext);
        break;

    case HTTP_REQ_TYPE::HTTP_DELETE:
        ret = controller->Delete(context->getClient(), id, controllerContext);
        break;

    default:;
        break;
    }

    context->keepAlive = controllerContext.keepAlive;

    return ret;
}

bool HTTPServer::GetView(const String resource, View *&view, String &id)
{
    struct Params
    {
        String id;
        String resource;
        View *view;
        bool ret;
    } params = { id, resource, NULL, true };

    viewCreators.ScanNodes([](ViewCreator *const &viewCreatorInst, const void *param)->bool
    {
        Params *params = (Params *)param;

        if (viewCreatorInst->viewPath.equals("/"))
        {
            if (params->resource.equals("/"))
            {
                params->view = viewCreatorInst->createView();
                return false;
            }
            return true;
        }
        if (params->resource.startsWith(viewCreatorInst->viewPath))
        {
            if (!params->resource.equals(viewCreatorInst->viewPath))
                params->id = params->resource.substring(viewCreatorInst->viewPath.length() + 1);
            else
                params->id = "";
            params->view = viewCreatorInst->createView();
            return false;
        }

        if (!viewCreatorInst->viewFilePath.equals("") && params->resource.startsWith(viewCreatorInst->viewFilePath))
        {
            params->ret = false;
            return false;
        }

        return true;
    }, &params);

    id = params.id;
    view = params.view;
    return params.ret;
}

bool HTTPServer::HandlePostRequest(ClientContext *context, const String &resource)
{
    View *view;
    String id;

    if (!GetView(resource, view, id))
        return false;
    if (view == NULL)
        return false;

    bool ret = view->post(context->getClient(), resource, id);
    delete view;

    return ret;
}

bool HTTPServer::HandleGetRequest(ClientContext *context, String &resource)
{
    EthClient *client = &context->getClient();
    AutoPtr<View> tempView;
    View *view = NULL;
    String id;

    if (!GetView(resource, view, id))
        return false;

    if (view == NULL)
    {
        view = new FileView(resource.c_str(), resource.c_str());
    }

    tempView.Attach(view);

#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Tracef("%d ", context->getClient().remotePort());
        Trace("View=");
        Trace(view->viewPath);
        Trace(", id=");
        Trace(id);
        Trace(", Path=");
        Traceln(view->viewFilePath);
    }
#endif

    if (view->redirect(*client, id))
        return true;

    byte buff[256];
    if (!view->open(buff, sizeof(buff)))
    {
        return false;
    }

    if (!context->getLastModified().isEmpty())
    {
        String lastModifiedTime;

        view->getLastModifiedTime(lastModifiedTime);
        if (context->getLastModified().equals(lastModifiedTime))
        {
#ifdef DEBUG_HTTP_SERVER
            {
                LOCK_TRACE();
                Tracef("%d ", context->getClient().remotePort());
                Trace("Resource: ");
                Trace(resource);
                Trace(" File was not modified. ");
                Traceln(context->getLastModified());
            }
#endif
            NotModified(*client);
            view->close();
            return true;
        }
    }

    CONTENT_TYPE type = view->getContentType();
    if (type == CONTENT_TYPE::UNKNOWN)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Unknown extention");
#endif
        view->close();
        return false;
    }

    long size = view->getViewSize();

    HttpHeaders::Header additionalHeaders[] = { {type}, {} };
    if (type != CONTENT_TYPE::HTML)
    {
        String lastModifiedTime;
        if (view->getLastModifiedTime(lastModifiedTime))
        {
            additionalHeaders[1] = {"Last-Modified", lastModifiedTime};
        }
    }

    HttpHeaders headers(*client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), size);

    long bytesSent = 0;
    while (bytesSent < size)
    {
        int nBytes = view->read();
        client->write(buff, nBytes);
        bytesSent += nBytes;
    }

#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Tracef("%d ", context->getClient().remotePort());
        Trace("Done sending ");
        Trace(view->viewFilePath.c_str());
        Trace(" Sent ");
        Trace(bytesSent);
        Traceln(" bytes");
    }
#endif

    view->close();

    return true;
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

void HTTPServer::ServiceRequest(ClientContext *context)
{
    String resource = context->getResource();
    String resourceOrg = resource;
    resource.toUpperCase();
    HTTP_REQ_TYPE requestType = context->getRequestType();

    if (resource.startsWith("/API"))
    {
        String controller = resourceOrg.substring(5);
        if (!DoController(context, controller, requestType))
            PageNotFound(context->getClient());
        return;
    }

    switch(requestType)
    {
    case HTTP_REQ_TYPE::HTTP_GET:
        if (!HandleGetRequest(context, resource))
            PageNotFound(context->getClient());
        break;

    case HTTP_REQ_TYPE::HTTP_POST:
        if (!HandlePostRequest(context, resource))
            PageNotFound(context->getClient());
        break;

    case HTTP_REQ_TYPE::HTTP_PUT:
    case HTTP_REQ_TYPE::HTTP_DELETE:
    case HTTP_REQ_TYPE::HTTP_UNKNOWN:
        PageNotFound(context->getClient());
        break;
    }
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
    ClientContext *context = new ClientContext(client);
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

void HTTPServer::RequestTask(ClientContext *context)
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
    ClientContext *context = (ClientContext *)params;
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