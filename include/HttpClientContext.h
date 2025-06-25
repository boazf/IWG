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
#include <EthernetUtil.h>
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

class HttpClientContext
{
public:
    HttpClientContext(EthClient &client);

    bool parseRequestHeaderSection();
    HTTP_REQ_TYPE getRequestType() const { return requestType; }
    EthClient &getClient() { return client; }
    uint16_t getRemotePort() const { return remotePort; }
    String getLastModified() const { return GET_HEADER_BY_NAME(IF_MODIFIED_SINCE_HEADER_NAME)->value; }
    size_t getContentLength() const { return atoi(GET_HEADER_BY_NAME(CONTENT_LENGTH_HEADER_NAME)->value.c_str()); }
    String getContentType() const { return GET_HEADER_BY_NAME(CONTENT_TYPE_HEADER_NAME)->value; }
    String getResource() const { return resource; }
    bool keepAlive;

private:
#define N_COLLECTED_HEADERS 3
    CollectedHeaders collectedHeaders;
    HTTP_REQ_TYPE requestType;
    EthClient client;
    uint16_t remotePort;
    String resource;
};

#endif // HttpClientContext_h