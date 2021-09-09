#ifndef Buttons_h
#define Buttons_h

#include <Arduino.h>

enum class buttonState
{
    BUTTON_OFF,
    BUTTON_ON
};

class Button
{
public:
    Button(uint8_t pin);
    buttonState state();

private:
    const uint8_t pin;
};

extern Button mr;
extern Button rr;
extern Button ul;
extern Button cc;

#endif // Buttons_h