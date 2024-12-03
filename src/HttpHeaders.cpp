#include <HttpHeaders.h>

void HttpHeaders::sendHeaderSection(int code, bool includeDefaultHeaders, Header headers[], size_t nHeaders, int length)
{
    client.printf("HTTP/1.1 %d %s\n", code, codeDescriptions.at(code).c_str());
    if (includeDefaultHeaders)
    {
        sendHeader("Connection", "close");
        sendHeader("Server", "Arduino");
        sendHeader("Content-Length", String(length));
    }
    if (headers)
    {
        for (size_t i = 0; i < nHeaders; i++)
            sendHeader(headers[i]);
    }
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
    if (name.isEmpty())
        return;

    client.printf("%s: %s\n", name.c_str(), value.c_str());
}

void HttpHeaders::sendHeader(const Header &header)
{
    sendHeader(header.name, header.value);
}

const std::map<int, String> HttpHeaders::codeDescriptions = 
{ 
    {200, "OK"}, 
    {302, "Found"}, 
    {304, "Not modified"}, 
    {404, "Not Found"},
    {500, "Internal Server Error"}
};

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