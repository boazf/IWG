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

#ifndef HtmlFillerView_h
#define HtmlFillerView_h

#include <View.h>
#include <HtmlFillerViewReader.h>
#include <FileViewReader.h>
#include <MemViewReader.h>

class 
HtmlFillerView : public View
{
public:
    HtmlFillerView(const char *viewFilePath, GetFillers getFillers) :
        View(new HtmlFillerViewReader(new FileViewReader(viewFilePath), getFillers))
    {
    }

    HtmlFillerView(const byte *mem, size_t size, CONTENT_TYPE contentType, GetFillers getFillers) :
        View(new HtmlFillerViewReader(new MemViewReader(mem, size, contentType), getFillers))
    {
    }

    static const String appBase()
    {
        return "'/'";
    }
};

#endif // HtmlFillerView_h