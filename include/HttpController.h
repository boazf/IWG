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

#ifndef HttpController_h
#define HttpController_h

#include <Arduino.h>
#include <HttpClientContext.h>
#include <memory>

/// @brief This class defines the interface for HTTP controllers.
/// Each controller should implement the methods to handle HTTP requests.
class HttpController
{
public:
    /// @brief Default constructor for the HttpController class.
    HttpController() {}
    /// @brief Virtual destructor for the HttpController class.
    /// This ensures that derived classes can clean up resources properly when deleted through a base class pointer.
    virtual ~HttpController() {}
    /// @brief This method is called to handle a GET request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being requested.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Get(HttpClientContext &client, const String id = "") = 0;
    /// @brief This method is called to handle a POST request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being created or modified.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Post(HttpClientContext &client, const String id = "") = 0;
    /// @brief This method is called to handle a PUT request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being updated.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Put(HttpClientContext &client, const String id = "") = 0;
    /// @brief This method is called to handle a DELETE request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being deleted.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Delete(HttpClientContext &client, const String id = "") = 0;
    /// @brief This method checks if the controller is a singleton.
    /// @return True if the controller is a singleton, false otherwise.
    /// @note A singleton controller means that there is only one instance of the controller in the application.
    /// This is useful for controllers that manage global resources or state.
    /// If a controller is not a singleton, it can be instantiated multiple times, allowing for multiple instances to handle requests independently.
    /// This method should be implemented by derived classes to indicate whether the controller is a singleton or not.
};

/// @brief This type definition is a function pointer that returns an instance of HttpController.
/// It is used to create instances of controllers dynamically.
typedef std::shared_ptr<HttpController> (*GetControllerInstance)();

#endif // HttpController_h