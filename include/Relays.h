#ifndef Relays_h
#define Relays_h

enum PowerState
{
    POWER_OFF,
    POWER_ON,
};

void InitRelays();

PowerState GetRouterPowerState();

PowerState GetModemPowerState();

void SetRouterPowerState(PowerState state);

void SetModemPowerState(PowerState state);

#endif // Relays_h