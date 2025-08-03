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

/// @brief Extended ICMP Echo Reply structure to include ping sent flag.
/// This structure extends the standard ICMP Echo Reply to include a boolean flag indicating whether the ping was sent.
typedef struct  ICMPEchoReplyEx
{
    bool pingSent;
    ICMPEchoReply reply;
} ICMPEchoReplyEx;

class ICMPPingEx
{
public:
    /// @brief Constructor for the ICMPPingEx class.
    /// @param s The socket to use for the ping.
    /// @param id The ID to use for the ping.
    ICMPPingEx(SOCKET s, uint8_t id) : orgS(s), s(MAX_SOCK_NUM), id(id), ping(NULL) {}
    /// @brief Destructor for the ICMPPingEx class.
    /// This destructor closes the socket if it is open and deletes the ping object.
    ~ICMPPingEx();

    /// @brief Execute a synchronous ping operation.
    /// @param addr The IP address to ping.
    /// @param nRetries The number of retries to attempt if the ping fails.
    /// @return An ICMPEchoReplyEx containing the result of the ping operation.
    ICMPEchoReplyEx operator()(const IPAddress&, int nRetries);
    /// @brief Execute a synchronous ping operation with a pre-allocated result object.
    /// @param addr The IP address to ping.
    /// @param nRetries The number of retries to attempt if the ping fails.
    /// @param result The ICMPEchoReplyEx object to store the result of the ping operation.
    void operator()(const IPAddress& addr, int nRetries, ICMPEchoReplyEx& result);
#ifdef ICMPPING_ASYNCH_ENABLE
    /// @brief Start an asynchronous ping operation.
    /// @param addr The IP address to ping.
    /// @param nRetries The number of retries to attempt if the ping fails.
    /// @param result The ICMPEchoReply object to store the result of the ping operation.
    /// @return True if the ping was sent successfully, false otherwise.
    bool asyncStart(const IPAddress& addr, int nRetries, ICMPEchoReply& result);
    /// @brief Complete an asynchronous ping operation.
    /// @param result The ICMPEchoReply object to store the result of the ping operation.
    /// @return True if the ping was completed successfully, false otherwise.
    /// @note If the return value is false, it means that the ping operation is still in progress and should be checked again later.
    bool asyncComplete(ICMPEchoReply& result);
#endif
private:
    /// @brief Get a ICMPPing object.
    /// @return A pointer to the ICMPPing object.
    /// @note It looks for a free socket to use. Then allocates a new ICMPPing object that uses that socket. 
    ///       If no available socket is found, it returns NULL.
    /// @note The ping object is stored in the ping member variable. Further calls to this method will return the same object.
    ICMPPing *getPing();

private:
    ICMPPing *ping;
    SOCKET s;
    SOCKET orgS;
    uint8_t id;
};

#endif // ICMPPing2_h