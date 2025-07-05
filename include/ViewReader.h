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
#include <Common.h>

class ViewReader {
public:
    virtual ~ViewReader() {}
    virtual bool open(byte *buff, int buffSize) { this->buff = buff; this->buffSize = buffSize; return true; };
    virtual void close() {};
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) = 0;
    virtual CONTENT_TYPE getContentType() = 0;
    virtual long getViewSize() = 0;
    virtual int read() { return read(0); };
    virtual int read(int offset) = 0;

protected:
    byte *buff;
    int buffSize;
};

#endif // ViewReader_h