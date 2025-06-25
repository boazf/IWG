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

class DirectFileView : public FileView
{
public:
    DirectFileView(const char *viewFilePath) :
        FileView(mapper(viewFilePath))
    {
    }
    bool isSingleton() { return false; }

private:
    static const String mapper(const String _filePath)
    {
        String filePath = _filePath;
        filePath.toUpperCase();
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