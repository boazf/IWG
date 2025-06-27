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

#ifndef ICMPPing2_h
#define ICMPPing2_h
#include <Common.h>
#include <Ethernet.h>
#include <ICMPPing.h>

typedef struct  ICMPEchoReplyEx
{
    bool pingSent;
    ICMPEchoReply reply;
} ICMPEchoReplyEx;

class ICMPPingEx
{
public:
    ICMPPingEx(SOCKET s, uint8_t id) : orgS(s), s(MAX_SOCK_NUM), id(id), ping(NULL) {}
    ~ICMPPingEx();
    ICMPEchoReplyEx operator()(const IPAddress&, int nRetries);
    void operator()(const IPAddress& addr, int nRetries, ICMPEchoReplyEx& result);
#ifdef ICMPPING_ASYNCH_ENABLE
    bool asyncStart(const IPAddress& addr, int nRetries, ICMPEchoReply& result);
    bool asyncComplete(ICMPEchoReply& result);
#endif
private:
    ICMPPing *getPing();

private:
    ICMPPing *ping;
    SOCKET s;
    SOCKET orgS;
    uint8_t id;
};

#endif // ICMPPing2_h