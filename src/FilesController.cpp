#ifdef ESP32
#include <Common.h>
#include <FilesController.h>
#include <EthernetUtil.h>
#include <SDUtil.h>
#include <TimeUtil.h>

void FilesController::normilizePath(String &path)
{
    path.replace("%20", " ");
}

bool FilesController::Get(EthernetClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Get %s\n", resource.c_str());
#endif
    normilizePath(resource);
    SdFile file = SD.open("/" + resource, FILE_READ);

    if (!file)
        return false;

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println(String("Content-Length: ") + file.size());
    client.println();

    byte buff[1024];

    size_t len = file.read(buff, sizeof(buff));
    while (len)
    {
        client.write(buff, len);
        client.flush();
        len = file.read(buff, sizeof(buff));
    }

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
    }
}

bool FilesController::Post(EthernetClient &client, String &resource, size_t contentLength, String contentType)
{
    normilizePath(resource);
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Post resource=%s, contentLength=%ul, contentType=%s\n", resource.c_str(), contentLength, contentType.c_str());
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

    if (!resource.equals(""))
        resource = "/" + resource;
    SdFile file = SD.open(resource + "/" + fileName, FILE_WRITE);

    if (!file)
        return false;

    bool failed = false;
    size_t boundaryLen = boundary.length() + 6;
    size_t restOfContent = contentLength - nBytes;
    byte buff[1024];
    size_t buffSize = sizeof(buff);
    for(;restOfContent % buffSize <= boundaryLen; buffSize--);
#ifdef DEBUG_HTTP_SERVER
    Tracef("File name: %s, File size: %lu, Boundary: %s %lu, Buff size: %lu, Reminder: %lu\n", fileName.c_str(), restOfContent - boundaryLen, boundary.c_str(), boundaryLen, buffSize, restOfContent % buffSize);
#endif
    nBytes = 0;
    while (!failed && nBytes < restOfContent)
    {
        time_t t0 = t_now;
        while (!client.available() && t_now - t0 <= 3);
        if (!client.available())
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("File was not entirely received. Expected: %lu, received: %lu\n", restOfContent, nBytes);
#endif
            failed = true;
            break;
        }

        size_t len = client.read(buff, buffSize);

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

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println("Content-Length: 0");
    client.println();
    client.flush();

    return true;
}

bool FilesController::Put(EthernetClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Put %s\n", resource.c_str());
#endif

    normilizePath(resource);
    String dirPath = "/" + resource;

    if (SD.exists(dirPath) || !SD.mkdir(dirPath))
        return false;

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println("Content-Length: 0");
    client.println();
    client.flush();

    return true;
}

bool FilesController::Delete(EthernetClient &client, String &resource)
{
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Delete %s\n", resource.c_str());
#endif

    normilizePath(resource);
    String path = "/" + resource;

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

    client.println("HTTP/1.1 200 OK");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
    client.println("Content-Length: 0");
    client.println();
    client.flush();

    return true;
}

FilesController filesController;
#endif // ESP32