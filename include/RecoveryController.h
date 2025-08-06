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

#ifndef RecoveryController_h
#define RecoveryController_h

#include <HttpController.h>

/// @brief This class implements a recovery controller for handling HTTP requests related to device recovery.
/// This controller is a singleton, meaning only one instance of it will exist in the system.
class RecoveryController : public HttpController
{
public:
    RecoveryController()
    {
    }

    bool Get(HttpClientContext &context, const String id);
    bool Post(HttpClientContext &context, const String id);
    bool Put(HttpClientContext &context, const String id);
    bool Delete(HttpClientContext &context, const String id);
    bool isSingleton() { return true; }
    static HttpController *getInstance();
};

extern RecoveryController recoveryController;

#endif // RecoveryController_h