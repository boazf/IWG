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

/// @brief This enum defines the possible states of a button.
/// It can be either UNPRESSED or PRESSED.
enum class ButtonState
{
    UNPRESSED,
    PRESSED
};

/// @brief This class represents a button and provides methods to read its state.
/// It also provides a method to initialize a state change monitor using a semaphore.
class Button
{
public:
    /// @brief Constructor for the Button class.
    /// @param pin The GPIO pin number to which the button is connected.
    /// The pin should be configured as an input with pull-up or pull-down resistor as needed
    Button(uint8_t pin);
    /// @brief Retrieves the current state of the button
    /// @return The current state of the button as a ButtonState enum value
    ButtonState state();
    /// @brief Installs an interrupt service routine (ISR) to monitor state changes of the button.
    /// This method sets up the GPIO pin for the button and registers an ISR that will be called
    /// whenever the button state changes (pressed or released).
    /// @param sem The semaphore to signal when the button state changes.
    /// The ISR will give this semaphore to notify that the button state has changed.
    /// @return True if the ISR was successfully installed, false otherwise.
    /// The ISR will be called in the IRAM, so it should be fast and not block.
    /// The ISR will also filter out false interrupts that may occur due to electrical noise or other reasons.
    /// @note This method should be called only once for each button.
    bool initStateChangedMonitor(xSemaphoreHandle sem);

private:
    /// @brief The GPIO pin number to which the button is connected.
    const uint8_t pin;

private:
  /// @brief The interrupt service routine (ISR) that is called when the button state changes.
  /// @param param A pointer to the ISR parameter, which is an instance of IsrParam.
  /// The ISR will read the current state of the button and give the semaphore to notify that the button state has changed.
  /// The ISR will also filter out false interrupts that may occur due to electrical noise or other reasons.`
  static void IRAM_ATTR isr(void *param);
};

/// @brief This class is used to notify observers when the button state changes.
/// It does not contain any data, but serves as a parameter for the observers.
/// The observer should then poll all buttons to retrieve their current state.
/// This works for buttons that are connected to an interrupt service routine (ISR).
class ButtonStateChangedParam
{
};

/// @brief This class manages multiple buttons and provides a way to monitor their state changes.
class Buttons
{
public:
  Buttons() : semButtonStateChanged(NULL) {}

  /// @brief This method initializes the buttons with the provided array of Button objects.
  /// It installs each button with an ISR that will give a semaphore when either button is pressed or released.
  /// Then it creates a task that is waiting for the semaphore to be given,
  /// and when it is given, it calls the observers with a ButtonStateChangedParam.
  /// @param buttons An array of Button objects to be monitored for state changes.
  /// @param nButtons The number of buttons in the array.
  /// @return True if the monitoring of the buttons was successfully initialized, false otherwise.
  bool init(Button buttons[], size_t nButtons);
  
public:
  /// @brief This observer object calls the enlisted observers when the state of any button changes.
  Observers<ButtonStateChangedParam> stateChanged;

private:
  /// @brief This semaphore is used to signal when the state of any button has changed.
  /// The ISR will give this semaphore when a button state changes, and the task will take it to notify the observers.
  /// The task will block on this semaphore until it is given, and then it will call the observers.
  /// This semaphore is created in the init() method and should not be deleted until the program ends.
  xSemaphoreHandle semButtonStateChanged;
};

/// @brief Global instance of Modem Recovery button
extern Button mr;
/// @brief Global instance of Router Recovery button
extern Button rr;
/// @brief Global instance of Unlock button
extern Button ul;
/// @brief Global instance of Connectivity Check button
extern Button cc;
/// @brief Global instance of the Buttons class that manages all buttons.
extern Buttons buttons;

/// @brief Initializes the buttons.
/// @return True if the buttons monitoring successfully initialized, false otherwise.
bool initButtons();

#endif // Buttons_h