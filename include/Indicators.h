#ifndef Indicators_h
#define Indicators_h

#include <Arduino.h>
#include <LinkedList.h>

enum class ledState
{
    LED_ON,
    LED_IDLE,
    LED_OFF,
    LED_BLINK
};

class Indicator
{
public:
    Indicator(uint8_t _channel, uint8_t pin);
    void set(ledState state);
    ledState get() { return currState; }

private:
    typedef LinkedList<Indicator *> IndicatorsList; 
    static IndicatorsList blinkingIndicators;
    static TaskHandle_t blinkerTaskHandle;

private:
    void Blink();
    void setInternal(ledState state);

private:
    const uint8_t channel;
    ledState currState;
};

extern Indicator mri;
extern Indicator rri;
extern Indicator opi;
extern Indicator uli;


#endif // Indicators_h