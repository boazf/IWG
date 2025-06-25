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

#include <FileViewReader.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

bool FileViewReader::open(byte *buff, int buffSize)
{
    AutoSD autoSD;
    String fileName;
    fileName = "/wwwroot" + viewFilePath;
    SdFile file = SD.open(fileName, FILE_READ);
#ifdef DEBUG_HTTP_SERVER
    if (!file)
    {
        LOCK_TRACE();
        Trace("Failed to open file ");
        Traceln(fileName.c_str());
    }
#endif

    return open(buff, buffSize, file);
}

bool FileViewReader::open(byte *buff, int buffSize, SdFile file)
{
    if (!ViewReader::open(buff, buffSize))
        return false;
    this->file = file;
    if (file)
    {
        SD.begin();
        return true;
    }

    return false;
}

void FileViewReader::close()
{
    if (file)
    {
        file.close();
        SD.end();
    }
}

int FileViewReader::read()
{
    return file.read(buff, buffSize);
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
    int dot = viewFilePath.lastIndexOf('.');

    if (dot == -1)
        return CONTENT_TYPE::UNKNOWN;

    String ext = viewFilePath.substring(dot + 1);
    ext.toUpperCase();
    FileTypesMap::const_iterator fileType = fileTypesMap.find(ext);
    if (fileType == fileTypesMap.end())
        return CONTENT_TYPE::UNKNOWN;

    return fileType->second;
}
