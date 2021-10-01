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
    void disable(bool on = true);
    void setPin(uint8_t pin);

private:
    uint8_t pin;
    bool disabled;
};

extern Button mr;
extern Button rr;
extern Button ul;
extern Button cc;

bool InitButtons();

#endif // Buttons_h