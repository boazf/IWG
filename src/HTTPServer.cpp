/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <Arduino.h>
#include <Common.h>
#include <HTTPServer.h>
#include <LinkedList.h>
#include <AutoPtr.h>
#include <DirectFileView.h>
#include <SSEController.h>

HttpClientContext::HttpClientContext(EthClient &client) :
    keepAlive(false), // default to false
    client(client), // Store the client connection
    requestType(HTTP_REQ_TYPE::HTTP_UNKNOWN), // Initialize request type to unknown
    // Initializes the collected headers with the names of the headers we are interested in
    // This allows us to easily access these headers later in the code.
    collectedHeaders{IF_MODIFIED_SINCE_HEADER_NAME, CONTENT_LENGTH_HEADER_NAME, CONTENT_TYPE_HEADER_NAME} 
{
    remotePort = client.remotePort();
}

bool HttpClientContext::parseRequestHeaderSection()
{
    HttpHeaders headers(client); // Create an instance of HttpHeaders to handle the request headers

    bool res = headers.parseRequestHeaderSection(requestType, resource, collectedHeaders.data(), collectedHeaders.size());
#ifdef DEBUG_HTTP_SERVER
    Tracef("%d %s\n", remotePort, headers.getRequestLine().c_str());
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
    // Get the resource pf the request from the context
    String resource = context->getResource();

    // Prepare the parameters to the function that will be used to find the controller
    // that will handle the request.
    struct Params
    {
        String id;
        String resource;
        HttpController *controller;
    } params = {"", resource, NULL };

    // Scan the list of controllers to find the best matching controller for the request
    controllersData.ScanNodes([](HttpControllerCreatorData const &creatorData, const void *param)->bool
    {
        Params *params = const_cast<Params *>(static_cast<const Params *>(param));
        String path = creatorData.getPath();
        String resource = params->resource;
        resource.toUpperCase();

        if (path.equals("/"))
        {
            // If the path is "/", we need to check if the resource is also "/"
            if (resource.equals("/"))
            {
                // Get an instance for the controller that handles the root path.
                // This is typically the index.htm file or a controller that handles the root path.
                params->controller = creatorData.getInstanceGetter()();
                return false;
            }
            return true; // Stop iterating the list nodes
        }

        if (resource.startsWith(path))
        {
            if (!resource.equals(path))
            {
                // If the resource is not exactly the same as the path, we need to extract the id
                // from the resource. The id is the part of the resource that comes after the path.
                params->id = params->resource.substring(path.length());
                // The id must start with a '/' otherwise it is not a valid id.
                // For example: if the controller path is settings and the resource is /settings3,
                // We don't want to identify the resource as settings and the id as 3.
                // A correct example is /settings/3, where the idresource is settings and the id is 3.
                if (params->id[0] != '/')
                    return true; // Continue searching.
                params->id = params->id.substring(1);
            }
            // If the resource starts with the path, we have found a controller that can handle the request.
            params->controller = creatorData.getInstanceGetter()();
            return false; // Stop searching
        }

        return true; // Continue searching
    }, &params);

    // Extract the results returned from the scan function
    id = params.id;
    controller = params.controller;
    if (controller == NULL)
        // If no controller was found, we return a controller that attemps to open the file specified in the resource
        // part of the URL. If this file exists, the content of the file will be return to the client.
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

    // Get the controller that will handle the request.
    // If no controller is found, we will return a 404 Not Found response.
    if (!GetController(context, controller, id))
    {
        PageNotFound(context->getClient());
        return;
    }

    // Get the request type from the context
    // This is the type of the HTTP request (GET, POST, PUT, DELETE).
    HTTP_REQ_TYPE requestType = context->getRequestType();

    bool ret = false;

    // Call the appropriate method on the controller based on the request type
    // The controller will handle the request and return a response to the client.
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

    // If the request was not handled successfully, we return a 404 Not Found response.
    // This can happen if the controller does not implement the requested method
    // or the controller failed to serve the request.
    if (!ret)
        PageNotFound(context->getClient());

    // If the controller is not singleton, we delete it.
    if (!controller->isSingleton())
        delete controller;
}

#ifndef USE_WIFI
// If we are not using WiFi, we use the Ethernet server
EthServer HTTPServer::server(80);
#else
#define MAX_CLIENTS 12
// If we are using WiFi, we use the WiFi server
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
    // Listen for incoming clients
    EthClient client = server.accept();
    // If no client is connected, return
    if (!client.connected())
        return;
#ifdef DEBUG_HTTP_SERVER
    // Log the new client connection details
#ifndef USE_WIFI
    Tracef("New client: IP=%s, port=%d, socket: %d\n", client.remoteIP().toString().c_str(), client.remotePort(), client.getSocketNumber());
#else
    Tracef("New client: IP=%s, port=%d\n", client.remoteIP().toString().c_str(), client.remotePort());
#endif
#endif
    // Create a new HttpClientContext for the request
    HttpClientContext *context = new HttpClientContext(client);
    BaseType_t ret;
    for (int i = 0; i <= TASK_CREATE_MAX_RETRIES; i++)
    {
        // Create a new task to handle the request
        ret = xTaskCreate(RequestTask, "HTTPRequest", 4*1024, context, tskIDLE_PRIORITY + 1, NULL);
        // If the task was created successfully, break the loop
        // Otherwise, wait for a while and try again
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

    // If the task was not created successfully after the maximum number of retries,
    // drain the client and send a 500 Internal Server Error response.
    if (ret != pdPASS)
    {
#ifdef DEBUG_HTTP_SERVER
        // Log the failure to create the request task
        Tracef("%d Failed to create request task after %d attempts, error = %d\n", client.remotePort(), TASK_CREATE_MAX_RETRIES, ret);
#endif
        // Drain the client
        while(client.available())
        {
            uint8_t buff[128];
            client.read(buff, NELEMS(buff));
        }
        // Send 500 reply
        HttpHeaders headers(client);
        headers.sendHeaderSection(500);
        // Stop the client connection
        client.stop();
        // Delete the context to free resources
        delete context;
    }
}

void HTTPServer::RequestTask(HttpClientContext *context)
{
    EthClient client = context->getClient();

    // Wait for the client to send data
    // We wait for up to 3 seconds for the client to send data
    unsigned long t0 = millis();
    while (!client.available() && millis() - t0 < 3000);
    if (!client.available())
        return;

    // Parse the request header section from the client
    // This will read the HTTP request headers and determine the request type (GET, POST, etc.)
    // It will also extract the requested resource and collect relevant headers.
    if (!context->parseRequestHeaderSection())
    {
        // If the request header section could not be parsed, we send a 400 Bad Request response
        HttpHeaders headers(context->getClient());
        headers.sendHeaderSection(400);
        return;
    }
    // Do the actual request handling
    ServiceRequest(context);
}

void HTTPServer::RequestTask(void *params)
{
    HttpClientContext *context = static_cast<HttpClientContext *>(params);
    // Serve the request
    RequestTask(context);

    // If the client should not be kept alive, then stop the client connection.
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
    // Delete the context to free resources
    delete context;
#ifdef DEBUG_HTTP_SERVER
    Tracef("%d Task stack high watermark: %d\n", context->getRemotePort(), uxTaskGetStackHighWaterMark(NULL));
#endif
    // Delete the task that was created to handle the request
    // This will free the resources allocated for the task
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