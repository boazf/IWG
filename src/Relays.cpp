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

#include <Arduino.h>
#include <Relays.h>
#include <Config.h>

// Default GPIO pin numbers for the modem and router relays.
#define MODEM_RELAY 26
#define ROUTER_RELAY 25

// Static variables to hold the relay GPIO pin numbers and their current power states.
static byte modemRelay = MODEM_RELAY;
static byte routerRelay = ROUTER_RELAY;
static PowerState modemPowerState;
static PowerState routerPowerState;

/// @brief Convert the power state to a line state for the relay.
/// @param state The power state to convert.
/// @return The corresponding line state (HIGH or LOW).
static int PowerStateToLineState(PowerState state)
{
    return state == PowerState::POWER_ON ? HIGH : LOW;
}

void InitRelays()
{
    // Initialize the modem and router relay GPIO pins based on the configuration.
    if (Config::modemRelay != 0)
        modemRelay = Config::modemRelay;
    if (Config::routerRelay != 0)
        routerRelay = Config::routerRelay;

    // Set the relay pins as outputs and power them on.
    pinMode(modemRelay, OUTPUT);
    pinMode(routerRelay, OUTPUT);
    // Set the initial power states to ON.
    SetModemPowerState(PowerState::POWER_ON);
    SetRouterPowerState(PowerState::POWER_ON);
}

PowerState GetRouterPowerState()
{
    return routerPowerState;
}

PowerState GetModemPowerState()
{
    return modemPowerState;
}

void SetModemPowerState(PowerState state)
{
    modemPowerState = state;
    digitalWrite(modemRelay, PowerStateToLineState(modemPowerState));
}

void SetRouterPowerState(PowerState state)
{
    routerPowerState = state;
    digitalWrite(routerRelay, PowerStateToLineState(routerPowerState));
}
