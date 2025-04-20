#include <Common.h>
#include <FilesController.h>
#include <EthernetUtil.h>
#include <SDUtil.h>
#include <TimeUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

static HttpHeaders::Header commonHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };

void FilesController::normalizePath(String &path)
{
    path.replace("%20", " ");
}

bool FilesController::Get(HttpClientContext &context, const String id)
{
    AutoSD autoSD;
    EthClient client = context.getClient();
    String resource = context.getResource();

    #ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Get %s\n", resource.c_str());
#endif
    String path = id;
    normalizePath(path);
    SdFile file = SD.open("/" + path, FILE_READ);

    if (!file)
        return false;

    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders), file.size());

    byte buff[1024];
    size_t fileSize = file.size();
    size_t nBytes = 0;

    while (nBytes < fileSize)
    {
        size_t len = file.read(buff, min<size_t>(sizeof(buff), fileSize - nBytes));
        nBytes += len;
        client.write(buff, len);
#ifdef USE_WIFI
        client.flush();
#endif
    }

#ifndef USE_WIFI
    client.flush();
#endif
    file.close();

    return true;
}

void FilesController::parseUploadHeaders(const String &header, String &boundary, String &fileName)
{
    if (header.startsWith("---"))
        boundary = header;
    else if (header.startsWith("Content-Disposition: "))
    {
        char fileNameId[] = "; filename=";
        fileName = header.substring(header.indexOf(fileNameId) + NELEMS(fileNameId));
        fileName = fileName.substring(0, fileName.indexOf("\""));
#ifdef DEBUG_HTTP_SERVER
        Tracef("File Name=%s\n", fileName.c_str());
#endif
    }
}

bool FilesController::Post(HttpClientContext &context, const String id)
{
    AutoSD autoSD;
    EthClient client = context.getClient();
    String resource = context.getResource();

    normalizePath(resource);
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Post resource=%s, contentLength=%lu, contentType=%s\n", resource.c_str(), context.getContentLength(), context.getContentType().c_str());
#endif
    String content;
    bool endOfHeaders = false;
    String boundary;
    String fileName;
    size_t nBytes = 0;
    while (!endOfHeaders && client.available())
    {
        char c = client.read();
        nBytes++;
        switch (c)
        {
            case '\r':
                break;
            case '\n':
                if (content.equals(""))
                    endOfHeaders = true;
                else
                {
                    parseUploadHeaders(content, boundary, fileName);
                    content = "";
                }
                break;
            default:
                content += c;
                break;
        }
    }

    String filePath = id;
    normalizePath(filePath);
    if (!filePath.equals(""))
        filePath = "/" + filePath;
    SdFile file = SD.open(filePath + "/" + fileName, FILE_WRITE);

    if (!file)
        return false;

    bool failed = false;
    size_t boundaryLen = boundary.length() + 6;
    size_t restOfContent = context.getContentLength() - nBytes;
    byte buff[1024];
    size_t buffSize = sizeof(buff);
    for(;restOfContent % buffSize <= boundaryLen; buffSize--);
#ifdef DEBUG_HTTP_SERVER
    Tracef("File name: %s, File size: %lu, Boundary: %s %lu, Buff size: %lu, Reminder: %lu\n", fileName.c_str(), restOfContent - boundaryLen, boundary.c_str(), boundaryLen, buffSize, restOfContent % buffSize);
#endif
    nBytes = 0;
    while (!failed && nBytes < restOfContent)
    {
        size_t expected = min<size_t>(buffSize, restOfContent - nBytes);
        size_t len = 0;
        time_t t0 = t_now;
        
        while(len < expected && t_now - t0 < 3)
        {
            size_t readRes = client.read(buff + len, buffSize - len);
            if (readRes != 0xffffffff)
                len += readRes;
        }

        if (len != expected)
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("File was not entirely received. Expected: %lu, received: %lu\n", restOfContent, nBytes);
#endif
            failed = true;
            break;
        }

        nBytes += len;
        if (nBytes >= restOfContent)
            len -= boundaryLen;
        size_t written = file.write(buff, len);
        if (written != len)
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("Written %lu bytes, expected: %lu\n", written, len);
#endif
            failed = true;
        }
        file.flush();
    }

    file.close();

    if (failed)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("File upload failed!");
#endif
        SD.remove(resource + "/" + fileName);
        return false;
    }

    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

bool FilesController::Put(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Put %s\n", context.getResource().c_str());
#endif

    String dirPath = id;
    normalizePath(dirPath);
    dirPath = "/" + dirPath;

    if (SD.exists(dirPath) || !SD.mkdir(dirPath))
        return false;

    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

bool FilesController::Delete(HttpClientContext &context, const String id)
{
    AutoSD autoSD;
    String resource = context.getResource();

#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Delete %s\n", resource.c_str());
#endif

    String path = id;
    normalizePath(path);
    path = String("/") + path;

    if (!SD.exists(path))
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Path does not exist %s\n", path.c_str());
#endif
        return false;
    }

    SdFile file =  SD.open(path);
    if (!file)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Failed to open %s\n", path.c_str());
#endif
        return false;
    }

    bool isDir = file.isDirectory();
    file.close();

#ifdef DEBUG_HTTP_SERVER
    Tracef("Resource is: %s\n", isDir ? "directory" : "file");
#endif

    bool success = isDir ? SD.rmdir(path) : SD.remove(path);

    if (!success)
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Failed to delete %s\n", path.c_str());
#endif
        return false;
    }

    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

HttpController *FilesController::getInstance() { return &filesController; }

FilesController filesController;
