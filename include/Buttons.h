#ifndef Buttons_h
#define Buttons_h

#include <Arduino.h>
#include <Common.h>
#include <Observers.h>

enum class ButtonState
{
    UNPRESSED,
    PRESSED
};

class Button
{
public:
    Button(uint8_t pin);
    ButtonState state();
    bool initStateChangedMonitor(xSemaphoreHandle sem);

private:
    const uint8_t pin;

private:
  static void IRAM_ATTR isr(void *param);
};

class ButtonStateChangedParam
{
};

class Buttons
{
public:
  Buttons() : semButtonStateChanged(NULL) {}

  bool init(Button buttons[], size_t nButtons);
  
public:
  Observers<ButtonStateChangedParam> stateChanged;

private:
  xSemaphoreHandle semButtonStateChanged;
};

extern Button mr;
extern Button rr;
extern Button ul;
extern Button cc;
extern Buttons buttons;

bool initButtons();

#endif // Buttons_h