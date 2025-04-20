#include <AutoPtr.h>
#include <View.h>
#include <SDUtil.h>
#include <time.h>
#include <Common.h>
#include <Config.h>
#include <HttpHeaders.h>
#include <HttpServer.h>
#include <map>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool View::open(byte *buff, int buffSize)
{
    this->buff = buff;
    this->buffSize = buffSize;

    return true;
}

void View::close()
{
}

bool View::redirect(EthClient &client, const String &id)
{
    return false;
}

bool View::Get(HttpClientContext &context, const String id)
{
    EthClient client = context.getClient();

    if (redirect(client, id))
        return true;

    byte buff[256];
    if (!open(buff, sizeof(buff)))
    {
        return false;
    }

    if (!context.getLastModified().isEmpty())
    {
        String lastModifiedTime;

        getLastModifiedTime(lastModifiedTime);
        if (context.getLastModified().equals(lastModifiedTime))
        {
#ifdef DEBUG_HTTP_SERVER
            {
                LOCK_TRACE();
                Tracef("%d ", context.getClient().remotePort());
                Trace("Resource: ");
                Trace(context.getResource());
                Trace(" File was not modified. ");
                Traceln(context.getLastModified());
            }
#endif
            HTTPServer::NotModified(client);
            close();
            return true;
        }
    }

    CONTENT_TYPE type = getContentType();
    if (type == CONTENT_TYPE::UNKNOWN)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Unknown extention");
#endif
        close();
        return false;
    }

    long size = getViewSize();

    HttpHeaders::Header additionalHeaders[] = { {type}, {} };
    if (type != CONTENT_TYPE::HTML)
    {
        String lastModifiedTime;
        if (getLastModifiedTime(lastModifiedTime))
        {
            additionalHeaders[1] = {"Last-Modified", lastModifiedTime};
        }
    }

    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), size);

    long bytesSent = 0;
    while (bytesSent < size)
    {
        int nBytes = read();
        client.write(buff, nBytes);
        bytesSent += nBytes;
    }

#ifdef DEBUG_HTTP_SERVER
    {
        LOCK_TRACE();
        Tracef("%d ", context.getClient().remotePort());
        Trace("Done sending, Sent ");
        Trace(bytesSent);
        Traceln(" bytes");
    }
#endif

    close();

    return true;
}

bool View::Post(HttpClientContext &context, const String id)
{
    return false;
}
bool View::Put(HttpClientContext &context, const String id)
{
    return false;
}
bool View::Delete(HttpClientContext &context, const String id)
{
    return false;
}
