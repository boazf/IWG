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

#ifndef DirectFileView_h
#define DirectFileView_h

#include <FileView.h>

/// @brief DirectFileView class.
/// This class takes the resource part of the URL from the request and attempts to 
/// find a file that matches the resource in the SD card.
/// If the file is found, it will return the content of the file to the client.
class DirectFileView : public FileView
{
public:
    // @brief Constructor for DirectFileView.
    // @param viewFilePath The file path to the view file.
    DirectFileView(const char *viewFilePath) :
        FileView(mapper(viewFilePath))
    {
    }
    bool isSingleton() { return false; }

private:
    /// @brief Maps the file path to a specific format.
    /// This method is used to map the file path to a specific format that is used on the SD card.
    static const String mapper(const String _filePath)
    {
        String filePath = _filePath;
        filePath.toUpperCase();
        // Map "GLYPHICONS-HALFLINGS-REGULAR.WOFF" to "GHR.WOF"
        // "GLYPHICONS-HALFLINGS-REGULAR.WOFF2" to "GHR.WF2"
        // This was done when the code used to run on an arduino AVR board. There,
        // there was only one file system type (FAT16). And only 8.3 file names could be used.
        // So this mapping was required. Now that the code only runs on ESP32, mapping can
        // be removed. But for now we remain with this mapping.
        int fileIndex = filePath.indexOf("GLYPHICONS-HALFLINGS-REGULAR");
        if (fileIndex != -1)
        {
            int dotIndex = filePath.lastIndexOf('.');
            String ext = filePath.substring(dotIndex + 1);
            if (ext.equals("WOFF"))
                ext = "WOF";
            else if (ext.equals("WOFF2"))
                ext = "WF2";
            filePath = filePath.substring(0, fileIndex) + "GHR." + ext;
        }
        return filePath;
    }
};

#endif // DirectFileView_h