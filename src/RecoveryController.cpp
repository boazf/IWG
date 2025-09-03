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

#include <RecoveryController.h>
#include <RecoveryControl.h>
#include <EthernetUtil.h>
#include <HttpHeaders.h>
#ifdef DEBUG_HTTP_SERVER
#include <Trace.h>
#endif

// Get request is unimplemented.
bool RecoveryController::Get(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Get");
#endif
    return false;
}

// Put request is unimplemented.
bool RecoveryController::Put(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Put");
#endif
    return false;
}

// Delete request is unimplemented.
bool RecoveryController::Delete(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Delete");
#endif
    return false;
}

// Post request handles recovery operations based on the recovery type specified in the request body.
// It reads the request body, extracts the recovery type, and starts the recovery process.
bool RecoveryController::Post(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Post");
#endif
    String content;
    EthClient client = context.getClient();

    // Read the request body from the client connection.
    // This assumes that the request body is sent as a JSON object containing the recovery type.
    while (client.connected() && !client.available()) {
        delay(10); // Wait for data to be available
    }
    while (client.available())
    {
        content += (char)client.read();
    }

#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Trace("RecoveryController::Post: ");
        Traceln(content);
    }
#endif

    // Parse the recovery type from the request body.
    RecoveryTypes recoveryType;
    sscanf(content.c_str(), "{\"recoveryType\":%d}", reinterpret_cast<int*>(&recoveryType));

#ifdef DEBUG_HTTP_SERVER
    TRACE_BLOCK
	{
        Trace("RecoveryType: ");
        Traceln((int)recoveryType);
    }
#endif

    // Start the recovery process based on the recovery type.
    // The recoveryControl instance is responsible for managing the recovery state machine and performing recovery actions.
    recoveryControl.StartRecoveryCycles(recoveryType);

    // Send a 200 OK response to the client with appropriate headers.
    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}

static std::shared_ptr<HttpController> recoveryController = std::make_shared<RecoveryController>();

// This method returns a singleton instance of the RecoveryController.
std::shared_ptr<HttpController> RecoveryController::getInstance() { return recoveryController; }
