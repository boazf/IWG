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

enum class HttpMethod
{
    GET = 1,
    POST = 2,
    PUT = 4,
    DELETE = 8,
    ALL = GET | POST | PUT | DELETE
};

inline HttpMethod operator|(HttpMethod lhs, HttpMethod rhs) { return static_cast<HttpMethod>(static_cast<int>(lhs) | static_cast<int>(rhs)); }
inline HttpMethod& operator|=(HttpMethod &lhs, HttpMethod rhs) { lhs = lhs | rhs; return lhs; }
inline HttpMethod operator&(HttpMethod lhs, HttpMethod rhs) { return static_cast<HttpMethod>(static_cast<int>(lhs) & static_cast<int>(rhs)); }
inline HttpMethod& operator&=(HttpMethod &lhs, HttpMethod rhs) { lhs = lhs & rhs; return lhs; }

/// @brief This class defines the interface for HTTP controllers.
/// Each controller should implement the methods to handle HTTP requests.
class HttpController
{
public:
    /// @brief Default constructor for the HttpController class.
    HttpController(HttpMethod methods = HttpMethod::GET) : methods(methods) {}

    /// @brief Virtual destructor for the HttpController class.
    /// This ensures that derived classes can clean up resources properly when deleted through a base class pointer.
    virtual ~HttpController() {}

    /// @brief This method is called to handle a GET request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being requested.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Get(HttpClientContext &client, const String id = "")
    {
        throw std::logic_error("GET method not implemented");
    }

    /// @brief This method is called to handle a POST request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being created or modified.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Post(HttpClientContext &client, const String id = "")
    {
        throw std::logic_error("POST method not implemented");
    }

    /// @brief This method is called to handle a PUT request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being updated.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Put(HttpClientContext &client, const String id = "")
    {
        throw std::logic_error("PUT method not implemented");
    }

    /// @brief This method is called to handle a DELETE request.
    /// @param client The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being deleted.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource
    virtual bool Delete(HttpClientContext &client, const String id = "")
    {
        throw std::logic_error("DELETE method not implemented");
    }
    
    /// @brief The HTTP methods supported by this controller.
    /// This field indicates which HTTP methods (GET, POST, PUT, DELETE) are supported by the controller.
    const HttpMethod methods;
};

/// @brief This type definition is a function pointer that returns an instance of HttpController.
/// It is used to create instances of controllers dynamically.
typedef std::shared_ptr<HttpController> (*GetControllerInstance)();

#endif // HttpController_h