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
#ifndef NTPClient_h
#define NTPClient_h

#include <EthernetUtil.h>
#include <time.h>
#include <Common.h>

class NTPClient
{
public:
    static time_t getUTC();

private:
    static const unsigned int localPort;  // local port to listen for UDP packets
    static const int NTP_PACKET_SIZE;     // NTP time stamp is in the first 48 bytes of the message
    static byte packetBuffer[];           //buffer to hold incoming and outgoing packets
    static EthUDP Udp;               // A UDP instance to let us send and receive packets over UDP
};

#endif // NTPClient_h
#endif // USE_WIFI