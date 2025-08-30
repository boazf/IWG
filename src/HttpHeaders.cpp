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

#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

void HttpHeaders::sendHeaderSection(int code, bool includeDefaultHeaders, Header headers[], size_t nHeaders, int length)
{
    // Send the HTTP status line
    client.printf("HTTP/1.1 %d %s\n", code, codeDescriptions.at(code).c_str());
    // Send the default headers
    if (includeDefaultHeaders)
    {
        sendHeader("Connection", "close");
        sendHeader("Server", "Arduino");
        sendHeader("Content-Length", String(length));
    }
    // Send custom headers
    if (headers)
        for (size_t i = 0; i < nHeaders; i++)
            sendHeader(headers[i]);
    // End of headers
    client.println();
    #ifdef USE_WIFI
        client.flush();
    #endif
}

void HttpHeaders::sendStreamHeaderSection()
{
    HttpHeaders::Header headers[] = { {"Access-Control-Allow-Origin", "*" }, {CONTENT_TYPE::STREAM}, {"Connection", "keep-alive"}, {"Cache-Control", "no-cache"}};
    HttpHeaders httpHeaders(client);
    httpHeaders.sendHeaderSection(200, false, headers, NELEMS(headers));
}

void HttpHeaders::sendHeader(const String &name, const String &value)
{
    // Do not send empty header names
    if (name.isEmpty())
        return;

    // Send the header
    client.printf("%s: %s\n", name.c_str(), value.c_str());
}

void HttpHeaders::sendHeader(const Header &header)
{
    sendHeader(header.name, header.value);
}

bool HttpHeaders::parseRequestHeaderSection(HTTP_REQ_TYPE &requestType, String &resource, Header collectedHeaders[], size_t nCollectedHeaders)
{
    requestType = HTTP_REQ_TYPE::HTTP_UNKNOWN;
    parsedLine = "";
    requestLine = "";

    // Create a map of header names to their indexes in the collectedHeaders array
    typedef std::map<String, int> HeaderNameIndexes;
    HeaderNameIndexes headerNamesIndexes;
    if (collectedHeaders != NULL)
        for (size_t i = 0; i < nCollectedHeaders; i++)
            headerNamesIndexes.insert(std::pair<String, int>(collectedHeaders[i].name, i));
    
    unsigned long t0 = millis();

    do
    {
        if (!client)
        {
            // Client got disconnected
#ifdef DEBUG_HTTP_SERVER
            Tracef("%d Received incomplete request!\n", client.remotePort());
#endif
            return false;
        }
        if (!client.available()) 
        {
            // Wait for data to become available, break if timeout
            if (millis() - t0 >= receiveTimeout)
            {
#ifdef DEBUG_HTTP_SERVER
                Tracef("%d Receive timeout!\n", client.remotePort());
#endif
                return false;
            }
            delay(10);
            continue;
        }

        // Read the next character
        char c = client.read();

        // Ignore carriage return characters
        if (c == '\r')
            continue;

        if (c != '\n')
        {
            // Append the character to the parsed line
            parsedLine += c;
            continue;
        }

        // We have here the entire line from the request
        if (parsedLine.isEmpty())
            return true;

        if (requestType == HTTP_REQ_TYPE::HTTP_UNKNOWN)
        {
            // The request type is found in the request status line.
            // If requestType is unknown, it means we are dealing with the request status line.
            requestLine = parsedLine;
            // Extract the request method
            int space = parsedLine.indexOf(' ');
            if (space == -1)
                return false;

            // Find the HTTP request type
            HttpReqTypesMap::const_iterator httpReqType = httpReqTypesMap.find(parsedLine.substring(0, space));
            if (httpReqType == httpReqTypesMap.end())
                // Unknown HTTP request type
                return false;

            // Store the request type
            requestType = httpReqType->second;
            // Extract the resource
            int secondSpace = parsedLine.indexOf(' ', space + 1);
            if (secondSpace == -1)
                // Resource not found
                return false;
            resource = parsedLine.substring(space + 1, secondSpace);
        }
        else
        {
            // Parse headers
            int delimiter = parsedLine.indexOf(": ");
            if (delimiter == -1)
                // Invalid header format
                return false;

            // Extract the header name
            String headerName = parsedLine.substring(0, delimiter);
            // If the header is in the collectedHeaders array, store its value
            HeaderNameIndexes::const_iterator headerIndex = headerNamesIndexes.find(headerName);
            if (headerIndex != headerNamesIndexes.end())
                collectedHeaders[headerIndex->second].value = parsedLine.substring(delimiter + 2);
        }
        // Clear the parsed line for the next iteration
        parsedLine = "";
    } while(true);
}

/// @brief HTTP status code descriptions
const std::map<int, String> HttpHeaders::codeDescriptions = 
{ 
    {200, "OK"}, 
    {302, "Found"}, 
    {304, "Not modified"}, 
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

/// @brief Content type header value
const std::map<CONTENT_TYPE, String> HttpHeaders::contentTypeValues =
{
    {CONTENT_TYPE::JAVASCRIPT, "application/javascript"},
    {CONTENT_TYPE::ICON, "image/x-icon"},
    {CONTENT_TYPE::HTML, "text/html"},
    {CONTENT_TYPE::CSS, "text/css"},
    {CONTENT_TYPE::JPEG, "image/x-jpeg"},
    {CONTENT_TYPE::EOT, "application/vnd.ms-fontobject"},
    {CONTENT_TYPE::SVG, "image/svg+xml"},
    {CONTENT_TYPE::TTF, "font/ttf"},
    {CONTENT_TYPE::WOFF, "font/woff"},
    {CONTENT_TYPE::WOFF2, "font/woff2"},
    {CONTENT_TYPE::JSON, "application/json"},
    {CONTENT_TYPE::STREAM, "text/event-stream"}
};

/// @brief HTTP request types
const HttpHeaders::HttpReqTypesMap HttpHeaders::httpReqTypesMap = 
{
    {"GET", HTTP_REQ_TYPE::HTTP_GET}, 
    {"POST", HTTP_REQ_TYPE::HTTP_POST}, 
    {"PUT", HTTP_REQ_TYPE::HTTP_PUT},
    {"DELETE", HTTP_REQ_TYPE::HTTP_DELETE}
};

