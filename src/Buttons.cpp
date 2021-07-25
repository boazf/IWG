#include <Buttons.h>

Button::Button(uint8_t _pin) : pin(_pin)
{
  pinMode(pin, INPUT);
}

buttonState Button::state()
{
    return digitalRead(pin) ? BUTTON_OFF : BUTTON_ON;
}

#define MODEM_RECOVERY_BUTTON 34
#define ROUTER_RECOVERY_BUTTON 35
#define UNLOCK_BUTTON 36
#define CHECK_CONNECTIVITY_BUTTON 39

Button mr(MODEM_RECOVERY_BUTTON);
Button rr(ROUTER_RECOVERY_BUTTON);
Button ul(UNLOCK_BUTTON);
Button cc(CHECK_CONNECTIVITY_BUTTON);
