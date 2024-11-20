#include <Arduino.h>
#include <HTTPServer.h>
#include <SDUtil.h>
#include <Config.h>
#include <LinkedList.h>
#include <AutoPtr.h>
#include <Common.h>
#include <time.h>
#include <FileView.h>
#include <SSEController.h>

String HTTPServer::RequestResource(String &request)
{
    int firstSpace = request.indexOf(' ');
    int secondSpace = request.indexOf(' ', firstSpace + 1);
    return request.substring(firstSpace + 1, secondSpace);
}

LinkedList<ViewCreator *> HTTPServer::viewCreators;

void HTTPServer::AddView(ViewCreator *viewCreator)
{
    viewCreators.Insert(viewCreator);
}

static LinkedList<Controller *> controllers;

void HTTPServer::AddController(Controller *controller)
{
    controllers.Insert(controller);
}

bool HTTPServer::DoController(PClientContext context, String &resource, HTTP_REQ_TYPE requestType)
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
        Tracef("%d ", context->client.remotePort());
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
        Tracef("%d Controller was not found!\n", context->client.remotePort());
#endif
        return false;
    }

    switch(requestType)
    {
    case HTTP_REQ_TYPE::HTTP_GET:
        return controller->Get(context->client, id);

    case HTTP_REQ_TYPE::HTTP_POST:
        return controller->Post(context->client, id, context->contentLength, context->contentType);

    case HTTP_REQ_TYPE::HTTP_PUT:
        return controller->Put(context->client, id);

    case HTTP_REQ_TYPE::HTTP_DELETE:
        return controller->Delete(context->client, id);

    default:;
    }

    return false;
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

bool HTTPServer::HandlePostRequest(PClientContext context, const String &resource)
{
    View *view;
    String id;

    if (!GetView(resource, view, id))
        return false;
    if (view == NULL)
        return false;

    bool ret = view->post(context->client, resource, id);
    delete view;

    return ret;
}

bool HTTPServer::HandleGetRequest(PClientContext context, String &resource)
{
    EthClient *client = &context->client;
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
        Tracef("%d ", context->client.remotePort());
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

    if (!context->lastModified.equals(""))
    {
        String lastModifiedTime;

        view->getLastModifiedTime(lastModifiedTime);
        if (context->lastModified.equals(lastModifiedTime))
        {
#ifdef DEBUG_HTTP_SERVER
            {
                LOCK_TRACE();
                Tracef("%d ", context->client.remotePort());
                Trace("Resource: ");
                Trace(resource);
                Trace(" File was not modified. ");
                Traceln(context->lastModified);
            }
#endif
            NotModified(*client);
            view->close();
            return true;
        }
    }

    CONTENT_TYPE type = view->getContentType();
    String contentTypeHeader("Content-Type: ");
    switch (type)
    {
    case CONTENT_TYPE::HTML:
        contentTypeHeader += "text/html";
        break;
    case CONTENT_TYPE::ICON:
        contentTypeHeader += "image/x-icon";
        break;
    case CONTENT_TYPE::JPEG:
        contentTypeHeader += "image/x-jpeg";
        break;
    case CONTENT_TYPE::JAVASCRIPT:
        contentTypeHeader += "application/javascript";
        break;
    case CONTENT_TYPE::CSS:
        contentTypeHeader += "text/css";
        break;
    case CONTENT_TYPE::EOT:
        contentTypeHeader += "application/vnd.ms-fontobject";
        break;
    case CONTENT_TYPE::SVG:
        contentTypeHeader += "image/svg+xml";
        break;
    case CONTENT_TYPE::TTF:
        contentTypeHeader += "font/ttf";
        break;
    case CONTENT_TYPE::WOFF:
        contentTypeHeader += "font/woff";
        break;
    case CONTENT_TYPE::WOFF2:
        contentTypeHeader += "font/woff2";
        break;
    case CONTENT_TYPE::UNKNOWN:
#ifdef DEBUG_HTTP_SERVER
        Traceln("Unknown extention");
#endif
        view->close();
        return false;
        break;
    }

    long size = view->getViewSize();

    client->println("HTTP/1.1 200 OK");
    client->print("Content-Length: ");
    client->println(size);
    client->println(contentTypeHeader);
    client->println("Connection: close"); 
    client->println("Server: Arduino");
    if (type != CONTENT_TYPE::HTML)
    {
        String lastModifiedTime;
        if (view->getLastModifiedTime(lastModifiedTime))
        {
            client->print("Last-Modified: ");
            client->println(lastModifiedTime);
        }
    }
    client->println();

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
        Tracef("%d ", context->client.remotePort());
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
    client.println("HTTP/1.1 304 Not Modified");
    client.println("Content-Length: 0");
    client.println("Server: Arduino");
    client.println("Connection: close"); 
    client.println();
}

void HTTPServer::PageNotFound(EthClient &client)
{
    // const char *lines[] =
    // {
    //     "<!DOCTYPE html>",
    //     "<html lang=\"en\" xmlns=\"http://www.w3.org/1999/xhtml\">",
    //     "<head>",
    //     "<meta charset=\"utf-8\" />",
    //     "<title>404 - Not Found</title>",
    //     "</head>",
    //     "<body>",
    //     "<img alt=\"sad-computer\" src=\"/Images/sadcomp.jpg\" width=\"50%\" style=\"position:fixed; z-index:-1\" />",
    //     "<h1 style=\"z-index:1; color:red\">404 - Not Found :(</h1>",
    //     "</body>",
    //     "</html>"
    // };

    // int len = 0;
    // for (size_t i = 0; i < NELEMS(lines); i++)
    //     len += strlen(lines[i])+2;
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Length: 0");
    // client.println(len);
    client.println("Server: Arduino");
    client.println("Connection: close"); 
    client.println();
    // for (size_t i = 0; i < NELEMS(lines); i++)
    //     client.println(lines[i]);
}

