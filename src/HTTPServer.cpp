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

bool HTTPServer::DoController(EthernetClient &client, String &resource, HTTP_REQ_TYPE requestType)
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

#ifdef DEBUG_HTTP_SERVER
    Serial.print("controller: ");
    Serial.print(controller.c_str());
    Serial.print(" id=");
    Serial.println(id);
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
        Serial.println("Controller was not found!");
#endif
        return false;
    }

    switch(requestType)
    {
    case HTTP_GET:
        return controllerNode->value->Get(client, id);

    case HTTP_POST:
        return controllerNode->value->Post(client, id);

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
#ifdef ESP32
    AutoSD autoSD;
#endif
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
    Serial.print("View=");
    Serial.print(view->viewPath);
    Serial.print(", id=");
    Serial.print(id);
    Serial.print(", Path=");
    Serial.println(view->viewFilePath);
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
            Serial.print("Resource: ");
            Serial.print(resource);
            Serial.print(" File was not modified. ");
            Serial.println(context->lastModified);
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
        Serial.println("Unknown extention");
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
    Serial.print("Done sending ");
    Serial.print(view->viewFilePath.c_str());
    Serial.print(" Sent ");
    Serial.print(bytesSent);
    Serial.println(" bytes");
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
        context->reqLine = "";
        return true;
    }

    return false;
}


void HTTPServer::ServiceRequest(PClientContext context)
{
#ifdef DEBUG_HTTP_SERVER
    Serial.println(context->request);
#endif
    String resource = RequestResource(context->request);
    resource.toUpperCase();
    HTTP_REQ_TYPE requestType = RequestType(context->request);

    if (resource.startsWith("/API"))
    {
        String controller = resource.substring(5);
        if (!DoController(context->client, controller, requestType))
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
    Serial.println("HTTP Server has started");
#endif
}

LinkedList<PClientContext> HTTPServer::clients;

#ifdef DEBUG_HTTP_SERVER
#ifndef ESP32
static void PrintIP(const IPAddress &address)
{
    Serial.print(address[0]);
    Serial.print(",");
    Serial.print(address[1]);
    Serial.print(",");
    Serial.print(address[2]);
    Serial.print(",");
    Serial.print(address[3]);
}
#endif
#endif

void HTTPServer::CheckForNewClients()
{
    EthernetClient client = server.accept();
    while (client)
    {
#ifdef DEBUG_HTTP_SERVER
        Serial.print("New client, socket=");
#ifndef ESP32
        Serial.print(client.getSocketNumber());
        Serial.print(", IP=");
        PrintIP(client.remoteIP());
        Serial.print(", port=");
        Serial.println(client.remotePort());
#else 
        Serial.printf("New client: IP=%s, port=%d\n", client.remoteIP().toString().c_str(), client.remotePort());
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
            Serial.println("List is empty");
            Serial.print("Free mem: ");
            Serial.println(freeMemory());
        }
        empty = true;
    }
    else
    {
        if (empty)
        {
            Serial.println("List is not empty");
            Serial.print("Free mem: ");
            Serial.println(freeMemory());
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
            Serial.print("Broken client: port=");
            Serial.print(listNode->value->remotePort);
            Serial.print(", ");
            Serial.println(remotePort);
        }
#endif
        if (brokenClient || !client->connected())
        {
#ifndef ESP32
#endif
#ifdef DEBUG_HTTP_SERVER
#ifndef ESP32
            Serial.print("Client disconnected, port=");
            Serial.println(listNode->value->remotePort);
#else
            if (!brokenClient)
                Serial.printf("Client disconnected, IP=%s, port=%d\n", client->remoteIP().toString().c_str(), client->remotePort());
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
