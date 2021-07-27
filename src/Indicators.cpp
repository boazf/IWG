#include <Indicators.h>
#include <Trace.h>

#define LED_FREQ 5000
#define LED_RESOLUTION 5

#define LED_ON_DUTY 31
#define LED_OFF_DUTY 0
#define LED_IDLE_DUTY 1

#define BLINK_FREQ 2 // Hz

LinkedList<Indicator *> Indicator::blinkingIndicators;
TaskHandle_t Indicator::blinkerTaskHandle = NULL;

void Indicator::BlinkerTask(void *param)
{
    while(true)
    {
        blinkingIndicators.ScanNodes(Blink, NULL);
        vTaskDelay(1000 / BLINK_FREQ / 2 / portTICK_PERIOD_MS);
    }
}

Indicator::Indicator(uint8_t _channel, uint8_t pin) : 
    channel(_channel)
{
    ledcSetup(channel, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(pin, channel);
    set(LED_OFF);
    if (blinkerTaskHandle == NULL)
    {
        xTaskCreate(BlinkerTask, "Blinker", 8*1024, NULL, tskIDLE_PRIORITY, &blinkerTaskHandle);
    }
}

void Indicator::Blink()
{
    ledcWrite(channel, ledcRead(channel) == LED_ON_DUTY ? LED_OFF_DUTY : LED_ON_DUTY);
}

bool Indicator::Blink(Indicator *const &indicator, const void *param)
{
    indicator->Blink();
    return true;
}

void Indicator::setInternal(ledState state)
{
    switch(state)
    {
        case LED_ON:
            ledcWrite(channel, LED_ON_DUTY);
            break;
        case LED_IDLE:
            ledcWrite(channel, LED_IDLE_DUTY);
            break;
        case LED_OFF:
            ledcWrite(channel, LED_OFF_DUTY);
            break;
        case LED_BLINK:
            break;
    }
}

void Indicator::set(ledState state)
{
    if (currState == LED_BLINK)
    {
        blinkingIndicators.Delete(this);
    }

    currState = state;
    
    switch(state)
    {
        case LED_ON:
        case LED_IDLE:
        case LED_OFF:
            setInternal(state);
            break;
        case LED_BLINK:
            setInternal(LED_ON);
            blinkingIndicators.Insert(this);
            break;
    }
}

#define MODEM_RECOVERY_INDICATOR 2
#define ROUTER_RECOVERY_INDICATOR 4
#define ULOCK_INDICATOR 21
#define OPERATIONAL_INDICATOR 22

#define MODEM_RECOVERY_INDICATOR_LED_CH 0
#define ROUTER_RECOVERY_INDICATOR_LED_CH 1
#define UNLOCK_INDICATOR_LED_CH 2
#define OPERATIONAL_INDICATOR_LED_CH 3

Indicator mri(MODEM_RECOVERY_INDICATOR_LED_CH, MODEM_RECOVERY_INDICATOR);
Indicator rri(ROUTER_RECOVERY_INDICATOR_LED_CH, ROUTER_RECOVERY_INDICATOR);
Indicator opi(OPERATIONAL_INDICATOR_LED_CH, OPERATIONAL_INDICATOR);
Indicator uli(UNLOCK_INDICATOR_LED_CH, ULOCK_INDICATOR);
