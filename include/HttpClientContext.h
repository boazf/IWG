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

#ifndef HttpClientContext_h
#define HttpClientContext_h

#include <Arduino.h>
#include <HttpHeaders.h>
#include <array>

#define N_COLLECTED_HEADERS 3
#define IF_MODIFIED_SINCE_HEADER_NAME "If-Modified-Since"
#define CONTENT_LENGTH_HEADER_NAME "Content-Length"
#define CONTENT_TYPE_HEADER_NAME "Content-Type"

typedef std::array<HttpHeaders::Header, N_COLLECTED_HEADERS> CollectedHeaders;
#define GET_HEADER_BY_NAME(headerName) \
    std::find_if( \
        std::begin(collectedHeaders), \
        std::end(collectedHeaders), \
        [](const HttpHeaders::Header &header) {return header.name.equals(headerName); })

/// @brief This class defines the context for an HTTP client request.
/// It holds the client connection, request type, headers, and other relevant information.
class HttpClientContext
{
public:
    /// @brief Constructor for the HttpClientContext class.
    /// @param client The EthClient instance representing the client connection.
    /// This constructor initializes the context with the client connection and prepares to parse the request headers.
    HttpClientContext(EthClient &client);

    /// @brief Parses the request header section from the client connection.
    /// This method reads the HTTP request headers from the client connection and determines the request type (GET, POST, etc.).
    /// It also extracts the requested resource and collects relevant headers.
    /// @return Returns true if the request header section was successfully parsed, false otherwise.
    /// If the request is malformed or an error occurs, it returns false.
    bool parseRequestHeaderSection();
    /// @brief Returns the request type of the HTTP request.
    /// This method retrieves the type of the HTTP request (GET, POST, PUT, DELETE) that was parsed from the request header section.
    /// @note The request type is determined during the parsing of the request header section.
    /// @return Returns the HTTP_REQ_TYPE enum value representing the request type.
    /// Possible values are HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, or HTTP_UNKNOWN if the request type could not be determined.
    /// @note If the request header section has not been parsed yet, this method will return HTTP_UNKNOWN.
    HTTP_REQ_TYPE getRequestType() const { return requestType; }
    /// @brief Returns the EthClient instance representing the client connection.
    /// This method provides access to the EthClient instance that is associated with this HTTP client context.
    /// It allows the caller to interact with the client connection, such as reading data, writing responses, or checking connection status.
    /// @return Returns a reference to the EthClient instance associated with this context.
    EthClient &getClient() { return client; }
    /// @brief Returns the remote port of the client connection.
    /// @return Returns the remote port as a uint16_t value.
    uint16_t getRemotePort() const { return remotePort; }
    /// @brief Returns the value of the "If-Modified-Since" header.
    /// @return Returns the value of the "If-Modified-Since" header as a String.
    /// This header is typically used to determine if the requested resource has been modified since the specified date and time.
    /// If the header is not present, it will return an empty String.
    String getLastModified() const { return GET_HEADER_BY_NAME(IF_MODIFIED_SINCE_HEADER_NAME)->value; }
    /// @brief Returns the value of the "Content-Length" header.
    /// @return Returns the value of the "Content-Length" header as a size_t.
    size_t getContentLength() const { return atoi(GET_HEADER_BY_NAME(CONTENT_LENGTH_HEADER_NAME)->value.c_str()); }
    /// @brief Returns the value of the "Content-Type" header.
    /// @return Returns the value of the "Content-Type" header as a String.
    String getContentType() const { return GET_HEADER_BY_NAME(CONTENT_TYPE_HEADER_NAME)->value; }
    /// @brief Returns the requested resource from the HTTP request.
    /// @return Returns the requested resource as a String.
    /// The resource is typically the path or URL that the client is trying to access.
    String getResource() const { return resource; }
    /// @brief Indicates whether the connection should be kept alive after the request is processed.
    /// @return Returns true if the connection should be kept alive, false otherwise.
    bool keepAlive;

private:
    /// @brief The headers collected from the HTTP request.
    /// This array holds the headers that are relevant for processing the HTTP request.
    CollectedHeaders collectedHeaders;
    /// @brief The type of the HTTP request (GET, POST, etc.).
    HTTP_REQ_TYPE requestType;
    /// @brief The EthClient instance representing the client connection.
    EthClient client;
    /// @brief The remote port of the client connection.
    uint16_t remotePort;
    /// @brief The requested resource from the HTTP request.
    /// This string holds the path or URL that the client is trying to access.
    String resource;
};

#endif // HttpClientContext_h