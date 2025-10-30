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

#ifndef ViewReader_h
#define ViewReader_h

#include <Arduino.h>
#include <HttpHeaders.h>

class ViewReader {
public:
    virtual ~ViewReader() {}
    /// @brief Open the view reader
    /// @param buff A buffer to read the view into
    /// @param buffSize The size of the buffer
    /// @return true if the view reader was opened successfully, false otherwise
    virtual bool open(byte *buff, int buffSize) { this->buff = buff; this->buffSize = buffSize; return true; };
    /// @brief Closes the view reader
    /// @note This function should be called when the view reader is no longer needed to free up resources.
    /// It is a virtual function that can be overridden by derived classes to implement specific closing logic
    virtual void close() {};
    /// @brief Gets the last modified time of the file
    /// @param lastModifiedTimeStr A string to fill with the last modified time of the file
    /// @return true if the last modified time was successfully retrieved, false otherwise
    /// @note The last modified time is in the format "YYYY-MM-DD HH:MM:SS". This format
    /// is used in the HTTP headers to indicate the last modified time of the file.
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) = 0;
    /// @brief Gets the content type of the file
    /// @return The content type of the file
    /// @note The content type is used in the HTTP headers to indicate the type of the file.
    /// The content type is determined based on the file extension. If the file extension is not
    /// recognized, it will return CONTENT_TYPE::UNKNOWN.
    virtual CONTENT_TYPE getContentType() = 0;
    /// @brief The size of the view in bytes
    /// @return This is required to send the correct Content-Length header in the HTTP response.
    virtual long getViewSize() = 0;
    /// @brief Reads data from the view into the buffer beginning at offset 0
    /// @note This function reads data from the view and fills the buffer with the data. If the view is larger than the buffer,
    /// it will read only the first buffSize bytes. Next call to read() will continue reading from the view from where it left off.
    /// @return The number of bytes read from the view, or -1 if there was an error, or there is no more data.
    virtual int read() { return read(0); };
    /// @brief Reads data from the view into the buffer
    /// @param offset offset in the buffer to start reading from.  Maximum actual number of bytes read will be buffSize - offset.
    /// @note This function reads data from the view and fills the buffer with the data. If the view is larger than the buffer,
    /// it will read only the first buffSize bytes. Next call to read() will continue reading from the view from where it left off.
    /// @return The number of bytes read from the view, or -1 if there was an error, or there is no more data.
    virtual int read(int offset) = 0;

protected:
    byte *buff;
    int buffSize;
};

#endif // ViewReader_h