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

#ifndef DefaultView_h
#define DefaultView_h

#include <DummyView.h>
#include <HttpHeaders.h>

/// @brief The default view. This is the view that is called when the URL is set to the root ("/").
class DefaultView : public DummyView
{
public:
    DefaultView(const char *viewFilePath) :
        DummyView(viewFilePath)
    {
    }

    static std::shared_ptr<HttpController> getInstance() { return std::make_shared<DefaultView>(""); }

protected:
    /// @brief Redirects the client to the index page.
    /// This method is called when the client requests the root URL ("/").
    /// @param client The EthClient instance representing the client connection.
    /// This client is used to send the HTTP response.
    /// @param id The identifier for the resource being requested.
    /// @return True to indicate that a redirection response was sent.
    bool redirect(EthClient &client, const String &id)
    {
        HttpHeaders::Header additionalHeaders[] = { {"Location", "/index"} };
        HttpHeaders headers(client);
        headers.sendHeaderSection(302, true, additionalHeaders, NELEMS(additionalHeaders));
        return true;
    }

};
#endif // DefaultView_h