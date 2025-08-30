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

#include <map>
#include <FileViewReader.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool FileViewReader::open(byte *buff, int buffSize)
{
    // Ensure SD card is mounted
    AutoSD autoSD;
    // Open the file on the SD card for reading
    String fileName;
    fileName = "/wwwroot" + viewFilePath;
    SdFile file = SD.open(fileName, FILE_READ);
#ifdef DEBUG_HTTP_SERVER
    if (!file)
	{
        LOCK_TRACE;
        Trace("Failed to open file ");
        Traceln(fileName.c_str());
    }
#endif

    // Open the reader by giving it the file object
    return open(buff, buffSize, file);
}

bool FileViewReader::open(byte *buff, int buffSize, SdFile file)
{
    // Open base class
    if (!ViewReader::open(buff, buffSize))
        return false;
    // Save the file object
    this->file = file;
    // Check the file object if it is a valid open file object
    if (file)
    {
        // Start an SD card session
        SD.begin();
        return true;
    }

    return false;
}

void FileViewReader::close()
{
    if (file)
    {
        // Close the file
        file.close();
        // End the SD card session
        SD.end();
    }
}

int FileViewReader::read(int offset)
{
    return file.read(buff + offset, buffSize - offset);
}

long FileViewReader::getViewSize()
{
    return file.size();
}

bool FileViewReader::getLastModifiedTime(String &lastModifiedTimeStr)
{
    tm tr;
    time_t fileTime = file.getLastWrite();
    gmtime_r(&fileTime, &tr);
    char lastModifiedTime[64];
    // Last-Modified: Sun, 21 Jun 2020 14:33:06 GMT
    strftime(lastModifiedTime, sizeof(lastModifiedTime), "%a, %d %h %Y %H:%M:%S GMT", &tr);
    lastModifiedTimeStr = lastModifiedTime;

    return true;
}

/// @brief Map file extensions to their content types
typedef std::map<String, CONTENT_TYPE> FileTypesMap;
static FileTypesMap fileTypesMap = 
{
    {"JS", CONTENT_TYPE::JAVASCRIPT},
    {"ICO", CONTENT_TYPE::ICON},
    {"HTM", CONTENT_TYPE::HTML},
    {"CSS", CONTENT_TYPE::CSS},
    {"JPG", CONTENT_TYPE::JPEG},
    {"MAP", CONTENT_TYPE::CSS},
    {"EOT", CONTENT_TYPE::EOT},
    {"SVG", CONTENT_TYPE::SVG},
    {"TTF", CONTENT_TYPE::TTF},
    {"WOF", CONTENT_TYPE::WOFF},
    {"WF2", CONTENT_TYPE::WOFF2}
};

CONTENT_TYPE FileViewReader::getContentType()
{
    // Locate the file extension in the file path
    int dot = viewFilePath.lastIndexOf('.');

    if (dot == -1)
        return CONTENT_TYPE::UNKNOWN;

    // Get the file extension
    String ext = viewFilePath.substring(dot + 1);
    // Convert to uppercase
    ext.toUpperCase();
    // Look up the content type
    FileTypesMap::const_iterator fileType = fileTypesMap.find(ext);
    if (fileType == fileTypesMap.end())
        // Unknown file extension
        return CONTENT_TYPE::UNKNOWN;

    // Return the content type
    return fileType->second;
}
