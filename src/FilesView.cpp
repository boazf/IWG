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
#include <FilesView.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

FilesView::FilesView(const char *_viewFile) : 
   FileView(_viewFile)
{
}

/// @brief Handle POST requests for the FilesView.
/// @param context The HTTP client context.
/// @param id The ID of the directory to list.
/// @return True if the request was successful, false otherwise.
/// This method processes the POST request to list files in a directory.
/// It reads the specified directory from the SD card and returns a JSON response with file details.
bool FilesView::Post(HttpClientContext &context, const String id)
{
    // Initialize the SD card and get the client and resource from the context.
    // The id parameter is used to specify the directory to list.
    AutoSD autoSD;
    String normalizedPath = "/" + id;
    normalizedPath.replace("%20", " ");
#ifdef DEBUG_HTTP_SERVER
    const String resource = context.getResource();
    Tracef("FilesView: POST: resource=%s, id=%s, path=%s\n", resource.c_str(), id.c_str(), normalizedPath.c_str());
#endif
    // Open the specified directory on the SD card.
    SdFile dir = SD.open(normalizedPath, FILE_READ);
    String resp;
    // If the directory could not be opened or the path represents a file, return an empty JSON array.
    if (!dir || !dir.isDirectory())
    {
        // Result is set to 1 to indicate that the request could not be processed.
        resp = "[{ \"result\": 1 }]";
    }
    else
    {
        // If the directory was opened successfully, set result to 0 and read its contents and prepare a JSON response.
        resp = "[ { \"result\": 0 }, { \"files\": [";
        // Iterate through the files and directories in the directory and add their details to the response.
        SdFile file = dir.openNextFile(FILE_READ);
        bool first = true;
        while (file)
        {
            // Collect information about the file/directory such as name, size, and last write time.
            // The last write time is formatted as a string in the format "dd/mm/yyyy hh:mm".
            // The file size is included in the response.
            time_t fileTime = file.getLastWrite();
            tm tmFile;
            char buff[64];
            localtime_r(&fileTime, &tmFile);
            strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M", &tmFile);
            // Append the file information to the response string.
            // The response is formatted as a JSON object with fields for time, name, isDir, and size.
            // The isDir field indicates whether the entry is a directory or a file.
            resp += String(first ? "" : ",\n") + "{ \"time\": \"" + buff + "\", \"name\": \"" + file.path() + "\", \"isDir\": " + (file.isDirectory() ? "true" : "false") + ", \"size\": " + file.size() + " }";
            // Close the current file and open the next one.
            file.close();
            file = dir.openNextFile(FILE_READ);
            first = false;
        }
        // Close the JSON array and the file list.
        resp += "] } ]";
    }
#ifdef DEBUG_HTTP_SERVER
    Traceln(resp);
#endif
    // Send the response back to the client.
    // The response is sent as a JSON object with the file details.
    // The content type is set to application/json.
    // The response length is calculated and included in the headers.
    unsigned int len = resp.length();
    HttpHeaders::Header additionalHeaders[] = {{CONTENT_TYPE::JSON}};
    EthClient client = context.getClient();
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders), len);

    // Send the response in slices because of a limitation of W5500. In case of WiFi this doesn't matter.
    unsigned int index = 0;
    #define BUFF_SIZE 1024U

    for (unsigned int index = 0; index < len; index += BUFF_SIZE)
        client.print(resp.substring(index, min<unsigned int>(index + BUFF_SIZE, len)));

    return true;
}
