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

#ifndef TimeUtil_h
#define TimeUtil_h

#include <time.h>
#include <Observers.h>

/// @brief Initialize the system time.
/// This function sets the system time based on the configured time server and timezone.
/// It also registers an observer for application configuration changes related to time settings.
/// If not using Wi-Fi, it creates a task to periodically update the time.
void InitTime();
/// @brief Check if the given time is valid.
/// @param t The time to check.
/// @return True if the time is valid, false otherwise.
bool isValidTime(time_t t);

/// @brief Current time in seconds since epoch.
/// This macro provides the current time in seconds since the Unix epoch (January 1, 1970).
#define t_now (([]()->time_t{time_t now; time(&now); return now; })())

/// @brief Parameter for time change events.
class TimeChangedParam
{
public:
    TimeChangedParam(time_t _currTime)
    {
        currTime = _currTime;
    }

    time_t currTime;
};

/// @brief Observer for time change events.
/// This observer class allows other components to listen for changes in the system time.
extern Observers<TimeChangedParam> timeChanged;

#endif // TimeUtil_h