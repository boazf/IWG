/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <Arduino.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <HTTPServer.h>
#include <HttpContollersUtil.h>
#include <ControllerUtil.h>
#include <SSEController.h>
#include <Relays.h>
#include <esp_task_wdt.h>
#include <Indicators.h>
#include <GWConnTest.h>
#include <PwrCntl.h>
#include <Buttons.h>
#include <SystemUtil.h>

/// @brief Sets indicators to reflect initialization progress.
/// @param last If true, indicates the last step of initialization.
static void initProgress(bool last = false)
{
  // Set the initial state of the indicators
  static int state = 0;
  // Array of indicator objects
  static Indicator *indicators[] = { &opi, &rri, &uli, &mri };

  if (state > 0)
    // Turn on the LED to indicate initialization stage done
    indicators[state - 1]->set(ledState::LED_ON);

  if (!last && state < NELEMS(indicators))
  {
    // Blink the next LED to indicate current initialization stage in progress
    indicators[state++]->set(ledState::LED_BLINK);
  }
}

#define CHECK_FACTORY_RESET_TIMEOUT 10000
#define KEY_PRESS_VALIDATION_TIME 1000

static void checkFactoryReset()
{
  if (cc.state() == ButtonState::UNPRESSED)
    return;
  unsigned long t0 = millis();
  unsigned long tPressed;
  enum state 
  { 
    firstButtonPressed, 
    firstButtonUnpressed, 
    secondButtonPressed, 
    secondButtonUnpressed, 
    thirdButtonPressed, 
    thirdButtonUnpressed 
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
        if (cc.state() == ButtonState::UNPRESSED)
        {
          tPressed = 0;
          currentState = firstButtonUnpressed;
        }
        break;
      case firstButtonUnpressed:
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
        if (rr.state() == ButtonState::UNPRESSED)
        {
          tPressed = 0;
          currentState = secondButtonUnpressed;
        }
        break;
      case secondButtonUnpressed:
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
        if (ul.state() == ButtonState::UNPRESSED)
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

void setup() {
  InitSerialTrace();
  initProgress();
  esp_task_wdt_init(30, true);
  InitPowerControl();
  InitSD();
  InitConfig();
  InitAppConfig();
  checkFactoryReset();
  InitRelays();
#ifndef USE_WIFI
  // Wait for router initialization time.
  // Some routers dors not function properly soon after startup.
  long tWait = Config::routerInitTimeSec * 1000 - millis();
  if (tWait > 0)
    delay(tWait);
#endif
  initProgress();
  InitEthernet();
  initProgress();
  WaitForDNS();
  initProgress();
  InitTime();
  initProgress(true);
  InitFileTrace();
  InitControllers();
  InitHttpControllers();
  InitHTTPServer();
  hardResetEvent.addObserver([](const HardResetEventParam &param, void *context)
  {
      switch (param.stage)
      {
          case HardResetStage::prepare:
              // Stop accepting new connections when preparing for hard reset
              HTTPServer::stop();
              break;
          case HardResetStage::failure:
              // Allow the server to continue accepting new connections after a hard reset failure
              HTTPServer::restart();
              break;
      }
  }, NULL);
  initButtons();
}

void loop() 
{
  if (gwConnTest.IsConnected())
  {
    static unsigned long tLastMaintain = 0;
    if (millis() - tLastMaintain >= 10000)
    {
      MaintainEthernet();
      tLastMaintain = millis();
    }
    DoHTTPService();
  }
  else
  {
    MaintainEthernet();
  }
  delay(1);
}

CriticalSection csSpi;
