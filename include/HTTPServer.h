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

#ifndef HTTPServer_h
#define HTTPServer_h

#include <EthernetUtil.h>
#include <FileView.h>
#include <HttpController.h>
#include <LinkedList.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

/// @brief This is a class that implements an HTTP server.
/// It handles incoming HTTP requests, routes them to the appropriate controllers,
/// and serves static files or dynamic content based on the request type.
class HTTPServer
{
public:
    /// @brief Initializes the HTTP server.
    /// This method sets up the server to listen for incoming connections on port 80.
    static void Init();
    /// @brief This method should repeatedly be called from the program loop.
    /// It checks for incoming client connections and serves requests.
    /// The request is actually served in a task that is created for each client request.
    static void ServeClient();
    /// @brief Adds a controller that serves specific client requests. Per each client request,
    /// the server will check if the request matches the controller's path and call the appropriate method
    /// (Get, Post, Put, Delete) based on the request type.
    /// @param path The path that the controller will handle. This is typically a URL path that the client requests.
    /// @param getControllerInstance A function pointer that returns an instance of the controller.
    static void AddController(const String path, GetControllerInstance getControllerInstance);

public:
    /// @brief Send a 304 Not Modified response to the client.
    /// This method is called when the requested resource has not been modified since the last request,
    /// @param client The EthClient instance representing the client connection.
    static void NotModified(EthClient &client);
    /// @brief Sends a 404 Not Found response to the client.
    /// This method is called when the requested resource is not found on the server.
    /// @param client The EthClient instance representing the client connection.
    static void PageNotFound(EthClient &client);

private:
    /// @brief Gets the controller instance for the specified path.
    /// This method searches through the registered controllers to find one that matches the given path.
    /// @param context The HTTP client context containing the request information.
    /// @param controller A reference to a pointer that will be set to the controller instance if found.
    /// @param id An optional identifier for the resource being requested.
    /// @return Returns true if a controller was found and set, false otherwise.
    static bool GetController(HttpClientContext *context, HttpController *&controller, String &id);
    /// @brief Handles the HTTP request by routing it to the appropriate controller.
    /// This method checks the request type and calls the corresponding method on the controller.
    static void ServiceRequest(HttpClientContext *context);
    /// @brief The task function that handles the client request
    /// @param params Pointer to parameters passed to the task, which is actually an HttpClientContext pointer.
    /// This function is called when a new client connection is accepted.
    /// @note This function calls the RequestTask(HttpClientContext*) function. It is doing initialization before calling
    /// the RequestTask(HttpClientContext*) function and is doing cleanup after calling it. This separation is done to 
    /// allow for better organization and readability of the code.
    static void RequestTask(void *params);
    /// @brief Requests a task to handle the client request.
    /// @param context The HTTP client context containing the request information.
    /// This method is called when a new client connection is accepted, and it creates a task to handle the request asynchronously.
    /// It allows the server to handle multiple client requests concurrently without blocking the main loop.
    /// @note This method is called from the RequestTask(void*) function. It is doing the actual work of handling the request.
    /// It is separated from the RequestTask(void*) function to allow for better organization and readability
    static void RequestTask(HttpClientContext *context);

private:
    /// @brief The server instance that listens for incoming connections.
    static EthServer server;
    /// @brief THis class handles creation of HttpControllers
    class HttpControllerCreatorData{
    public:    
        /// @brief The class constructor for HttpControllerCreatorData.
        /// @param path The path that the controller handles. This is typically a URL path that the client requests.
        /// @param instanceGetter A function pointer that returns an instance of the controller.
        HttpControllerCreatorData(const String path, GetControllerInstance instanceGetter) :
            path(path),
            instanceGetter(instanceGetter)
        {}

        /// @brief A copy constructor for HttpControllerCreatorData.
        /// @param other Another instance of HttpControllerCreatorData to copy from.
        HttpControllerCreatorData(const HttpControllerCreatorData &other) : 
            path(other.path),
            instanceGetter(other.instanceGetter)
        {}

        /// @brief Returns the path that this controller handles.
        /// @return Returns the path as a String.
        const String getPath() const { return path; }
        /// @brief Returns the function pointer that returns an instance of the controller.
        /// @return Returns the function pointer as a GetControllerInstance type. 
        const GetControllerInstance getInstanceGetter() const { return instanceGetter; }
    private:
        /// @brief The path that this controller handles.
        const String path;
        /// @brief A function pointer that returns an instance of the controller.
        GetControllerInstance instanceGetter;
    };

    typedef LinkedList<HttpControllerCreatorData> ControllersDataList;
    /// @brief A list of controllers that handle specific client requests.
    /// This list is used to store the registered controllers and their associated paths.
    /// Upon receiving a client request, the server will check this list to find a matching controller
    /// and will retrieve an instance of the controller to handle the request.
    static ControllersDataList controllersData;
};

/// @brief Initializes the HTTP server.
/// This function should be called once at the beginning of the program to set up the server.
void InitHTTPServer();
/// @brief Repeatedly calls the HTTP service function.
/// This function should be called in the main loop of the program to handle incoming client requests.
void DoHTTPService();

#endif // HTTPServer_h