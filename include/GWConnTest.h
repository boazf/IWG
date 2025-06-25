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

class GWConnTest
{
public:
    GWConnTest() :
        hGWConnTestTask(NULL),
        isConnected(true)
    {
    }

    void Start(time_t delay);
    bool IsConnected();
    static bool ping(int attempts, int tInterval);

private:
    bool isConnected;
    TaskHandle_t hGWConnTestTask;
    time_t tDelay;

private:
    static void gwConnTestTask(void *param);
    void gwConnTestTask();
    static bool ping();
};

extern GWConnTest gwConnTest;

#endif // GWConnTest_h