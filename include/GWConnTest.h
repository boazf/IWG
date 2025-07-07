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

#ifndef GWConnTest_h
#define GWConnTest_h

#include <Arduino.h>

/// @brief GWConnTest class.
/// This class is used to test the connection to the gateway.
/// It runs a task that periodically checks the connection status to the gateway
/// by pinging the gateway.
/// This is used after a router recovery to ensure that the connection to the gateway is restored
/// before trying to ping addresses on the internet.
class GWConnTest
{
public:
    /// @brief Constructor for GWConnTest.
    GWConnTest() :
        hGWConnTestTask(NULL),
        isConnected(true)
    {
    }

    /// @brief Start the gateway connection test task.
    /// @param delay The delay in milliseconds before starting the task.
    void Start(time_t delay);
    /// @brief Return true if the gateway is connected, false otherwise
    bool IsConnected();
    /// @brief Ping the gateway to check if it is reachable.
    /// @param attempts The number of ping attempts to make.
    /// @param tInterval The time interval in milliseconds between ping attempts.
    /// If the gateway is reachable, the method returns true, otherwise it returns false.
    /// @return True if the gateway is reachable, false otherwise.
    /// @note Once a ping attempt is successful, the method will return true immediately.
    /// If all ping attempts fail, the method will return false.
    static bool ping(int attempts, int tInterval);

private:
    /// @brief Indicates if the gateway is connected.
    bool isConnected;
    /// @brief Handle for the gateway connection test task.
    TaskHandle_t hGWConnTestTask;
    /// @brief Delay in milliseconds before the next ping attempt.
    time_t tDelay;

private:
    /// @brief The task function that runs the gateway connection test.
    /// @param param A pointer to the GWConnTest instance.
    /// @note The function is static so it can be used as a task entry point.
    /// It calls the gwConnTestTask method of the GWConnTest instance.
    static void gwConnTestTask(void *param);
    /// @brief The method that performs the gateway connection test.
    /// This method is called by the gwConnTestTask(void*) function.
    void gwConnTestTask();
    /// @brief Ping the gateway to check if it is reachable.
    /// @return True if the gateway is reachable, false otherwise.
    static bool ping();
};

/// @brief Global instance of GWConnTest.
extern GWConnTest gwConnTest;

#endif // GWConnTest_h