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

#define MODEM_RELAY 7
#define ROUTER_RELAY 8

static byte modemRelay = MODEM_RELAY;
static byte routerRelay = ROUTER_RELAY;
static PowerState modemPowerState;
static PowerState routerPowerState;

int PowerStateToLineState(PowerState state)
{
    return state == PowerState::POWER_ON ? HIGH : LOW;
}

void InitRelays()
{
    if (Config::modemRelay != 0)
        modemRelay = Config::modemRelay;
    if (Config::routerRelay != 0)
        routerRelay = Config::routerRelay;

    pinMode(modemRelay, OUTPUT);
    pinMode(routerRelay, OUTPUT);
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
