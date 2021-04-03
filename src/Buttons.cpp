#include <Buttons.h>

Button::Button(uint8_t _pin) : pin(_pin)
{
  pinMode(pin, PULLUP);
}

buttonState Button::state()
{
    return digitalRead(pin) ? BUTTON_OFF : BUTTON_ON;
}

#define MODEM_RECOVERY_BUTTON 22
#define ROUTER_RECOVERY_BUTTON 27
#define UNLOCK_BUTTON 21
#define CHECK_CONNECTIVITY_BUTTON 15

Button mr(MODEM_RECOVERY_BUTTON);
Button rr(ROUTER_RECOVERY_BUTTON);
Button ul(UNLOCK_BUTTON);
Button cc(CHECK_CONNECTIVITY_BUTTON);