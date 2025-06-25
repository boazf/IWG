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