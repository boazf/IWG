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