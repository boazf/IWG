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

#ifndef FileViewReader_h
#define FileViewReader_h

#include <Common.h>
#include <SDUtil.h>
#include <ViewReader.h>

/// @brief This class is a ViewReader that reads from a file on the SD card.
/// It is used to read data from a file and provide it as a view.
class FileViewReader : public ViewReader
{
public:
    /// @brief Class constructor
    /// @param viewFilePath The path to the file on the SD card to read from
    FileViewReader(const String viewFilePath) :
        viewFilePath(viewFilePath)
    {        
    }
    
    virtual ~FileViewReader()
    {
        close();
    }

protected:
    /// @brief Open the view reader
    /// @param buff A buffer to read the view into  
    /// @param buffSize The size of the buffer
    /// @return true if the view reader was opened successfully, false otherwise
    /// @note This function opens the file on the SD card. When read method is called,
    /// it will read from the file and fill the buffer with the data. 
    virtual bool open(byte *buff, int buffSize);
    /// @brief Opens the view reader with a given opened file object
    /// @param buff A buffer to read the view into
    /// @param buffSize The size of the buffer
    /// @param file The opened file object to read from
    /// @return true if the view reader was opened successfully, false otherwise
    /// @note If the file object is not of a valid opened file, it will return false.
    virtual bool open(byte *buff, int buffSize, SdFile file);
    /// @brief Closes the view reader
    /// @note This function closes the file on the SD card. It should be called when
    /// the view reader is no longer needed to free up resources.
    virtual void close();
    /// @brief Reads data from the file into the buffer
    /// @param offset offset in the buffer to start reading from. Maximum actual number of bytes read will be buffSize - offset.
    /// @note This function reads data from the file and fills the buffer with the data. If the file is larger than the buffer,
    /// it will read only the first buffSize bytes. Next call to read() will continue reading from the view from where it left off.
    /// @return The number of bytes read from the file, or -1 if there was an error, or there is no more data.
    virtual int read(int offset);
    /// @brief The file size in bytes
    /// @return number of bytes in the file. This is required to send the correct Content-Length header in the HTTP response.
    virtual long getViewSize();
    /// @brief Gets the last modified time of the file
    /// @param lastModifiedTimeStr A string to fill with the last modified time of the file
    /// @return true if the last modified time was successfully retrieved, false otherwise
    /// @note The last modified time is in the format "YYYY-MM-DD HH:MM:SS". This format
    /// is used in the HTTP headers to indicate the last modified time of the file.
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr);
    /// @brief Gets the content type of the file
    /// @return The content type of the file
    /// @note The content type is used in the HTTP headers to indicate the type of the file.
    /// The content type is determined based on the file extension. If the file extension is not
    /// recognized, it will return CONTENT_TYPE::UNKNOWN.
    virtual CONTENT_TYPE getContentType();

private:
    String viewFilePath;
    SdFile file;
};

#endif // FileViewReader