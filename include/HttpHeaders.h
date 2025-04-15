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
    JPEG,
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

    typedef struct _header
    {
        _header() : _header("", "") {}
        _header(String name) : _header(name, "") {}
        _header(const char *name) : _header(String(name), "") {}
        _header(CONTENT_TYPE contentType) : _header("Content-Type", contentTypeValues.at(contentType)) {}
        _header(String name, String value) : name(name), value(value) {}
        String name;
        String value;
    } Header;

    void sendHeaderSection(int code, bool includeDefaultHeaders = true, Header headers[] = NULL, size_t nHeaders = 0, int length = 0);
    void sendStreamHeaderSection();
    void sendHeader(const String &name, const String &value);
    void sendHeader(const Header &header);
    bool parseRequestHeaderSection(HTTP_REQ_TYPE &requestType, String &resource, Header collectedHeaders[] = NULL, size_t nCollectedHeaders = 0);
    void setReceiveTimeout(unsigned long timeout) { receiveTimeout = timeout; }
    const String &getLastParsedLine() { return parsedLine; }
    const String &getRequestLine() { return requestLine; }

private:
    String parsedLine;
    String requestLine;
    EthClient client;
    unsigned long receiveTimeout;
    static const std::map<int, String> codeDescriptions;
    static const std::map<CONTENT_TYPE, String> contentTypeValues;
    typedef std::map<String, HTTP_REQ_TYPE> HttpReqTypesMap;
    static const HttpReqTypesMap httpReqTypesMap;
};

#endif // HTTP_HEADERS_H