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