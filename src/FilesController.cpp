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
#include <FilesController.h>
#include <EthernetUtil.h>
#include <SDUtil.h>
#include <TimeUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

/// @brief Common headers used in HTTP responses from the FilesController.
/// These headers are sent with every response to ensure proper caching and CORS support.
static HttpHeaders::Header commonHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };

void FilesController::normalizePath(String &path)
{
    path.replace("%20", " ");
}

/// @brief Handle GET requests for files.
/// @param context The HTTP client context.
/// @param id The ID of the file to retrieve.
/// @return True if the request was successful, false otherwise.
/// This method reads the specified file from the SD card and downloads it to the client.
/// If the file does not exist, it returns false.
bool FilesController::Get(HttpClientContext &context, const String id)
{
    // Initialize the SD card and get the client and resource from the context.
    AutoSD autoSD;
    EthClient client = context.getClient();
    String resource = context.getResource();

#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Get %s\n", resource.c_str());
#endif
    // Normalize the file path to ensure it is properly formatted.
    String path = id;
    normalizePath(path);
    // Open the file on the SD card for reading.
    SdFile file = SD.open("/" + path, FILE_READ);

    // If the file could not be opened, return false.
    if (!file)
        return false;

    // Send the headers section with the file size and common headers.
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders), file.size());

    byte buff[1024];
    size_t fileSize = file.size();
    size_t nBytes = 0;

    // Pump the file content to the client in chunks.
    // This loop reads the file in chunks of 1024 bytes until the entire file is sent.
    // It ensures that the client receives the data in a timely manner.
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
    // This method parses the upload headers from the HTTP request.
    // It extracts the boundary and file name from the header line.
    // The boundary is used to separate different parts of the multipart form data,
    if (header.startsWith("---"))
        boundary = header;
    else if (header.startsWith("Content-Disposition: "))
    {
        // Extract the file name from the header.
        char fileNameId[] = "; filename=";
        fileName = header.substring(header.indexOf(fileNameId) + NELEMS(fileNameId));
        fileName = fileName.substring(0, fileName.indexOf("\""));
#ifdef DEBUG_HTTP_SERVER
        Tracef("File Name=%s\n", fileName.c_str());
#endif
    }
}

