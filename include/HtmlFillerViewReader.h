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

#include <Common.h>
#include <ViewReader.h>
#include <memory>

typedef void (*ViewFiller)(String &fill);
typedef int (*GetFillers)(const ViewFiller *&fillers);

/// @brief This class is a ViewReader that fills the view with values from a provided GetFillers function.
/// It replaces filler indices in the view with their corresponding values.
/// It searches for '%' characters followed by a number (filler index), which indicates a filler index and
/// replaces it with the corresponding filler value.
/// After the filler index there should be enough spaces to fill the value.
/// If the filler index is not valid or there is not enough space in the buffer to fill the value, it will
/// fill whatever possible and will continue processing rest of the buffer.
class HtmlFillerViewReader : public ViewReader
{
public:
    /// @brief Class constructor
    /// @param viewReader A pointer to a ViewReader object that will be used to read the view.
    /// The data provided by this ViewReader will be processed to fill the view.
    /// When the HtmlFillerViewReader is opened, it will also open the viewReader.
    /// When the HtmlFillerViewReader is closed, it will also close the viewReader.
    /// When the HtmlFillerViewReader is deleted, it will also delete the viewReader.
    /// @param getFillers A function that retrieves the fillers from the provided GetFillers function.
    /// It should return the number of fillers available and fill the fillers array with the appropriate values
    HtmlFillerViewReader(std::unique_ptr<ViewReader> viewReader, GetFillers getFillers) :
        viewReader(std::move(viewReader)),
        getFillers(getFillers)
    {
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
    std::unique_ptr<ViewReader> viewReader;

private:
    size_t viewHandler(size_t buffSize, bool last = false);
    bool DoFill(int nFill, String &fill);

private:
    int offset;
    bool endOfView;
};

#endif // HtmlFillerViewReader_h