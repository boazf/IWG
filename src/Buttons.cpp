#include <Buttons.h>
#include <Config.h>

Button::Button(uint8_t _pin) : 
  pin(_pin), 
  disabled(false)
{
  pinMode(pin, INPUT);
}

buttonState Button::state()
{
    if (disabled)
      return buttonState::BUTTON_OFF;

    return digitalRead(pin) ? buttonState::BUTTON_OFF : buttonState::BUTTON_ON;
}

void Button::disable(bool on)
{
    disabled = on;
}

void Button::setPin(uint8_t _pin)
{
    pin = _pin;
    pinMode(pin, INPUT);
}

#define MODEM_RECOVERY_BUTTON 34
#define ROUTER_RECOVERY_BUTTON 35
#define UNLOCK_BUTTON 36
#define CHECK_CONNECTIVITY_BUTTON 39

Button mr(MODEM_RECOVERY_BUTTON);
Button rr(ROUTER_RECOVERY_BUTTON);
Button ul(UNLOCK_BUTTON);
Button cc(CHECK_CONNECTIVITY_BUTTON);

bool InitButtons()
{
    if (!Config::singleDevice)
        return true;

    mr.disable();
    rr.setPin(MODEM_RECOVERY_BUTTON);

    return true;
}