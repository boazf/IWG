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

#ifndef Lock_h
#define Lock_h

#include <Arduino.h>

class CriticalSection
{
public:
    CriticalSection()
    {
        _binarySem = xSemaphoreCreateRecursiveMutex();
        xSemaphoreGiveRecursive(_binarySem);
    }

    ~CriticalSection()
    {
        vSemaphoreDelete(_binarySem);
    }

    void Enter() const
    {
        //portENTER_CRITICAL(&mux);
        xSemaphoreTakeRecursive(_binarySem, portMAX_DELAY);
    }

    void Leave() const
    {
        //portEXIT_CRITICAL(&mux);
        xSemaphoreGiveRecursive(_binarySem);
    }

private:
    //portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    xSemaphoreHandle _binarySem;
};

class Lock
{
public:
    Lock(const CriticalSection &cs) : _cs(cs)
    {
        _cs.Enter();
    }

    ~Lock()
    {
        _cs.Leave();
    }

private:
    const CriticalSection &_cs;
};
#endif // Lock_h