HTTP_REQ_TYPE HTTPServer::RequestType(String &request)
{
    if (strncmp(request.c_str(), "GET ", 4) == 0)
        return HTTP_REQ_TYPE::HTTP_GET;
    else if (strncmp(request.c_str(), "POST ", 5) == 0)
        return HTTP_REQ_TYPE::HTTP_POST;
    else if (strncmp(request.c_str(), "PUT ", 4) == 0)
        return HTTP_REQ_TYPE::HTTP_PUT;
    else if (strncmp(request.c_str(), "DELETE ", 7) == 0)
        return HTTP_REQ_TYPE::HTTP_DELETE;

    return HTTP_REQ_TYPE::HTTP_UNKNOWN;
}

bool HTTPServer::ProcessLine(PClientContext context)
{
    if (!context->reqLine.equals(""))
    {
        if (context->request.equals(""))
            context->request = context->reqLine;
        else if (context->reqLine.startsWith("If-Modified-Since: "))
            context->lastModified = context->reqLine.substring(context->reqLine.indexOf(' ') + 1);
        else if (context->reqLine.startsWith("Content-Length: "))
            sscanf(context->reqLine.substring(context->reqLine.indexOf(' ') + 1).c_str(), "%u", &(context->contentLength));
        else if (context->reqLine.startsWith("Content-Type: "))
            context->contentType = context->reqLine.substring(context->reqLine.indexOf(' ') + 1);
        context->reqLine = "";
        return true;
    }

    return false;
}


void HTTPServer::ServiceRequest(PClientContext context)
{
    AutoSD autoSD;
#ifdef DEBUG_HTTP_SERVER
    Tracef("%d %s\n", context->client.remotePort(), context->request.c_str());
#endif
    String resource = RequestResource(context->request);
    String resourceOrg = resource;
    resource.toUpperCase();
    HTTP_REQ_TYPE requestType = RequestType(context->request);

    if (resource.startsWith("/API"))
    {
        String controller = resourceOrg.substring(5);
        if (!DoController(context, controller, requestType))
            PageNotFound(context->client);
        return;
    }

    switch(RequestType(context->request))
    {
    case HTTP_REQ_TYPE::HTTP_GET:
        if (!HandleGetRequest(context, resource))
            PageNotFound(context->client);
        break;

    case HTTP_REQ_TYPE::HTTP_POST:
        if (!HandlePostRequest(context, resource))
            PageNotFound(context->client);
        break;

    case HTTP_REQ_TYPE::HTTP_PUT:
    case HTTP_REQ_TYPE::HTTP_DELETE:
    case HTTP_REQ_TYPE::HTTP_UNKNOWN:
        PageNotFound(context->client);
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
    while (client)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("New client: IP=%s, port=%d\n", client.remoteIP().toString().c_str(), client.remotePort());
#endif
        PClientContext context = new ClientContext(client);
        TaskHandle_t requestTaskHandle;
        BaseType_t ret;
        for (int i = 0; i <= TASK_CREATE_MAX_RETRIES; i++)
        {
            ret = xTaskCreate(RequestTask, "HTTPRequest", 8*1024, context, tskIDLE_PRIORITY + 1, &requestTaskHandle);
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
            client.println("HTTP/1.1 500 Internal Server Error");
            client.println("Connection: close"); 
            client.println("Server: Arduino");
            client.println();
            client.stop();
            delete context;
        }
        client = server.accept();
    }
}

void HTTPServer::RequestTask(void *params)
{
    PClientContext context = (PClientContext)params;
    EthClient *client = &context->client;
    do
    {
        delay(1);
        uint16_t remotePort;
        try
        {
            remotePort = client->remotePort();
        }
        catch(...)
        {
            remotePort = 0;
        }

        bool brokenClient = context->remotePort != remotePort;
#ifdef DEBUG_HTTP_SERVER
        if (brokenClient)
        {
            LOCK_TRACE();
            Trace("Broken client: port=");
            Trace(context->remotePort);
            Trace(", ");
            Traceln(remotePort);
        }
#endif
        if (brokenClient || !client->connected())
        {
#ifdef DEBUG_HTTP_SERVER
#ifndef USE_WIFI
            {
                LOCK_TRACE();
                Trace("Client disconnected, port=");
                Traceln(context->remotePort);
            }
#else
            if (!brokenClient)
                Tracef("Client disconnected, IP=%s, port=%d\n", client->remoteIP().toString().c_str(), client->remotePort());
#endif
#endif
            if (!sseController.DeleteClient(*client, !brokenClient) && !brokenClient)
            {
                client->stop();
            }

#ifdef DEBUG_HTTP_SERVER
            Tracef("%d Task stack high watermark: %d\n", context->remotePort, uxTaskGetStackHighWaterMark(NULL));
#endif
            delete context;
            vTaskDelete(NULL);
        }
        if (!client->available()) 
        {
            continue;
        }
        char c = client->read();
        if (c == '\r');
        else if (c == '\n')
        {
            if (!ProcessLine(context))
                ServiceRequest(context);
        }
        else
            context->reqLine += c;
    }
    while(true);
}

void InitHTTPServer()
{
    HTTPServer::Init();
}

void DoHTTPService()
{
    HTTPServer::ServeClient();
}
