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

#include <SDUtil.h>
#include <ViewReader.h>

class FileViewReader : public ViewReader
{
public:
    FileViewReader(const String viewFilePath) :
        viewFilePath(viewFilePath)
    {        
    }
    
    virtual ~FileViewReader()
    {
        close();
    }

protected:
    virtual bool open(byte *buff, int buffSize);
    virtual bool open(byte *buff, int buffSize, SdFile file);
    virtual void close();
    virtual int read();
    int read(int offset);
    virtual long getViewSize();
    virtual bool getLastModifiedTime(String &lastModifiedTimeStr);
    virtual CONTENT_TYPE getContentType();

private:
    String viewFilePath;
    SdFile file;
};

#endif // FileViewReader