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
    return state == POWER_ON ? HIGH : LOW;
}

void InitRelays()
{
    if (Config::modemRelay != 0)
        modemRelay = Config::modemRelay;
    if (Config::routerRelay != 0)
        routerRelay = Config::routerRelay;

    pinMode(modemRelay, 7);
    pinMode(routerRelay, 8);
    SetModemPowerState(POWER_ON);
    SetRouterPowerState(POWER_ON);
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
