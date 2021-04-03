#include <Indicators.h>

#define LED_FREQ 5000
#define LED_RESOLUTION 5

Indicator::Indicator(uint8_t _channel, uint8_t pin) : 
    channel(_channel)
{
    ledcSetup(channel, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(pin, channel);
    set(LED_OFF);
}

void Indicator::set(ledState state)
{
    currState = state;
    
    switch(state)
    {
        case LED_ON:
            ledcWrite(channel, 31);
            break;
        case LED_IDLE:
            ledcWrite(channel, 1);
            break;
        case LED_OFF:
            ledcWrite(channel, 0);
            break;
    }
}

#define MODEM_RECOVERY_INDICATOR 13
#define MODEM_RECOVERY_INDICATOR_LED_CH 0
#define ROUTER_RECOVERY_INDICATOR 14
#define ROUTER_RECOVERY_INDICATOR_LED_CH 1
#define ULOCK_INDICATOR 4
#define UNLOCK_INDICATOR_LED_CH 2
#define OPERATIONAL_INDICATOR 2
#define OPERATIONAL_INDICATOR_LED_CH 3

Indicator mri(MODEM_RECOVERY_INDICATOR_LED_CH, MODEM_RECOVERY_INDICATOR);
Indicator rri(ROUTER_RECOVERY_INDICATOR_LED_CH, ROUTER_RECOVERY_INDICATOR);
Indicator opi(OPERATIONAL_INDICATOR_LED_CH, OPERATIONAL_INDICATOR);
Indicator uli(UNLOCK_INDICATOR_LED_CH, ULOCK_INDICATOR);