/// @brief Handle POST requests for file uploads.
/// @param context The HTTP client context.
/// @param id The ID of the file to upload.
/// @return True if the request was successful, false otherwise.
/// This method uploads a file from the client and saves it to the SD card.
/// It reads the file data from the client, writes it to the SD card, and sends a response back to the client.
/// If the upload fails, it removes the file from the SD card and returns false.
bool FilesController::Post(HttpClientContext &context, const String id)
{
    // Initialize the SD card and get the client and resource from the context.
    AutoSD autoSD;
    EthClient client = context.getClient();
    String resource = context.getResource();

    // Normalize the file path to ensure it is properly formatted.
    normalizePath(resource);
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Post resource=%s, contentLength=%lu, contentType=%s\n", resource.c_str(), context.getContentLength(), context.getContentType().c_str());
#endif

    // We get here after the headers section has been read and interpreted.
    // In case of a file upload, the message body contains at the beginning some other headers that we need to parse.
    // The headers are separated by a blank line from the file data, so we read until we find an empty line.
    // After that, we can read the file data.
    String content;
    bool endOfHeaders = false;
    String boundary;
    String fileName;
    size_t nBytes = 0;
    while (!endOfHeaders && client.available())
    {
        // Read a character from the client.
        char c = client.read();
        nBytes++;
        switch (c)
        {
            case '\r':
                // Ignore carriage return characters.
                break;
            case '\n':
                // If we reach a new line, check if we have reached the end of the headers.
                if (content.equals(""))
                    // If the content is empty, we have reached the end of the headers.
                    endOfHeaders = true;
                else
                {
                    // If we have a header line, parse it.
                    parseUploadHeaders(content, boundary, fileName);
                    // Reset the content for the next header line.
                    content = "";
                }
                break;
            default:
                // Append the character to the content line.
                content += c;
                break;
        }
    }

    // The file path on the local SD card is set in the id parameter.
    String filePath = id;
    normalizePath(filePath);
    // If the file path is not empty, we prepend a slash to it.
    if (!filePath.equals(""))
        filePath = "/" + filePath;
    // Open the file on the SD card for writing.
    SdFile file = SD.open(filePath + "/" + fileName, FILE_WRITE);

    // If the file could not be opened, return false.
    if (!file)
        return false;

    bool failed = false;
    size_t boundaryLen = boundary.length() + 6;
    // The size of the file is the content length minus the number of bytes read so far.
    size_t restOfContent = context.getContentLength() - nBytes;
    byte buff[1024];
    size_t buffSize = sizeof(buff);
    // Find a buffer length that will entirely contain the boundary on the last read. 
    // This is done because the boundary is at the end of the file, and we need to remove it.
    // We don't want to remove it in parts, if it is not entirely contained in the buffer.
    for(;restOfContent % buffSize <= boundaryLen; buffSize--);
#ifdef DEBUG_HTTP_SERVER
    Tracef("File name: %s, File size: %lu, Boundary: %s %lu, Buff size: %lu, Reminder: %lu\n", fileName.c_str(), restOfContent - boundaryLen, boundary.c_str(), boundaryLen, buffSize, restOfContent % buffSize);
#endif
    nBytes = 0;
    // Read the file data from the client in chunks.
    while (!failed && nBytes < restOfContent)
    {
        // Calculate the expected length of the next chunk.
        size_t expected = min<size_t>(buffSize, restOfContent - nBytes);
        size_t len = 0;
        time_t t0 = t_now;
        
        // Read the next chunk from the client.
        // This can be done in several reads, because the data may not be available if the connection is slow.
        while(len < expected && t_now - t0 < 3)
        {
            size_t readRes = client.read(buff + len, buffSize - len);
            if (readRes != 0xffffffff)
                len += readRes;
        }

        // If the length of the read data is less than the expected length, we have a problem.
        if (len != expected)
        {
#ifdef DEBUG_HTTP_SERVER
            Tracef("File was not entirely received. Expected: %lu, received: %lu\n", restOfContent, nBytes);
#endif
            failed = true;
            break;
        }

        // Calculate the number of bytes written so far.
        nBytes += len;
        if (nBytes >= restOfContent)
            // If we have read all the data, we need to remove the boundary from the end of the file.
            len -= boundaryLen;
        // Write the data to the file.
        size_t written = file.write(buff, len);
        if (written != len)
        {
            // If the number of bytes written is not equal to the number of bytes in the buffer, we have a problem.
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
        // If the upload failed, we remove the file from the SD card.
#ifdef DEBUG_HTTP_SERVER
        Traceln("File upload failed!");
#endif
        SD.remove(resource + "/" + fileName);
        return false;
    }

    // If the upload was successful, we send an OK response back to the client.
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

/// @brief Handle PUT requests for creating directories.
/// @param context The HTTP client context.
/// @param id The ID of the directory to create.
/// @return True if the directory was created successfully, false otherwise.
/// This method creates a directory on the SD card with the specified path in the id parameter.
bool FilesController::Put(HttpClientContext &context, const String id)
{
    // Initialize the SD card and get the client and resource from the context.
    AutoSD autoSD;
#ifdef DEBUG_HTTP_SERVER
    Tracef("FilesController Put %s\n", context.getResource().c_str());
#endif

    String dirPath = id;
    normalizePath(dirPath);
    dirPath = "/" + dirPath;

    if (SD.exists(dirPath) || !SD.mkdir(dirPath))
        // If the directory already exists or could not be created, return false.
        return false;

    // If the directory was created successfully, send an OK response back to the client.
    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

/// @brief Handle DELETE requests for deleting files or directories.
/// @param context The HTTP client context.
/// @param id The ID of the file or directory to delete.
/// @return True if the file or directory was deleted successfully, false otherwise.
/// This method deletes a file or directory from the SD card.
bool FilesController::Delete(HttpClientContext &context, const String id)
{
    // Initialize the SD card.
    AutoSD autoSD;
 
#ifdef DEBUG_HTTP_SERVER
    String resource = context.getResource();
   Tracef("FilesController Delete %s\n", resource.c_str());
#endif

    // The file/directory path to be deleted is specified in the id parameter.
    String path = id;
    normalizePath(path);
    path = String("/") + path;

    // Check if the path exists on the SD card.
    if (!SD.exists(path))
    {
#ifdef DEBUG_HTTP_SERVER
        Tracef("Path does not exist %s\n", path.c_str());
#endif
        return false;
    }

    // Open the file or directory to check if it is a directory.
    SdFile file =  SD.open(path);
    if (!file)
    {
        // If the file could not be opened, return false.
#ifdef DEBUG_HTTP_SERVER
        Tracef("Failed to open %s\n", path.c_str());
#endif
        return false;
    }

    // Check if the file is a directory or a regular file.
    bool isDir = file.isDirectory();
    file.close();

#ifdef DEBUG_HTTP_SERVER
    Tracef("Resource is: %s\n", isDir ? "directory" : "file");
#endif

    // Remove  the directory or the file depending on the type.
    bool success = isDir ? SD.rmdir(path) : SD.remove(path);

    if (!success)
    {
        // If the removal failed, return false.
#ifdef DEBUG_HTTP_SERVER
        Tracef("Failed to delete %s\n", path.c_str());
#endif
        return false;
    }

    // If the removal was successful, send an OK response back to the client.
    HttpHeaders headers(context.getClient());
    headers.sendHeaderSection(200, true, commonHeaders, NELEMS(commonHeaders));

    return true;
}

static std::shared_ptr<HttpController> filesController = std::make_shared<FilesController>();

/// @brief Get the singleton instance of the FilesController.
/// @return A pointer to the singleton instance of the FilesController.
/// @note Since this controller has no member variables, it can be safely
///       used as a singleton and handle multiple requests concurrently.
std::shared_ptr<HttpController> FilesController::getInstance() { return filesController; }
