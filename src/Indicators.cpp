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

#include <Indicators.h>
#include <Trace.h>

#define LED_FREQ 5000
#define LED_RESOLUTION 5

#define LED_ON_DUTY 32
#define LED_OFF_DUTY 0
#define LED_IDLE_DUTY 1

#define BLINK_FREQ 2 // Hz

Indicator::IndicatorsList Indicator::blinkingIndicators;
TaskHandle_t Indicator::blinkerTaskHandle = NULL;

Indicator::Indicator(uint8_t _channel, uint8_t pin) : 
    channel(_channel)
{
    ledcSetup(channel, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(pin, channel);
    set(ledState::LED_OFF);
    if (blinkerTaskHandle == NULL)
    {
        xTaskCreate([](void *param)
        {
            while(true)
            {
                blinkingIndicators.ScanNodes([](Indicator *const &indicator, const void *param)->bool
                {
                    indicator->Blink();
                    return true;
                }, NULL);
                vTaskDelay(1000 / BLINK_FREQ / 2 / portTICK_PERIOD_MS);
            }
        }, "Blinker", 2*1024, NULL, tskIDLE_PRIORITY, &blinkerTaskHandle);
    }
}

void Indicator::Blink()
{
    ledcWrite(channel, ledcRead(channel) == LED_ON_DUTY ? LED_OFF_DUTY : LED_ON_DUTY);
}

void Indicator::setInternal(ledState state)
{
    switch(state)
    {
        case ledState::LED_ON:
            ledcWrite(channel, LED_ON_DUTY);
            break;
        case ledState::LED_IDLE:
            ledcWrite(channel, LED_IDLE_DUTY);
            break;
        case ledState::LED_OFF:
            ledcWrite(channel, LED_OFF_DUTY);
            break;
        case ledState::LED_BLINK:
            break;
    }
}

void Indicator::set(ledState state)
{
    if (currState == ledState::LED_BLINK)
    {
        blinkingIndicators.Delete(this);
    }

    currState = state;
    
    switch(state)
    {
        case ledState::LED_ON:
        case ledState::LED_IDLE:
        case ledState::LED_OFF:
            setInternal(state);
            break;
        case ledState::LED_BLINK:
            setInternal(ledState::LED_ON);
            blinkingIndicators.Insert(this);
            break;
    }
}

#define MODEM_RECOVERY_INDICATOR 4
#define ROUTER_RECOVERY_INDICATOR 2
#define ULOCK_INDICATOR 21
#define OPERATIONAL_INDICATOR 22

#define MODEM_RECOVERY_INDICATOR_LED_CH 0
#define ROUTER_RECOVERY_INDICATOR_LED_CH 1
#define UNLOCK_INDICATOR_LED_CH 2
#define OPERATIONAL_INDICATOR_LED_CH 3

Indicator mri(MODEM_RECOVERY_INDICATOR_LED_CH, MODEM_RECOVERY_INDICATOR);
Indicator rri(ROUTER_RECOVERY_INDICATOR_LED_CH, ROUTER_RECOVERY_INDICATOR);
Indicator opi(OPERATIONAL_INDICATOR_LED_CH, OPERATIONAL_INDICATOR);
Indicator uli(UNLOCK_INDICATOR_LED_CH, ULOCK_INDICATOR);
