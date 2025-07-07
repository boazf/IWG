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
    // Check if the task is already running
    if (hGWConnTestTask != NULL)
        return ;

    isConnected = false; // Reset the connection status. Assume it is not connected until proven otherwise.
    tDelay = delay; // Set the delay before starting pinging the gateway
    // Create the task to test the gateway connection
    xTaskCreate(gwConnTestTask, "GWConnTest", 2 * 1024, this, tskIDLE_PRIORITY, &hGWConnTestTask);
}

// Static function to be used as a task entry point
// It calls the gwConnTestTask method of the GWConnTest instance
void GWConnTest::gwConnTestTask(void *param)
{
    static_cast<GWConnTest *>(param)->gwConnTestTask();
}

bool GWConnTest::ping(int attempts, int tInterval)
{
    bool success = true;

    // Check of the Ethernet is connected. Once it is connected, return true
    for (int i = 0; i < attempts && success; i++)
    {
        delay(tInterval);
        success = ping();
    }

    return success;
}

bool GWConnTest::ping()
{
    // Get the gateway IP address of the gateway
    IPAddress gw = Eth.gatewayIP();
#ifdef USE_WIFI
    // Use the ESP32Ping library to ping the gateway
    // The ping method returns true if the gateway is reachable, false otherwise
    return Ping.ping(gw, 1);
#else
    // Use the ICMPEchoReplyEx class to ping the gateway
    // The ping method returns true if the gateway is reachable, false otherwise
    ICMPPingEx ping(MAX_SOCK_NUM, 2);
    ICMPEchoReplyEx result = ping(gw, 1);
    return result.pingSent && result.reply.status == SUCCESS;
#endif
}

void GWConnTest::gwConnTestTask()
{
    // This is the task that runs the gateway connection test
    // It will ping the gateway until it is reachable

    // Wait before starting the pinging
    delay(tDelay);
#ifdef DEBUG_ETHERNET
    Tracef("GWConnTest: Starting pinging %s\n", Eth.gatewayIP().toString().c_str());
#endif
    // Ping the gateway until it is reachable
    while (!ping(5, 500));
#ifdef DEBUG_ETHERNET
    Traceln("GW Connection retrieved!");
#endif
    hGWConnTestTask = NULL; // Clear the task handle to indicate that the task is no longer running
    isConnected = true; // Set the connection status to true, indicating that the gateway is reachable
    vTaskDelete(NULL); // Delete the task
}

bool GWConnTest::IsConnected()
{
    return isConnected;
}

/// @brief Global instance of GWConnTest.
GWConnTest gwConnTest;