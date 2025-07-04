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

class MemViewReader : public ViewReader
{
public:
    MemViewReader(const byte *mem, size_t size, CONTENT_TYPE contentType) : 
        mem(mem), 
        size(size), 
        contentType(contentType), 
        offset(0) 
    {}

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

        readSize = min<size_t>(readSize, size - offset);
        memcpy(readBuff, mem + offset, readSize);
        offset += readSize;

        return readSize;
    }
};
#endif // MemViewReader_h
