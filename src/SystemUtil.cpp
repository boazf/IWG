#include "Common.h"
#include "SystemUtil.h"
#include "AppConfig.h"
#include "HistoryControl.h"
#include "Buttons.h"
#include "Indicators.h"

#define CHECK_FACTORY_RESET_TIMEOUT 10000
#define KEY_PRESS_VALIDATION_TIME 1000

void checkFactoryResetOnStartup()
{
  if (cc.state() == ButtonState::RELEASED)
    return;
  unsigned long t0 = millis();
  unsigned long tPressed;
  enum state 
  { 
    firstButtonPressed, 
    firstButtonReleased, 
    secondButtonPressed, 
    secondButtonReleased, 
    thirdButtonPressed, 
    thirdButtonReleased 
  };
  state currentState = firstButtonPressed;
  ledState opiState = opi.get();
  ledState rriState = rri.get();
  ledState uliState = uli.get();
  opi.set(ledState::LED_ON);
  bool stop = false;
  while (!stop && millis() - t0 <= CHECK_FACTORY_RESET_TIMEOUT) // Wait 10 seconds for factory reset confirmation.
  {
    switch (currentState)
    {
      case firstButtonPressed:
        if (cc.state() == ButtonState::RELEASED)
        {
          tPressed = 0;
          currentState = firstButtonReleased;
        }
        break;
      case firstButtonReleased:
        if (rr.state() == ButtonState::PRESSED)
        {
          if (tPressed == 0)
          {
            tPressed = millis();
          }
          if (millis() - tPressed >= KEY_PRESS_VALIDATION_TIME)
          {
            rri.set(ledState::LED_ON);
            currentState = secondButtonPressed;
          }
        }
        else
        {
          tPressed = 0;
        }
        break;
      case secondButtonPressed:
        if (rr.state() == ButtonState::RELEASED)
        {
          tPressed = 0;
          currentState = secondButtonReleased;
        }
        break;
      case secondButtonReleased:
        if (ul.state() == ButtonState::PRESSED)
        {
          if (tPressed == 0)
          {
            tPressed = millis();
          }
          if (millis() - tPressed >= KEY_PRESS_VALIDATION_TIME)
          {
            uli.set(ledState::LED_ON);
            currentState = thirdButtonPressed;
          }
        }
        else
        {
          tPressed = 0;
        }
        break;
      case thirdButtonPressed:
        if (ul.state() == ButtonState::RELEASED)
        {
          factoryReset();
          stop = true;
        }
        break;
    }
  }
  opi.set(opiState);
  rri.set(rriState);
  uli.set(uliState);
}

void factoryReset()
{
    // Implementation of the factory reset logic goes here.
    // This should reset the system to its default state, erasing all configurations and settings.
    AppConfig::factoryReset();
    historyControl.reset();
#ifdef DEBUG_HTTP_SERVER
    Serial.println("Factory reset performed.");
#endif
}

