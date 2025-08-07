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

#ifndef PwrCntl_h
#define PwrCntl_h
#include <Arduino.h>
#include <Observers.h>

/// @brief Initialize power control.
/// @param addObserver If true, adds an observer for hard reset events.
/// This function initializes the power control system by setting up the watchdog timer and creating a task to trigger the watchdog.
/// It also waits for the watchdog timer to load before proceeding.
void InitPowerControl(bool addObserver = true);
/// @brief Perform a hard reset.
/// @param timeout Before performing the hard reset, components that registered for hard reset event will be called 
///        to perform a graceful shutdown. The time for this shutdown should be limited by this timeout.
/// @param returnTimeout The time to wait after the hard reset is performed before returning from the function.
///        This is usefull when the watchdog disable switch is turned on.
/// @note This function performs a hard reset by signaling the watchdog semaphore, which will cause the watchdog
///       task to end and disconnect the power.
/// @note If the watchdog disable switch is turned on, this function will return after returnTimeout.
///       The watchdog switch is only intended to be used when debugging. In normal operation it should be turned off.
/// @note Components registered for the hard reset event will be called twice before the hard reset occurs:
///       First Call – This call is intended to prepare for shutdown by performing any quick, time-unbounded operations.
///       Its purpose is to immediately stop all further operations in each of the components.
///       Second Call – This call is for performing any necessary cleanup operations before the hard reset. 
///       These operations must complete within the specified timeout.
///       And a third call is done in case the watchdog disable switch is turned on, to indicate that the hard reset failed.
///       In this call, the components should reverse the shutdown operations and allow the system to continue running.
void HardReset(int timeout, int returnTimeout = portMAX_DELAY);

/// @brief Stages of the hard reset process.
enum class HardResetStage
{
    /// @brief Stage to prepare for shutdown. Quickly stop any further operations.
    prepare, 
    /// @brief Stage to perform cleanup operations before the hard reset. Must complete within the specified timeout.
    shutdown,
    /// @brief Stage to indicate that the watchdog disable switch is turned on, hence hard reset was not done, allowing components to reverse shutdown operations.
    failure
};

/// @brief Parameters for the hard reset event.
/// This class holds the stage of the hard reset and the timeout for the shutdown operations.
class HardResetEventParam
{
public:
    HardResetEventParam(HardResetStage stage, int timeout) : stage(stage), timeout(timeout) {}
    HardResetStage stage;
    int timeout;
};

/// @brief Global Observers object for hard reset events.
extern Observers<HardResetEventParam> hardResetEvent;

#endif // PwrCntl_h