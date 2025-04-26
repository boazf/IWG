#include <Common.h>
#include <View.h>
#include <HttpHeaders.h>
#include <HttpServer.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool View::Get(HttpClientContext &context, const String id)
{
    EthClient client = context.getClient();

    if (redirect(client, id))
        return true;

    byte buff[256];
    if (!viewReader->open(buff, sizeof(buff)))
    {
        return false;
    }

    if (!context.getLastModified().isEmpty())
    {
        String lastModifiedTime;

        viewReader->getLastModifiedTime(lastModifiedTime);
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
            viewReader->close();
            return true;
        }
    }

    CONTENT_TYPE type = viewReader->getContentType();
    if (type == CONTENT_TYPE::UNKNOWN)
    {
#ifdef DEBUG_HTTP_SERVER
        Traceln("Unknown extention");
#endif
        viewReader->close();
        return false;
    }

    long size = viewReader->getViewSize();

    HttpHeaders::Header additionalHeaders[] = { {type}, {} };
    if (type != CONTENT_TYPE::HTML)
    {
        String lastModifiedTime;
        if (viewReader->getLastModifiedTime(lastModifiedTime))
        {
            additionalHeaders[1] = {"Last-Modified", lastModifiedTime};
        }
    }

    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), size);

    long bytesSent = 0;
    while (bytesSent < size)
    {
        int nBytes = viewReader->read();
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

    viewReader->close();

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
