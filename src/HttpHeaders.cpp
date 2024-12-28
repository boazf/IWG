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
        for (size_t i = 0; i < nHeaders; i++)
            sendHeader(headers[i]);
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

bool HttpHeaders::parseRequestHeaderSection(HTTP_REQ_TYPE &requestType, String &resource, Header collectedHeaders[], size_t nCollectedHeaders)
{
    requestType = HTTP_REQ_TYPE::HTTP_UNKNOWN;
    parsedLine = "";
    requestLine = "";

    typedef std::map<String, int> HeaderNameIndexes;
    HeaderNameIndexes headerNamesIndexes;
    if (collectedHeaders != NULL)
        for (size_t i = 0; i < nCollectedHeaders; i++)
            headerNamesIndexes.insert(std::pair<String, int>(collectedHeaders[i].name, i));
    
    unsigned long t0 = millis();

    do
    {
        if (!client.available()) 
        {
            if (millis() - t0 >= receiveTimeout)
                return false;

            delay(10);
            continue;
        }

        char c = client.read();

        if (c == '\r')
            continue;

        if (c != '\n')
        {
            parsedLine += c;
            continue;
        }

        if (parsedLine.isEmpty())
            return true;

        if (requestType == HTTP_REQ_TYPE::HTTP_UNKNOWN)
        {
            requestLine = parsedLine;
            int space = parsedLine.indexOf(' ');
            if (space == -1)
                return false;
            
            HttpReqTypesMap::const_iterator httpReqType = httpReqTypesMap.find(parsedLine.substring(0, space));
            if (httpReqType == httpReqTypesMap.end())
                return false;
            
            requestType = httpReqType->second;
            int secondSpace = parsedLine.indexOf(' ', space + 1);
            resource = parsedLine.substring(space + 1, secondSpace);
        }
        else
        {
            int delimiter = parsedLine.indexOf(": ");
            if (delimiter == -1)
            {
                return false;
            }

            String headerName = parsedLine.substring(0, delimiter);
            HeaderNameIndexes::const_iterator headerIndex = headerNamesIndexes.find(headerName);
            if (headerIndex != headerNamesIndexes.end())
                collectedHeaders[headerIndex->second].value = parsedLine.substring(delimiter + 2);
        }
        parsedLine = "";
    } while(true);
}

const std::map<int, String> HttpHeaders::codeDescriptions = 
{ 
    {200, "OK"}, 
    {302, "Found"}, 
    {304, "Not modified"}, 
    {400, "Bad Request"},
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

const HttpHeaders::HttpReqTypesMap HttpHeaders::httpReqTypesMap = 
{
    {"GET", HTTP_REQ_TYPE::HTTP_GET}, 
    {"POST", HTTP_REQ_TYPE::HTTP_POST}, 
    {"PUT", HTTP_REQ_TYPE::HTTP_PUT},
    {"DELETE", HTTP_REQ_TYPE::HTTP_DELETE}
};

