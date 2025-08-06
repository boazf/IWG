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

#ifndef Relays_h
#define Relays_h

enum class PowerState
{
    POWER_OFF,
    POWER_ON,
};

/// @brief Initialize the relays based on configuration.
/// Sets the modem and router relays to their configured pins and powers them on.
void InitRelays();

/// @brief Retrieve the current power state of the router.
/// @return The current power state of the router.
PowerState GetRouterPowerState();

/// @brief Retrieve the current power state of the modem.
/// @return The current power state of the modem.
PowerState GetModemPowerState();

/// @brief Set the power state of the router.
/// @param state The desired power state to set for the router.
void SetRouterPowerState(PowerState state);

/// @brief Set the power state of the modem.
/// @param state The desired power state to set for the modem.
void SetModemPowerState(PowerState state);

#endif // Relays_h