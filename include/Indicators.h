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
    LED_ON,     // LED is on
    LED_IDLE,   // LED is idle, on in low intensity
    LED_OFF,    // LED is off
    LED_BLINK   // LED is blinking
};

class Indicator
{
public:
    /// @brief Constructor for the Indicator class.
    /// @param _channel The PWM channel number for the indicator.
    /// @param pin The pin number for the indicator.    
    Indicator(uint8_t _channel, uint8_t pin);
    /// @brief Set the state of the indicator.
    /// @param state The state to set the indicator to.
    void set(ledState state);
    /// @brief Get the current state of the indicator.
    /// @return The current state of the indicator.
    ledState get() { return currState; }

private:
    typedef LinkedList<Indicator *> IndicatorsList; 
    /// @brief List of indicators that are blinking.
    /// This is a static member that holds all indicators that are set to blink.
    /// The blinker task will iterate through this list to toggle the state of each indicator.
    static IndicatorsList blinkingIndicators;
    /// @brief Task handle for the blinker task.
    /// This is a static member that holds the task handle for the blinker task.
    /// The blinker task is responsible for toggling the state of blinking indicators at a regular interval.
    static TaskHandle_t blinkerTaskHandle;

private:
    /// @brief Function to toggle the state of a blinking indicator. It will turn the LED on if it is off, and off if it is on.
    void Blink();
    /// @brief Internal function to set the state of the indicator.
    /// @param state The state to set the indicator to.
    /// It handles the actual LED control using PWM.
    void setInternal(ledState state);

private:
    /// @brief The PWM channel number for the indicator.
    const uint8_t channel;
    /// @brief The current state of the indicator.
    ledState currState;
};

extern Indicator mri; // Global indicator for modem recovery.
extern Indicator rri; // Global indicator for router recovery.
extern Indicator opi; // Global indicator for operational state (internet connectivity state).
extern Indicator uli; // Global indicator for unlock state.


#endif // Indicators_h