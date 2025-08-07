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

    // Check if redirect is needed.
    // If the redirect is successful, it will send the redirect response and return true.
    // Otherwise, it will return false, and we will proceed to read the view.
    if (redirect(client, id))
        return true;

    // Open the view and pass it a buffer to read the view data.
    byte buff[256];
    if (!viewReader->open(buff, sizeof(buff)))
    {
        return false;
    }

    // Check if the view was modified since the last request.
    if (!context.getLastModified().isEmpty())
    {
        String lastModifiedTime;

        viewReader->getLastModifiedTime(lastModifiedTime);
        if (context.getLastModified().equals(lastModifiedTime))
        {
            // If the last modified time matches, we can return a 304 Not Modified response.
#ifdef DEBUG_HTTP_SERVER
            TRACE_BLOCK
	        {
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

    // If we reach here, it means we need to send the view data.
    CONTENT_TYPE type = viewReader->getContentType();
    if (type == CONTENT_TYPE::UNKNOWN)
    {
        // If the content type is unknown, we must fail the request.
#ifdef DEBUG_HTTP_SERVER
        Traceln("Unknown extention");
#endif
        viewReader->close();
        return false;
    }

    // Prepare the headers to send.
    long size = viewReader->getViewSize();

    // Prepare two additional headers, one for the content type and one for the last modified time.
    // If the content type is HTML, we will not send the last modified time header.
    // This is because HTML files are often dynamically generated and may not have a last modified time.
    // For other content types, we will send the last modified time header if it is available.
    HttpHeaders::Header additionalHeaders[] = { {type}, {} };
    if (type != CONTENT_TYPE::HTML)
    {
        String lastModifiedTime;
        if (viewReader->getLastModifiedTime(lastModifiedTime))
        {
            additionalHeaders[1] = {"Last-Modified", lastModifiedTime};
        }
    }

    // Send the headers to the client.
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), size);

    // Pump the response body to the client.
    long bytesSent = 0;
    while (bytesSent < size)
    {
        int nBytes = viewReader->read();
        client.write(buff, nBytes);
        bytesSent += nBytes;
    }

#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Tracef("%d ", context.getClient().remotePort());
        Trace("Done sending, Sent ");
        Trace(bytesSent);
        Traceln(" bytes");
    }
#endif

    // Close the view reader to release resources.
    viewReader->close();

    return true;
}

// The Post, Put, and Delete methods are not implemented in the View class.
// They are left as stubs to indicate that these methods are not supported for views.
// If you need to implement these methods, you can override them in a derived class.
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
