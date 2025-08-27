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
    // Forward declarations of states
    class ConnectivityCheck; 
    class Disconnected;
    class RecoveryFailure;
    class ModemRecovery;
    class Connected;
    class RouterRecovery;
    class PeriodicRestart;
    class Unlock;

    /// @brief Common base class for manual control states.
    /// This class provides common functionality for manual control states, such as handling recovery state changes and button presses.
    /// It is used to avoid code duplication in the state implementations.
    class CommonManualControlState : public ManualControl
    {
    public:
        /// @brief Constructor for the CommonManualControlState class.
        /// @param isRecovery Indicates whether the state is in recovery mode.
        /// If true, the state will handle recovery actions, otherwise it will handle normal operations.
        CommonManualControlState(bool isRecovery = false) : isRecovery(isRecovery) {}

        void entry() override
        {
            if (!isRecovery)
            {
                // Set all the indicators to idle state when not in recovery
                opi.set(ledState::LED_IDLE);
                uli.set(ledState::LED_IDLE);
                mri.set(ledState::LED_IDLE);
                rri.set(ledState::LED_IDLE);
            }
            else
            {
                // Set all the indicators to off state when in recovery
                opi.set(ledState::LED_OFF);
                uli.set(ledState::LED_OFF);
                mri.set(ledState::LED_OFF);
                rri.set(ledState::LED_OFF);
            }
        }

        /// @brief Reacts to the RecoveryStateChanged event.
        /// This method handles the recovery state changes and transitions to the appropriate state based on the recovery type.
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

        /// @brief Handles button presses and releases.
        /// This method checks the state of the buttons and performs actions based on the button states.
        /// It is used to handle manual control actions such as unlocking the system or starting recovery cycles.
        /// @param isConnected Indicates whether the system is connected to the internet.
        void doButtons(bool isConnected)
        {
            CommonManualControlState::isConnected = isConnected;

            if (ul.state() == ButtonState::PRESSED && 
                rr.state() == ButtonState::UNPRESSED && 
                mr.state() == ButtonState::UNPRESSED && 
                cc.state() == ButtonState::UNPRESSED)
            {
                // If only the unlock button is pressed, transition to the Unlock state
                transit<Unlock>();
                return;
            }

            if (cc.state() == ButtonState::PRESSED && 
                ul.state() == ButtonState::UNPRESSED && 
                rr.state() == ButtonState::UNPRESSED && 
                mr.state() == ButtonState::UNPRESSED)
            {
                // If only the connectivity check button is pressed, start the connectivity check
                recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
            }
        }

    protected:
        bool isRecovery;
        static bool isConnected;
    };

    /// @brief Static variable to indicate if the system is connected to the internet.
    bool CommonManualControlState::isConnected;

    /// @brief Class representing the connected state.
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
            // Handle button presses in the connected state
            doButtons(true);
        }
    };

    /// @brief Class representing the periodic restart state.
    class PeriodicRestart : public CommonManualControlState
    {
    public:
        /// @brief Constructor for the PeriodicRestart state.
        /// This state is used for periodic restarts of the router and/or modem.
        /// @note The constructor sets the isRecovery flag to true to indicate that this state is in recovery mode.
        PeriodicRestart() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            if (AppConfig::getPeriodicallyRestartRouter())
                // if periodic restart is enabled for the router, set the router recovery indicator to blinking
                rri.set(ledState::LED_BLINK);
            if (AppConfig::getPeriodicallyRestartModem())
                // if periodic restart is enabled for the modem, set the modem recovery indicator to blinking
                mri.set(ledState::LED_BLINK);
        }
    };

    /// @brief Class representing the router recovery state.
    class RouterRecovery : public CommonManualControlState
    {
    public:
        /// @brief Constructor for the RouterRecovery state.
        /// This state is used for recovering the internet connection by disconnecting and reconnecting the router.
        /// @note The constructor sets the isRecovery flag to true to indicate that this state is in recovery mode.
        RouterRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            rri.set(ledState::LED_BLINK);
        }
    };

    /// @brief Class representing the modem recovery state.
    class ModemRecovery : public CommonManualControlState
    {
    public:
        /// @brief Constructor for the ModemRecovery state.
        /// This state is used for recovering the internet connection by disconnecting and reconnecting the modem.
        /// @note The constructor sets the isRecovery flag to true to indicate that this state is in recovery mode.
        ModemRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            mri.set(ledState::LED_BLINK);
        }
    };

    /// @brief Class representing the recovery failure state.
    class RecoveryFailure : public CommonManualControlState
    {
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_OFF);
        }
    
        void react(ButtonsStateChanged const &event) override
        {
            // Handle button presses in the recovery failure state
            doButtons(false);
        }
    };

    /// @brief Class representing the connectivity check state.
    class ConnectivityCheck : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_BLINK);
        }
    };

    /// @brief Class representing the unlock state.
    /// This state allows the user to unlock the router and modem recovery buttons and perform manual recovery actions.
    class Unlock : public CommonManualControlState
    {
    public:
        /// @brief  Event to trigger the unlock delay.
        struct UnlockDelay : tinyfsm::Event {};

    public:
        void entry() override
        {
            // Initialize the unlock state to false.
            // The user should hold the unlock button for 1 second to unlock the router and modem recovery buttons.
            unlocked = false;
            ledState opiState = opi.get();
            CommonManualControlState::entry();
            // Restore the operational indicator state to the previous state. This should be done after the entry actions.
            opi.set(opiState);
            // Turn off the unlock indicator as a feedback to the user that the unlock button press was detected.
            uli.set(ledState::LED_OFF);
            // Create a timer to wait for the unlock button to be pressed for 1 second.
            // If the unlock button is pressed for 1 second, the system will be unlocked and the modem and router recovery buttons will be enabled.
            // If the unlock button is released before 1 second, the system will return to the previous state.
            esp_timer_create_args_t args = {onTimer, this, ESP_TIMER_TASK, "UnlockTimer"};
            esp_timer_create(&args, &hTimer);
            esp_timer_start_once(hTimer, 1000000);
        }

        /// @brief Reacts to timer events.
        void react(const UnlockDelay &event)
        {
            if (unlocked)
            {
                // If the system is already unlocked, return to the previous state.
                // This means that the user did not press any recovery button during the 3 seconds unlock period.
                returnToPrevState();
                return;
            }

            // If the unlock button was pressed for 1 second, enable the modem and router recovery buttons.
            unlocked = true;
            // Turm on the unlock, modem recovery and router recovery indicators to indicate that the system is unlocked.
            mri.set(ledState::LED_ON);
            rri.set(ledState::LED_ON);
            uli.set(ledState::LED_ON);
            // Start the timer to wait for the user to press the modem or router recovery buttons.
            // If the user does not press any recovery button for 3 seconds, the system will return to the previous state.
            esp_timer_start_once(hTimer, 3000000);
        }

        /// @brief Reacts to button state changes.
        void react(ButtonsStateChanged const &event) override
        {
            if (!unlocked)
            {
                // If the system is not unlocked, and not only the unlock button is pressed, return to the previous state.
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
                // If only the router recovery button is pressed, start the router recovery cycles.
                recoveryControl.StartRecoveryCycles(RecoveryTypes::Router);
            }
            else if (ul.state() == ButtonState::UNPRESSED && 
                     mr.state() == ButtonState::PRESSED && 
                     rr.state() == ButtonState::UNPRESSED && 
                     cc.state() == ButtonState::UNPRESSED)
            {
                // If only the modem recovery button is pressed, start the modem recovery cycles.
                recoveryControl.StartRecoveryCycles(RecoveryTypes::Modem);        
            }
            else if (cc.state() == ButtonState::PRESSED && 
                     ul.state() == ButtonState::UNPRESSED && 
                     mr.state() == ButtonState::UNPRESSED && 
                     rr.state() == ButtonState::UNPRESSED)
            {
                // If only the connectivity check button is pressed, start hard reset.
                esp_timer_stop(hTimer);
                opi.set(ledState::LED_BLINK);
                uli.set(ledState::LED_OFF);
                rri.set(ledState::LED_OFF);
                mri.set(ledState::LED_OFF);
                HardReset(3000, 15000);
                returnToPrevState();
            }
        }

        void exit() override
        {
            // When exiting the unlock state, stop the timer and delete it.
            // This is important to avoid memory leaks and ensure that the timer does not trigger after the state has changed.
            esp_timer_stop(hTimer);
            esp_timer_delete(hTimer);
        }

    private:
        esp_timer_handle_t hTimer;
        bool unlocked;

    private:
        /// @brief Static function to handle the timer event.
        static void onTimer(void *arg)
        {
            static_cast<Unlock *>(arg)->react(UnlockDelay());
        }

        /// @brief Returns to the previous state based on the current connection status.
        /// If the system is connected, it transitions to the Connected state.
        /// If the system is not connected, it transitions to the RecoveryFailure state.
        void returnToPrevState()
        {
            if (isConnected)
                transit<Connected>();
            else
                transit<RecoveryFailure>();
        }
    };

}

manualcontrol::ManualControl manualControl;

FSM_INITIAL_STATE(manualcontrol::ManualControl, manualcontrol::ConnectivityCheck)
