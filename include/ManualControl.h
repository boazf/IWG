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

#ifndef ManualControl_h
#define ManualControl_h

#include <CommonFsmDefs.h>
#include <RecoveryControl.h>
#include <Buttons.h>

namespace manualcontrol
{
    #undef ON_RECOVERY_STATE_CHANGED
    #define ON_RECOVERY_STATE_CHANGED(fnName) ON_EVENT(ManualControl, RecoveryStateChangedParams, fnName)
    #undef ON_BUTTONS_STATE_CHANGED
    #define ON_BUTTONS_STATE_CHANGED(fnName) ON_EVENT(ManualControl, ButtonStateChangedParam, fnName)
    struct ButtonsStateChanged : tinyfsm::Event {};

    /// @brief Class for manual control state machine.
    /// This class defines the states and transitions for manual control of the system.
    /// It handles recovery states, button presses, and other manual control actions.
    /// The state machine is implemented using the tinyfsm library.
    class ManualControl : public tinyfsm::Fsm<ManualControl>
    {
    public:
        /* default reaction for unhandled events */
        void react(tinyfsm::Event const &) { };

        virtual void react(RecoveryStateChanged const &) {};
        virtual void react(ButtonsStateChanged const &) {};

        virtual void entry(void) { };  /* entry actions in some states */
        virtual void exit(void)  { };  /* entry actions in some states */

    public:
        // @brief Initializes the manual control state machine.
        /// This method sets up the initial state and registers observers for recovery state changes and button state changes.
        /// It should be called once at the start of the application to set up the manual control.
        void init()
        {
            tinyfsm::FsmList<ManualControl>::start();
            recoveryControl.addRecoveryStateChangedObserver(onRecoveryStateChanged, this);
            buttons.stateChanged.addObserver(onButtonsStateChanged, this);
        }

    protected:
        template<typename E>
        void send_event(E const &event)
        {
            tinyfsm::FsmList<ManualControl>::template dispatch<E>(event);
        }

    private:
        /// @brief This observer function is called when the recovery state changes.
        /// It sends a RecoveryStateChanged event to the state machine.
        /// @param param The parameters of the recovery state change, including the recovery type and source.
        ON_RECOVERY_STATE_CHANGED(onRecoveryStateChanged)
        {
            send_event(RecoveryStateChanged(param.m_recoveryType, param.m_source));
        }

        /// @brief This observer function is called when the button state changes.
        /// It sends a ButtonsStateChanged event to the state machine.
        /// @param param The parameters of the button state change.
        /// It is used to notify the state machine about button presses and releases.
        ON_BUTTONS_STATE_CHANGED(onButtonsStateChanged)
        {
            send_event(ButtonsStateChanged());
        }
    };
}

// A global instance of the ManualControl state machine.
extern manualcontrol::ManualControl manualControl;

#endif // ManualControl_h