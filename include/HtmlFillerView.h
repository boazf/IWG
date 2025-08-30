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

/// @brief HTML filler view
class HtmlFillerView : public View
{
public:
    /// @brief Constructor for HTML filler view that is read from file
    /// @param viewFilePath The path to the view file
    /// @param getFillers The function to get the fillers
    HtmlFillerView(const char *viewFilePath, GetFillers getFillers) :
        View(new HtmlFillerViewReader(new FileViewReader(viewFilePath), getFillers))
    {
    }

    /// @brief Constructor for HTML filler view that is read from memory
    /// @param mem The memory buffer containing the view data
    /// @param size The size of the memory buffer
    /// @param contentType The content type of the view
    /// @param getFillers The function to get the fillers
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