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

#ifndef FileView_h
#define FileView_h

#include <View.h>
#include <FileViewReader.h>

/// @brief FileView class.
/// This class is a view that reads from a file on the SD card.
class FileView : public View
{
public:
    FileView(const String viewFilePath) : 
        View(std::unique_ptr<ViewReader>(new FileViewReader(viewFilePath)))
    {
    }
};
#endif // FileView_h