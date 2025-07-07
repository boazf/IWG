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

#ifndef DummyView_h
#define DummyView_h

#include <View.h>

/// @brief DummyViewReader class.
/// This class is a dummy implementation of the ViewReader interface.
/// It is used with views that doesn't require reading from a file or any other source.
/// For example the default view is using this view reader.
class DummyViewReader : public ViewReader
{
public:
    DummyViewReader()
    {
    }

protected:
    int read(int offset)
    {
        return -1;
    }

    long getViewSize()
    {
        return 0;
    }

    bool getLastModifiedTime(String &lastModifiedTimeStr)
    {
        return false;
    }

    CONTENT_TYPE getContentType()
    {
        return CONTENT_TYPE::HTML;
    }

    void close()
    {
    }
};

class DummyView : public View
{
public:
    DummyView(const char *viewFilePath) :
        View(new DummyViewReader())
    {
    }

    bool isSingleton() { return false; }
    static HttpController *getInstance() { return new DummyView(""); }

protected:
};
#endif // DummyView_h