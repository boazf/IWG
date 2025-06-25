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

#ifndef HtmlFillerViewReader_h
#define HtmlFillerViewReader_h

#include <ViewReader.h>

typedef void (*ViewFiller)(String &fill);
typedef int (*GetFillers)(const ViewFiller *&fillers);

class HtmlFillerViewReader : public ViewReader
{
public:
    HtmlFillerViewReader(ViewReader *viewReader, GetFillers getFillers) :
        viewReader(viewReader),
        getFillers(getFillers)
    {
    }

    virtual ~HtmlFillerViewReader()
    {
        delete viewReader;
    }

    virtual void close() { viewReader->close(); };
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr) { return viewReader->getLastModifiedTime(lastModifiedTimeStr); };
    virtual CONTENT_TYPE getContentType() { return viewReader->getContentType(); };
    virtual long getViewSize() { return viewReader->getViewSize(); };
    virtual bool open(byte *buff, int buffSize);
    virtual int read();
    virtual int read(int offset) { return -1; }

protected:
    GetFillers getFillers;
    ViewReader *viewReader;

private:
    size_t viewHandler(byte *buff, size_t buffSize);
    bool DoFill(int nFill, String &fill);

private:
    int offset;
};

#endif // HtmlFillerViewReader_h