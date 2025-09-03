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

#ifndef IndexView_h
#define IndexView_h

#include <HtmlFillerView.h>
#include <atomic>

// This class represents the index view of the application.
// It inherits from HtmlFillerView and provides the functionality to fill the HTML template with dynamic data.
// The index view is used to display the main page of the application, which includes various controls and information.
class IndexView : public HtmlFillerView
{
public:
    IndexView();     
    bool redirect(EthClient &client, const String &_id);
    static std::shared_ptr<HttpController> getInstance() { return std::make_shared<IndexView>(); }

protected:
    static int getFillers(const ViewFiller *&fillers);

private:
    static ViewFiller fillers[];

private:
    static std::atomic<int> id;
};
#endif // IndexView_h