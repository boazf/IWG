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
#include <MemUtil.h>

String HTTPServer::RequestResource(String &request)
{
    int firstSpace = request.indexOf(' ');
    int secondSpace = request.indexOf(' ', firstSpace + 1);
    return request.substring(firstSpace + 1, secondSpace);
}

LinkedList<View *> HTTPServer::views;

void HTTPServer::AddView(View *view)
{
    views.Insert(view);
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
    String controller;

    if (slashIndex != -1)
    {
        id = resource.substring(slashIndex + 1);
        controller = resource.substring(0, slashIndex);
    }
    else
    {
        id = "";
        controller = resource;
    }

    controller.toUpperCase();

#ifdef DEBUG_HTTP_SERVER
    Trace("controller: ");
    Trace(controller.c_str());
    Trace(" id=");
    Traceln(id);
#endif

    ListNode<Controller *> *controllerNode = controllers.head;

    while (controllerNode)
    {
        if (controllerNode->value->name.equals(controller))
            break;
        controllerNode = controllerNode->next;
    }

    if (controllerNode == NULL)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Controller was not found!");
#endif
        return false;
    }

    switch(requestType)
    {
    case HTTP_GET:
        return controllerNode->value->Get(context->client, id);

    case HTTP_POST:
        return controllerNode->value->Post(context->client, id, context->contentLength, context->contentType);

    case HTTP_PUT:
        return controllerNode->value->Put(context->client, id);

    case HTTP_DELETE:
        return controllerNode->value->Delete(context->client, id);

    default:;
    }

    return false;
}

bool HTTPServer::GetView(const String resource, View *&view, String &id)
{
    view = NULL;

    for (ListNode<View *> *viewNode = views.head; viewNode != NULL; viewNode = viewNode->next)
    {
        if (viewNode->value->viewPath.equals("/"))
        {
            if (resource.equals("/"))
            {
                view = viewNode->value;
                return true;
            }
            continue;
        }
        if (resource.startsWith(viewNode->value->viewPath))
        {
            if (!resource.equals(viewNode->value->viewPath))
                id = resource.substring(viewNode->value->viewPath.length() + 1);
            else
                id = "";
            view = viewNode->value;
            break;
        }

        if (!viewNode->value->viewFilePath.equals("") && resource.startsWith(viewNode->value->viewFilePath))
        {
            return false;
        }
    }

    return true;
}

bool HTTPServer::HandlePostRequest(PClientContext context, const String &resource)
{
    View *view = NULL;
    String id;

    if (!GetView(resource, view, id))
        return false;
    if (view == NULL)
        return false;

    return view->post(context->client, resource, id);
}

bool HTTPServer::HandleGetRequest(PClientContext context, String &resource)
{
    TRACK_FREE_MEMORY(__func__);
    EthernetClient *client = &context->client;
    AutoPtr<View> tempView;
    View *view = NULL;
    String id;

    if (!GetView(resource, view, id))
        return false;

    if (view == NULL)
    {
        tempView.Attach(new FileView(resource.c_str(), resource.c_str()));
        view = tempView;
    }

#ifdef DEBUG_HTTP_SERVER
    Trace("View=");
    Trace(view->viewPath);
    Trace(", id=");
    Trace(id);
    Trace(", Path=");
    Traceln(view->viewFilePath);
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
            Trace("Resource: ");
            Trace(resource);
            Trace(" File was not modified. ");
            Traceln(context->lastModified);
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
    case CONTENT_TYPE::CT_UNKNOWN:
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
    Trace("Done sending ");
    Trace(view->viewFilePath.c_str());
    Trace(" Sent ");
    Trace(bytesSent);
    Traceln(" bytes");
#endif

    view->close();

    return true;
}

void HTTPServer::NotModified(EthernetClient &client)
{
    client.println("HTTP/1.1 304 Not Modified");
    client.println("Content-Length: 0");
    client.println("Server: Arduino");
    client.println("Connection: close"); 
    client.println();
}

