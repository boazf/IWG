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

struct IsrParam
{
  IsrParam() : IsrParam(0, -1, NULL) {}
  IsrParam(uint8_t pin, int lastLevel, xSemaphoreHandle sem) : pin(pin), lastLevel(lastLevel), lastTime(0), sem(sem) {}
  uint8_t pin;
  int lastLevel;
  int64_t lastTime;
  xSemaphoreHandle sem;
};

bool Button::initStateChangedMonitor(xSemaphoreHandle sem)
{
  static bool isrServiceInstalled = false;
  if (!isrServiceInstalled)
  {
    if (gpio_install_isr_service(ESP_INTR_FLAG_IRAM) != ESP_OK)
      return false;
    isrServiceInstalled = true;
  }

  gpio_config_t config =
  {
    .pin_bit_mask = 1ULL << pin,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_ANYEDGE
  };

  IsrParam *isrParam = new IsrParam(pin, digitalRead(pin), sem);

  if (gpio_config(&config) != ESP_OK ||
      gpio_isr_handler_add((gpio_num_t)pin, isr, isrParam) != ESP_OK)
    return false;

  return true;
}

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

  int64_t t = esp_timer_get_time();
  if (t - isrParam->lastTime < 150000)
  {
    isrParam->lastTime = t;
    return;
  }
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
  semButtonStateChanged = xSemaphoreCreateBinary();
  if (semButtonStateChanged == NULL)
    return false;

  for (size_t i = 0; i < nButtons; i++)
    if (!buttons[i].initStateChangedMonitor(semButtonStateChanged))
      return false;

  if (xTaskCreate(
        [](void *param)
        {
          Buttons *buttons = static_cast<Buttons *>(param);
          do
          {
            xSemaphoreTake(buttons->semButtonStateChanged, portMAX_DELAY);
            buttons->stateChanged.callObservers(ButtonStateChangedParam());
          } while(true);
        }, 
        "ButtonsStateChangeHandlerTask", 
        2 * 1024, 
        this, 
        tskIDLE_PRIORITY, NULL) != pdPASS)
    return false;

  return true;
}


#define MODEM_RECOVERY_BUTTON 35
#define ROUTER_RECOVERY_BUTTON 34
#define UNLOCK_BUTTON 36
#define CHECK_CONNECTIVITY_BUTTON 39

Button mr(MODEM_RECOVERY_BUTTON);
Button rr(ROUTER_RECOVERY_BUTTON);
Button ul(UNLOCK_BUTTON);
Button cc(CHECK_CONNECTIVITY_BUTTON);

Buttons buttons;

bool initButtons()
{
  Button buttonsArr[] = {mr, rr, ul, cc};
  return buttons.init(buttonsArr, NELEMS(buttonsArr));
}