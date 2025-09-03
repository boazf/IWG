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

#ifndef FilesController_h
#define FilesController_h

#include <HttpController.h>

/// @brief FilesController class.
/// This class handles HTTP requests made from the FILES.HTM page.
class FilesController : public HttpController
{
public:
    FilesController()
    {
    }

    bool Get(HttpClientContext &context, const String id);
    bool Post(HttpClientContext &context, const String id);
    bool Put(HttpClientContext &context, const String id);
    bool Delete(HttpClientContext &context, const String id);
    static std::shared_ptr<HttpController> getInstance();

private:
    /// @brief Normalize the file path.
    /// @param path The file path to normalize.
    /// This method replaces spaces in the file path with "%20" to ensure proper URL encoding.
    static void normalizePath(String &path);
    /// @brief Parse the upload headers from the HTTP request.
    /// @param header The header line to parse.
    /// @param boundary The boundary string to extract from the header.
    /// @param fileName The file name to extract from the header.
    static void parseUploadHeaders(const String &header, String &boundary, String &fileName);
};

#endif // FilesController_h
