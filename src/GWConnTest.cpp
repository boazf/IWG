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

#include <Common.h>
#include <GWConnTest.h>
#include <EthernetUtil.h>
#ifdef USE_WIFI
#include <ESP32ping.h>
#else
#include <ICMPPingEx.h>
#endif
#ifdef DEBUG_ETHERNET
#include <Trace.h>
#endif

void GWConnTest::Start(time_t delay)
{
    if (hGWConnTestTask != NULL)
        return ;

    tDelay = delay;
    xTaskCreate(gwConnTestTask, "GWConnTest", 2 * 1024, this, tskIDLE_PRIORITY, &hGWConnTestTask);
}

void GWConnTest::gwConnTestTask(void *param)
{
    static_cast<GWConnTest *>(param)->gwConnTestTask();
}

bool GWConnTest::ping(int attempts, int tInterval)
{
    bool success = true;

    for (int i = 0; i < attempts && success; i++)
    {
        delay(tInterval);
        success = ping();
    }

    return success;
}

bool GWConnTest::ping()
{
    IPAddress gw = Eth.gatewayIP();
#ifdef USE_WIFI
    return Ping.ping(gw, 1);
#else
    ICMPPingEx ping(MAX_SOCK_NUM, 2);
    ICMPEchoReplyEx result = ping(gw, 1);
    return result.success && result.reply.status == SUCCESS;
#endif
}

void GWConnTest::gwConnTestTask()
{
    delay(tDelay);
#ifdef DEBUG_ETHERNET
    Tracef("GWConnTest: Starting pinging %s\n", Eth.gatewayIP().toString().c_str());
#endif
    while (!ping(5, 500))
        isConnected = false;
#ifdef DEBUG_ETHERNET
    Traceln("GW Connection retrieved!");
#endif
    hGWConnTestTask = NULL;
    isConnected = true;
    vTaskDelete(NULL);
}

bool GWConnTest::IsConnected()
{
    return isConnected;
}

GWConnTest gwConnTest;