void HTTPServer::PageNotFound(EthernetClient &client)
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
#ifdef ESP32
    AutoSD autoSD;
#endif
#ifdef DEBUG_HTTP_SERVER
    Traceln(context->request);
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
    case HTTP_GET:
        if (!HandleGetRequest(context, resource))
            PageNotFound(context->client);
        break;

    case HTTP_POST:
        if (!HandlePostRequest(context, resource))
            PageNotFound(context->client);
        break;

    case HTTP_PUT:
    case HTTP_DELETE:
    case HTTP_UNKNOWN:
        PageNotFound(context->client);
        break;
    }
}

#ifndef ESP32
EthernetServer HTTPServer::server(80);
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

LinkedList<PClientContext> HTTPServer::clients;

#ifdef DEBUG_HTTP_SERVER
#ifndef ESP32
static void PrintIP(const IPAddress &address)
{
    Trace(address[0]);
    Trace(",");
    Trace(address[1]);
    Trace(",");
    Trace(address[2]);
    Trace(",");
    Trace(address[3]);
}
#endif
#endif

void HTTPServer::CheckForNewClients()
{
    EthernetClient client = server.accept();
    while (client)
    {
#ifdef DEBUG_HTTP_SERVER
#ifndef ESP32
        Trace("New client, socket=");
        Trace(client.getSocketNumber());
        Trace(", IP=");
        PrintIP(client.remoteIP());
        Trace(", port=");
        Traceln(client.remotePort());
#else 
        Tracef("New client: IP=%s, port=%d\n", client.remoteIP().toString().c_str(), client.remotePort());
#endif
#endif
        PClientContext context = new ClientContext(client);
        clients.Insert(context);
        client = server.accept();
    }
}

void HTTPServer::ServeClient()
{
    // listen for incoming clients
    CheckForNewClients();

    ListNode<PClientContext> *listNode = clients.head;

#if 0 //TRACK_MEMORY
    static bool empty = false;
    if (listNode == NULL)
    {
        if (!empty)
        {
            Traceln("List is empty");
            Trace("Free mem: ");
            Traceln(freeMemory());
        }
        empty = true;
    }
    else
    {
        if (empty)
        {
            Traceln("List is not empty");
            Trace("Free mem: ");
            Traceln(freeMemory());
        }
        empty = false;
    }
#endif
    
    while(listNode != NULL)
    {
        PClientContext context = listNode->value;
        EthernetClient *client = &context->client;
        word remotePort;
    #ifdef ESP32
        try
        {
            remotePort = client->remotePort();
        }
        catch(...)
        {
            remotePort = 0;
        }
    #else
        remotePort = client->remotePort();
    #endif

        bool brokenClient = listNode->value->remotePort != remotePort;
#ifdef DEBUG_HTTP_SERVER
        if (brokenClient)
        {
            Trace("Broken client: port=");
            Trace(listNode->value->remotePort);
            Trace(", ");
            Traceln(remotePort);
        }
#endif
        if (brokenClient || !client->connected())
        {
#ifndef ESP32
#endif
#ifdef DEBUG_HTTP_SERVER
#ifndef ESP32
            Trace("Client disconnected, port=");
            Traceln(listNode->value->remotePort);
#else
            if (!brokenClient)
                Tracef("Client disconnected, IP=%s, port=%d\n", client->remoteIP().toString().c_str(), client->remotePort());
#endif
#endif
            if (!sseController.DeleteClient(*client, !brokenClient) && !brokenClient)
            {
                client->stop();
            }

            delete listNode->value;
            listNode->value = NULL;
            listNode = clients.Delete(listNode);
            continue;
        }
        if (!client->available()) 
        {
            listNode = listNode->next;
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
        listNode = listNode->next;
    }
}

void InitHTTPServer()
{
    HTTPServer::Init();
}

void DoHTTPService()
{
    HTTPServer::ServeClient();
}
