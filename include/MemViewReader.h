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

#ifndef MemViewReader_h
#define MemViewReader_h

#include <ViewReader.h>

/// @brief This class is a ViewReader that reads from a memory buffer.
/// It is used to read data from a memory buffer and provide it as a view.
class MemViewReader : public ViewReader
{
public:
    /// @brief Class constructor
    /// @param mem A buffer in memory to read from
    /// @param size The size of the memory buffer
    /// @param contentType The content type of the data in the memory buffer
    /// @note The content type is used to determine how the data should be interpreted.
    MemViewReader(const byte *mem, size_t size, CONTENT_TYPE contentType) : 
        mem(mem), 
        size(size), 
        contentType(contentType), 
        offset(0) 
    {}

    virtual bool open(byte *buff, int buffSize) override 
    {
        offset = 0; // Reset offset for reading
        return ViewReader::open(buff, buffSize);
    };
    virtual void close() {};
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) {return false; };
    virtual CONTENT_TYPE getContentType() { return contentType; };
    virtual long getViewSize() { return size; }
    virtual int read(int offset) { return read(buff + offset, buffSize - offset); }

    
protected:
    const byte *mem;
    const size_t size;
    CONTENT_TYPE contentType;
    size_t offset;

private:
    int read(byte *readBuff, size_t readSize)
    {
        if (offset >= size)
            return -1;

        readSize = std::min<size_t>(readSize, size - offset);
        memcpy(readBuff, mem + offset, readSize);
        offset += readSize;

        return readSize;
    }
};
#endif // MemViewReader_h
