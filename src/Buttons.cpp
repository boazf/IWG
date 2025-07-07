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

#include <Buttons.h>
#include <driver/adc.h>

Button::Button(uint8_t _pin) : pin(_pin) 
{
  pinMode(pin, INPUT);
}

/// @brief This structure holds the parameters for the ISR that monitors button state changes.
struct IsrParam
{
  IsrParam() : IsrParam(0, -1, NULL) {}
  /// @brief Constructor for the IsrParam structure.
  /// @param pin The GPIO pin number of the last button that its state was changed
  /// @param lastLevel The last level of the pin (0 or 1) before the state change
  /// @param lastTime The last time the state was changed, in microseconds
  /// @param sem The semaphore to signal when the button state changes
  IsrParam(uint8_t pin, int lastLevel, xSemaphoreHandle sem) : pin(pin), lastLevel(lastLevel), lastTime(0), sem(sem) {}
  uint8_t pin;
  int lastLevel;
  int64_t lastTime;
  xSemaphoreHandle sem;
};

/// @brief Initializes the state change monitor (ISR) for the button.
/// @param sem The semaphore to signal when the button state changes.
/// The ISR will give this semaphore to notify that the button state has changed.
/// @return True if the ISR was successfully installed, false otherwise.
bool Button::initStateChangedMonitor(xSemaphoreHandle sem)
{
  // The ISR should be installed only once for all buttons. This code is not thread safe but it is called only from the setup() function
  // which is called only once at the beginning of the program.
  static bool isrServiceInstalled = false;
  if (!isrServiceInstalled)
  {
    if (gpio_install_isr_service(ESP_INTR_FLAG_IRAM) != ESP_OK)
      return false;
    isrServiceInstalled = true;
  }

  // Configure the GPIO pin for input with pull-up or pull-down resistor as needed
  // The pullup and pulldown resistors are disabled because there are pullup resistors on the buttons themselves.
  gpio_config_t config =
  {
    .pin_bit_mask = 1ULL << pin,              // Use a bit mask to specify the pin
    .mode = GPIO_MODE_INPUT,                  // Set the pin mode to input      
    .pull_up_en = GPIO_PULLUP_DISABLE,        // Disable pull-up resistor
    .pull_down_en = GPIO_PULLDOWN_DISABLE,    // Disable pull-down resistor
    .intr_type = GPIO_INTR_ANYEDGE            // Set the interrupt type to trigger on any edge (rising or falling)
  };

  // Create an instance of IsrParam to hold the pin number, last level, last time and semaphore
  IsrParam *isrParam = new IsrParam(pin, digitalRead(pin), sem);

  // Register the ISR handler for the button pin
  if (gpio_config(&config) != ESP_OK ||
      gpio_isr_handler_add((gpio_num_t)pin, isr, isrParam) != ESP_OK)
    return false;

  return true;
}

/// @brief The ISR that is called when the button state changes.
/// It reads the current state of the button and gives the semaphore to notify that the button state has changed.
/// It also filters out false interrupts that may occur due to electrical noise or other reasons.
void IRAM_ATTR Button::isr(void *param)
{
  IsrParam *isrParam = static_cast<IsrParam *>(param);
  // When using WiFi, pins 36 and 39 are geting "false" interrupts because (probably)
  // ADC1 usage by WiFi hardware. According to ChatGPT: Wi-Fi uses ADC1 for:
  // Signal strength measurement (RSSI calibration)
  // Temperature or RF calibration
  // Other internal purposes
  // So, in order to filter those false interrupts, we look for actual change in pin
  // state before triggering the semaphore.
  // This logic is not needed in wired configuration, but it doesn't interfere.
  // Also this code is not needed for pins 34 and 35, but it doesn't interfere.
  int level = gpio_get_level(static_cast<gpio_num_t>(isrParam->pin));
  if (level == isrParam->lastLevel)
    return;

  // Filter out false interrupts that may occur due to electrical noise or other reasons.
  // We use a time threshold of 150 milliseconds to filter out false interrupts.
  int64_t t = esp_timer_get_time();
  if (t - isrParam->lastTime < 150000)
  {
    isrParam->lastTime = t;
    return;
  }
  // Update the last time and level
  // and give the semaphore to notify that the button state has changed.
  isrParam->lastTime = t;
  isrParam->lastLevel = level;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(isrParam->sem, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

ButtonState Button::state()
{
    return digitalRead(pin) ? ButtonState::UNPRESSED : ButtonState::PRESSED;
}

bool Buttons::init(Button buttons[], size_t nButtons)
{
  // Create a binary semaphore to signal when the state of any button has changed.
  semButtonStateChanged = xSemaphoreCreateBinary();
  if (semButtonStateChanged == NULL)
    return false;

  // Install the ISR for each button.
  for (size_t i = 0; i < nButtons; i++)
    if (!buttons[i].initStateChangedMonitor(semButtonStateChanged))
      return false;

  // Create a task that will wait for the semaphore to be given.
  // When the semaphore is given, it will call the observers with a ButtonStateChangedParam
  if (xTaskCreate(
        [](void *param)
        {
          Buttons *buttons = static_cast<Buttons *>(param);
          do
          {
            // Wait for the semaphore to be given
            xSemaphoreTake(buttons->semButtonStateChanged, portMAX_DELAY);
            // Call the observers with a ButtonStateChangedParam
            buttons->stateChanged.callObservers(ButtonStateChangedParam());
          } while(true);
        }, 
        "ButtonsStateChangeHandlerTask", 
        2 * 1024, // Stack size for the task
        this, // Pass the Buttons instance as a parameter
        tskIDLE_PRIORITY, // Task priority
        NULL // Task handle (not used, so NULL is passed)
      ) != pdPASS)
    return false;

  return true;
}


#define MODEM_RECOVERY_BUTTON 35 // GPIO35 is used for Modem Recovery button
#define ROUTER_RECOVERY_BUTTON 34 // GPIO34 is used for Router Recovery button
#define UNLOCK_BUTTON 36 // GPIO36 is used for Unlock button
#define CHECK_CONNECTIVITY_BUTTON 39 // GPIO39 is used for Connectivity Check button

Button mr(MODEM_RECOVERY_BUTTON); // Modem Recovery button
Button rr(ROUTER_RECOVERY_BUTTON); // Router Recovery button
Button ul(UNLOCK_BUTTON); // Unlock button
Button cc(CHECK_CONNECTIVITY_BUTTON); // Connectivity Check button

Buttons buttons;

bool initButtons()
{
  Button buttonsArr[] = {mr, rr, ul, cc};
  return buttons.init(buttonsArr, NELEMS(buttonsArr));
}