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

#include <ManualControl.h>
#include <Indicators.h>
#include <AppConfig.h>
#include <PwrCntl.h>

namespace manualcontrol
{
    class ConnectivityCheck; 
    class Disconnected;
    class RecoveryFailure;
    class HWFailure;
    class ModemRecovery;
    class Connected;
    class RouterRecovery;
    class PeriodicRestart;
    class Unlock;

    class CommonManualControlState : public ManualControl
    {
    public:
        CommonManualControlState(bool isRecovery = false) : isRecovery(isRecovery) {}

        void entry() override
        {
            if (!isRecovery)
            {
                opi.set(ledState::LED_IDLE);
                uli.set(ledState::LED_IDLE);
                mri.set(ledState::LED_IDLE);
                rri.set(ledState::LED_IDLE);
            }
            else
            {
                opi.set(ledState::LED_OFF);
                uli.set(ledState::LED_OFF);
                mri.set(ledState::LED_OFF);
                rri.set(ledState::LED_OFF);
            }
        }

        void react(RecoveryStateChanged const &event) override
        {
            switch(event.m_recoveryType)
            {
            case RecoveryTypes::ConnectivityCheck:
                transit<ConnectivityCheck>();

            case RecoveryTypes::Disconnected:
            case RecoveryTypes::Failed:
                transit<RecoveryFailure>();
                break;

            case RecoveryTypes::HWFailure:
                transit<HWFailure>();
                break;

            case RecoveryTypes::Modem:
                transit<ModemRecovery>();
                break;

            case RecoveryTypes::NoRecovery:
                transit<Connected>();
                break;

            case RecoveryTypes::Router:
            case RecoveryTypes::RouterSingleDevice:
                transit<RouterRecovery>();
                break;

            case RecoveryTypes::Periodic:
                transit<PeriodicRestart>();
                break;
            }
        }

        void doButtons(bool isConnected)
        {
            CommonManualControlState::isConnected = isConnected;

            if (ul.state() == ButtonState::PRESSED && 
                rr.state() == ButtonState::UNPRESSED && 
                mr.state() == ButtonState::UNPRESSED && 
                cc.state() == ButtonState::UNPRESSED)
            {
                transit<Unlock>();
                return;
            }

            if (cc.state() == ButtonState::PRESSED && 
                ul.state() == ButtonState::UNPRESSED && 
                rr.state() == ButtonState::UNPRESSED && 
                mr.state() == ButtonState::UNPRESSED)
            {
                recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
            }
        }

    protected:
        bool isRecovery;
        static bool isConnected;
    };

    bool CommonManualControlState::isConnected;

    class Connected : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_ON);
        }

        void react(ButtonsStateChanged const &event) override
        {
            doButtons(true);
        }
    };

    class PeriodicRestart : public CommonManualControlState
    {
    public:
        PeriodicRestart() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            if (AppConfig::getPeriodicallyRestartRouter())
                rri.set(ledState::LED_BLINK);
            if (AppConfig::getPeriodicallyRestartModem())
                mri.set(ledState::LED_BLINK);
        }
    };

    class RouterRecovery : public CommonManualControlState
    {
    public:
        RouterRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            rri.set(ledState::LED_BLINK);
        }
    };

    class ModemRecovery : public CommonManualControlState
    {
    public:
        ModemRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            mri.set(ledState::LED_BLINK);
        }
    };

    class HWFailure : public CommonManualControlState
    {
    public:
        HWFailure() : CommonManualControlState(true) {}
        
        void entry() override
        {
            CommonManualControlState::entry();
            mri.set(ledState::LED_BLINK);
            rri.set(ledState::LED_BLINK);
        }
    };

    class RecoveryFailure : public CommonManualControlState
    {
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_OFF);
        }
    
        void react(ButtonsStateChanged const &event) override
        {
            doButtons(false);
        }
    };

    class ConnectivityCheck : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_BLINK);
        }
    };

    class Unlock : public CommonManualControlState
    {
    public:
        struct UnlockDelay : tinyfsm::Event {};

    public:
        void entry() override
        {
            unlocked = false;
            ledState opiState = opi.get();
            CommonManualControlState::entry();
            opi.set(opiState);
            uli.set(ledState::LED_OFF);
            esp_timer_create_args_t args = {onTimer, this, ESP_TIMER_TASK, "UnlockTimer"};
            esp_timer_create(&args, &hTimer);
            esp_timer_start_once(hTimer, 1000000);
        }

        void react(const UnlockDelay &event)
        {
            if (unlocked)
            {
                returnToPrevState();
                return;
            }

            unlocked = true;
            mri.set(ledState::LED_ON);
            rri.set(ledState::LED_ON);
            uli.set(ledState::LED_ON);
            esp_timer_start_once(hTimer, 3000000);
        }

        void react(ButtonsStateChanged const &event) override
        {
            if (!unlocked)
            {
                if (ul.state() == ButtonState::UNPRESSED || 
                    mr.state() == ButtonState::PRESSED || 
                    rr.state() == ButtonState::PRESSED || 
                    cc.state() == ButtonState::PRESSED)
                    returnToPrevState();
                return;
            }

            if (ul.state() == ButtonState::UNPRESSED && 
                mr.state() == ButtonState::UNPRESSED && 
                rr.state() == ButtonState::PRESSED && 
                cc.state() == ButtonState::UNPRESSED)
            {
                recoveryControl.StartRecoveryCycles(RecoveryTypes::Router);
            }
            else if (ul.state() == ButtonState::UNPRESSED && 
                     mr.state() == ButtonState::PRESSED && 
                     rr.state() == ButtonState::UNPRESSED && 
                     cc.state() == ButtonState::UNPRESSED)
            {
                recoveryControl.StartRecoveryCycles(RecoveryTypes::Modem);        
            }
            else if (cc.state() == ButtonState::PRESSED && 
                     ul.state() == ButtonState::UNPRESSED && 
                     mr.state() == ButtonState::UNPRESSED && 
                     rr.state() == ButtonState::UNPRESSED)
            {
                esp_timer_stop(hTimer);
                opi.set(ledState::LED_BLINK);
                uli.set(ledState::LED_OFF);
                rri.set(ledState::LED_OFF);
                mri.set(ledState::LED_OFF);
                HardReset(3000);
            }
        }

        void exit() override
        {
            esp_timer_stop(hTimer);
            esp_timer_delete(hTimer);
        }

    private:
        esp_timer_handle_t hTimer;
        bool unlocked;

    private:
        static void onTimer(void *arg)
        {
            static_cast<Unlock *>(arg)->react(UnlockDelay());
        }

        void returnToPrevState()
        {
            if (isConnected)
                transit<Connected>();
            else
                transit<RecoveryFailure>();
        }
    };

    class Init : public CommonManualControlState
    {
        void entry() override
        {
            CommonManualControlState::entry();
            if (recoveryControl.GetRecoveryState() != RecoveryTypes::ConnectivityCheck)
                transit<Connected>();
        }

        void react(RecoveryStateChanged const &event) override
        {
            if (event.m_recoveryType !=  RecoveryTypes::ConnectivityCheck)
                transit<Connected>();
        }
    };
}

manualcontrol::ManualControl manualControl;

FSM_INITIAL_STATE(manualcontrol::ManualControl, manualcontrol::Init)
