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

#ifndef View_h
#define View_h

#include <HttpController.h>
#include <ViewReader.h>

/// @brief This class implements a view controller that handles HTTP requests for a specific view.
/// It uses a ViewReader to read the view data and respond to HTTP requests.
/// A view is typically a static resource like an HTML file, CSS file, or image.
/// The View class is responsible for handling GET, POST, PUT, and DELETE requests for the view.
class View : public HttpController
{
public:
    /// @brief Constructor for the View class.
    /// @param viewReader A pointer to a ViewReader instance that will be used to read the view data.
    /// The ViewReader is responsible for reading the view data from a specific source, such as a file or a memory buffer.
    /// The View class takes ownership of the ViewReader and will delete it in its destructor.
    View(ViewReader *viewReader) : 
        viewReader(viewReader)
    {
    }

    virtual ~View()
    {
        delete viewReader;
    }

    /// @brief This method is called to handle a GET request.
    /// @param context The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being requested.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource.
    virtual bool Get(HttpClientContext &context, const String id);
    /// @brief This method is called to handle a POST request.
    /// @param context The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being created or modified.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource.
    virtual bool Post(HttpClientContext &context, const String id);
    /// @brief This method is called to handle a PUT request.
    /// @param context The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being updated.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource.
    virtual bool Put(HttpClientContext &context, const String id);
    /// @brief This method is called to handle a DELETE request.
    /// @param context The context of the HTTP client that made the request.
    /// @param id An optional identifier for the resource being deleted.
    /// @return True if the request was handled successfully, false otherwise.
    /// @note The id parameter is optional and can be used to specify a resource identifier if applicable.
    /// If the id is not provided, the controller should handle the request for the root resource.
    virtual bool Delete(HttpClientContext &context, const String id);

protected:
    /// @brief This method is called to redirect the client to a different resource, if necessary.
    /// @param client The EthClient instance representing the client connection.
    /// @param id An optional identifier for the resource being redirected to.
    /// @return True if the redirection is required, false otherwise. 
    /// @note If redirection is required, the method should send a redirect response to the client.
    virtual bool redirect(EthClient &client, const String &id) { return false; }

protected:
    ViewReader *viewReader;
};

#endif // View_h