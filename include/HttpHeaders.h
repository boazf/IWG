#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
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

class HttpHeaders
{
public:
    HttpHeaders(EthClient &client) : client(client)
    {        
    }

    typedef struct _header
    {
        _header() : name(""), value("") {}
        _header(String name, String value) : name(name), value(value) {}
        _header(CONTENT_TYPE contentType) : name("Content-Type"), value(contentTypeValues.at(contentType)) {}
        String name;
        String value;
    } Header;

    void sendHeaderSection(int code, bool includeDefaultHeaders = true, Header headers[] = NULL, size_t nHeaders = 0, int length = 0);
    void sendStreamHeaderSection();
    void sendHeader(const String &name, const String &value);
    void sendHeader(const Header &header);

private:
    EthClient client;
    static const std::map<int, String> codeDescriptions;
    static const std::map<CONTENT_TYPE, String> contentTypeValues;
};

#endif // HTTP_CLIENT_H