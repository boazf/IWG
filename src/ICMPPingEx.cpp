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

    // Close the socket if it is open
    if (s != MAX_SOCK_NUM)
    {
        Lock lock(csSpi);
        W5100Ex.execCmdSn(s, Sock_CLOSE);
    }
    // Delete the ping object
    delete ping; 
    ping = NULL;
}


ICMPPing *ICMPPingEx::getPing()
{
    // If ping is already set, it means we are trying to use an object that was created for async ping
    if (ping != NULL)
        return ping;

    s = orgS;
    if (s >= MAX_SOCK_NUM)
    {
        // If the socket is not set, find a free socket
    	uint8_t chip, maxindex=MAX_SOCK_NUM;
        chip = W5100Ex.getChip();
        if (!chip) 
            return NULL; // immediate error if no hardware detected
#if MAX_SOCK_NUM > 4
        if (chip == 51) 
            maxindex = 4; // W5100 chip never supports more than 4 sockets
#endif
        {
            // Look for a free socket
            for (s = 0; s < maxindex; s++) {
                if (W5100Ex.readSnSR(s) == SnSR::CLOSED)
                    break;
            }
        }

        // If no available socket is found, return NULL
        if (s == maxindex)
        {
            s = MAX_SOCK_NUM;
            return NULL;
        }
    }

    // Create a new ICMPPing object that uses the found socket
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
    result.pingSent = false;
    if (ping)
    {
        // If ping is already set, it means we are trying to use an object that was created for async ping
        // The failure is indicated by the pingSent field in the result
#ifdef DEBUG_ETHERNET
        Traceln("ICMPPingEx: can't use object that was created for async ping");
#endif
        return;
    }
    // Get a new ICMPPing object
    ping = getPing();
    if (!ping)
    {
        // If no available socket is found, return. The failure is indicated by the pingSent field in the result
#ifdef DEBUG_ETHERNET
        Tracef("ICMPPingEx: No available socket for pinging %s\n", addr.toString().c_str());
#endif
        return;
    }   
    // Execute the ping operation
    ICMPEchoReply reply;
    ping->operator()(addr, nRetries, reply);
    // Clean up the ping object
    delete ping;
    ping = NULL;
    // Set the result
    result.pingSent = true;
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
        // If ping is not set, it means we are trying to complete an async ping without starting it first.
#ifdef DEBUG_ETHERNET
        Traceln("ICMPPingEx: should not call asyncComplete without calling asyncStart first");
#endif
        return false;
    }
    // Call the asyncComplete method on the ping object
    return ping->asyncComplete(result);
}
#endif // ICMPPING_ASYNCH_ENABLE

#endif // USE_WIFI