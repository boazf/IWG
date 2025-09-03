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

#ifndef HistoryView_h
#define HistoryView_h

#include <FileView.h>
#include <Lock.h>

/// @brief History view file reader
class HistoryViewReader : public FileViewReader
{
public:
    HistoryViewReader(const char *viewFile) :
        FileViewReader(viewFile)
    {}

    /// @brief Opens the history view file for reading.
    /// @param buff The buffer to read the file contents into.
    /// @param buffSize The size of the buffer.
    /// @return True if the operation was successful, false otherwise.
    virtual bool open(byte *buff, int buffSize);

private:
    static CriticalSection cs;
};

/// @brief History view
class HistoryView : public View
{
public:
    HistoryView(const char *viewFile) : 
        View(std::unique_ptr<ViewReader>(new HistoryViewReader(viewFile)))
    {
    }

    static std::shared_ptr<HttpController> getInstance() { return std::make_shared<HistoryView>("/HISTORY.HTM"); }
};
#endif // HistoryView_h