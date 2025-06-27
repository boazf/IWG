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

#ifndef USE_WIFI
#include <w5100ex.h>
#include <ICMPPingEx.h>
#ifdef DEBUG_ETHERNET
#include <Trace.h>
#endif

ICMPPingEx::~ICMPPingEx()
{ 
    if (!ping)
        return;
    if (s != MAX_SOCK_NUM)
    {
        Lock lock(csSpi);
        W5100Ex.execCmdSn(s, Sock_CLOSE);
    }
    delete ping; 
    ping = NULL;
}


ICMPPing *ICMPPingEx::getPing()
{
    if (ping != NULL)
        return ping;

    s = orgS;
    if (s >= MAX_SOCK_NUM)
    {
    	uint8_t chip, maxindex=MAX_SOCK_NUM;
        chip = W5100Ex.getChip();
        if (!chip) 
            return NULL; // immediate error if no hardware detected
#if MAX_SOCK_NUM > 4
        if (chip == 51) 
            maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
        {
            for (s = 0; s < maxindex; s++) {
                if (W5100Ex.readSnSR(s) == SnSR::CLOSED)
                    break;
            }
        }
        if (s == maxindex)
        {
            s = MAX_SOCK_NUM;
            return NULL;
        }
    }

    ping = new ICMPPing(s, id);
    return ping;
}

ICMPEchoReplyEx ICMPPingEx::operator()(const IPAddress& addr, int nRetries)
{
    ICMPEchoReplyEx result;
    operator()(addr, nRetries, result);
    return result;
}

void ICMPPingEx::operator()(const IPAddress& addr, int nRetries, ICMPEchoReplyEx& result)
{
    Lock lock(csSpi);
    result.success = false;
    if (ping)
    {
#ifdef DEBUG_ETHERNET
        Traceln("ICMPPingEx: can't use object that was created for async ping");
#endif
        return;
    }
    ping = getPing();
    if (!ping)
    {
#ifdef DEBUG_ETHERNET
        Tracef("ICMPPingEx: No available socket for pinging %s\n", addr.toString().c_str());
#endif
        return;
    }   
    ICMPEchoReply reply;
    ping->operator()(addr, nRetries, reply);
    delete ping;
    ping = NULL;
    result.success = true;
    result.reply = reply;
}

#ifdef ICMPPING_ASYNCH_ENABLE
bool ICMPPingEx::asyncStart(const IPAddress& addr, int nRetries, ICMPEchoReply& result)
{
    Lock lock(csSpi);
    return getPing()->asyncStart(addr, nRetries, result);
}

bool ICMPPingEx::asyncComplete(ICMPEchoReply& result)
{
    Lock lock(csSpi);
    if (!ping)
    {
#ifdef DEBUG_ETHERNET
        Traceln("ICMPPingEx: should not call asyncComplete without calling asyncStart first");
#endif
        return false;
    }
    return ping->asyncComplete(result);
}
#endif // CMPPING_ASYNCH_ENABLE

#endif // USE_WIFI