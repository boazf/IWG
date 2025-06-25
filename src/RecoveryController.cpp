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

bool RecoveryController::Get(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Get");
#endif
    return false;
}

bool RecoveryController::Put(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Put");
#endif
    return false;
}

bool RecoveryController::Delete(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Delete");
#endif
    return false;
}

bool RecoveryController::Post(HttpClientContext &context, const String id)
{
#ifdef DEBUG_HTTP_SERVER
    Traceln("RecoveryController Post");
#endif
    String content;
    EthClient client = context.getClient();

    while (client.available())
    {
        content += (char)client.read();
    }

#ifdef DEBUG_HTTP_SERVER
    {
		LOCK_TRACE();
        Trace("RecoveryController::Post: ");
        Traceln(content);
    }
#endif

    RecoveryTypes recoveryType;
    sscanf(content.c_str(), "{\"recoveryType\":%d}", reinterpret_cast<int*>(&recoveryType));

#ifdef DEBUG_HTTP_SERVER
    {
		LOCK_TRACE();
        Trace("RecoveryType: ");
        Traceln((int)recoveryType);
    }
#endif

    recoveryControl.StartRecoveryCycles(recoveryType);

    HttpHeaders::Header additionalHeaders[] = { {"Access-Control-Allow-Origin", "*" }, {"Cache-Control", "no-cache"} };
    HttpHeaders headers(client);
    headers.sendHeaderSection(200, true, additionalHeaders, NELEMS(additionalHeaders));

    return true;
}

HttpController *RecoveryController::getInstance() { return &recoveryController; }

RecoveryController recoveryController;