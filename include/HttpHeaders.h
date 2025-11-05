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

#ifndef HTTP_HEADERS_H
#define HTTP_HEADERS_H
#include <Arduino.h>
#include <Common.h>
#include <EthernetUtil.h>
#include <map>

enum class CONTENT_TYPE
{
    UNKNOWN,
    JAVASCRIPT,
    ICON,
    HTML,
    CSS,
    PLAIN,
    JPEG,
    PNG,
    EOT,
    SVG,
    TTF,
    WOFF,
    WOFF2,
    JSON,
    STREAM
};

enum class HTTP_REQ_TYPE
{
    HTTP_UNKNOWN,
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE
};

#define DEFAULT_RECEIVE_TIMEOUT 3000

class HttpHeaders
{
public:
    HttpHeaders(EthClient &client) : 
        client(client),
        receiveTimeout(DEFAULT_RECEIVE_TIMEOUT)
    {        
    }

    /// @brief HTTP header structure
    typedef struct _header
    {
        _header() : _header("", "") {} // Default constructor, creates an empty header
        _header(String name) : _header(name, "") {} // Constructor with name only
        _header(const char *name) : _header(String(name), "") {} // Constructor with C-style string
        _header(CONTENT_TYPE contentType) : _header("Content-Type", contentTypeValues.at(contentType)) {} // Constructor with content type
        _header(String name, String value) : name(name), value(value) {} // Constructor with name and value
        String name; // Header name
        String value; // Header value
    } Header;

    /// @brief Sends an HTTP header section
    /// @param code The HTTP status code
    /// @param includeDefaultHeaders Whether to include default headers
    /// @param headers Custom headers to include
    /// @param nHeaders Number of custom headers
    /// @param length The length to be set in the content length header. If includeDefaultHeaders is false, this value is ignored.
    void sendHeaderSection(int code, bool includeDefaultHeaders = true, Header headers[] = NULL, size_t nHeaders = 0, int length = 0);
    /// @brief Sends an HTTP stream header section
    /// @note This function is used to send the headers for a streaming response such as SSE.
    void sendStreamHeaderSection();
    /// @brief Sends a single HTTP header
    /// @param name The name of the header
    /// @param value The value of the header
    void sendHeader(const String &name, const String &value);
    /// @brief Sends a single HTTP header
    /// @param header The header to send
    void sendHeader(const Header &header);
    /// @brief Parse the HTTP request header section
    /// @param requestType The request type
    /// @param resource The requested resource
    /// @param collectedHeaders The headers collected from the request
    /// @param nCollectedHeaders The number of collected headers
    /// @return True if the header section was successfully parsed, false otherwise
    bool parseRequestHeaderSection(HTTP_REQ_TYPE &requestType, String &resource, Header collectedHeaders[] = NULL, size_t nCollectedHeaders = 0);
    /// @brief Sets the receive timeout for the HTTP client
    /// @param timeout The timeout value in milliseconds
    void setReceiveTimeout(unsigned long timeout) { receiveTimeout = timeout; }
    /// @brief Gets the last parsed line
    /// @return The last parsed line
    /// @note This function is used only for logging erroneous headers in debug builds
    const String &getLastParsedLine() { return parsedLine; }
    /// @brief Gets the request line
    /// @return The request line
    /// @note This function is used only for logging erroneous headers in debug builds
    const String &getRequestLine() { return requestLine; }

private:
    String parsedLine; // The last parsed line
    String requestLine; // The request line
    EthClient client; // The Ethernet client
    unsigned long receiveTimeout; // The receive timeout
    static const std::map<int, String> codeDescriptions; // The HTTP status code descriptions
    static const std::map<CONTENT_TYPE, String> contentTypeValues; // The content type values
    typedef std::map<String, HTTP_REQ_TYPE> HttpReqTypesMap; // The HTTP request types map
    static const HttpReqTypesMap httpReqTypesMap;
};

#endif // HTTP_HEADERS_H