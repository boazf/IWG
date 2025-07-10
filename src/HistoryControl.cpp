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

#include <HistoryControl.h>
#include <AppConfig.h>
#include <TimeUtil.h>

namespace historycontrol
{
    // Static members initialization
    int HistoryControl::maxHistory;
    RecoveryTypes HistoryControl::recoveryType;
    RecoverySource HistoryControl::recoverySource;
    HistoryStorage HistoryControl::storage;
    HistoryStorageItem *HistoryControl::currStorageItem = NULL;
    time_t HistoryControl::lastUpdate;

    void HistoryControl::init()
    {
        // Add recovery state change observer
        recoveryControl.GetRecoveryStateChanged().addObserver(onRecoveryStateChanged, this);
        // Get the maximum history records from the application configuration
        maxHistory = AppConfig::getMaxHistory();
        // Add observer for maximum history record changes
        recoveryControl.GetMaxHistoryRecordsChanged().addObserver(onMaxHistoryChanged, this);
        // Read the history records from storage (EEPROM)
        storage.init(maxHistory);
        // Set the last update time to the current time
        lastUpdate = t_now;
        // Start the FSM
        tinyfsm::FsmList<HistoryControl>::start();
    }

    time_t HistoryControl::getLastRecovery()
    { 
        return storage.getLastRecovery(); 
    }

    time_t HistoryControl::getLastUpdate()
    {
        return lastUpdate;
    }

    class ConnectivityCheck; 
    class RecoveryFailure;
    class HWFailure;
    class ModemRecovery;
    class Connected;
    class RouterRecovery;
    class PeriodicRestart;

    /// @brief Common base class for all states in the HistoryControl FSM.
    /// This class handles the common functionality for reacting to recovery state changes.
    class CommonHistoryControlState : public HistoryControl
    {
    public:
        void react(RecoveryStateChanged const &event) override
        {
            recoveryType = event.m_recoveryType;
            recoverySource = event.m_source;

            switch(event.m_recoveryType)
            {
            case RecoveryTypes::ConnectivityCheck:
                transit<ConnectivityCheck>();
                break;

            case RecoveryTypes::Disconnected:
                // Do nothing.
                break;

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
    };

    void HistoryControl::onRecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context)
    {
        // Cast the context to HistoryControl pointer and call the send_event method`
        HistoryControl *control = const_cast<HistoryControl *>(static_cast<const HistoryControl *>(context));
        // Trigger the FSM transition by sending the RecoveryStateChanged event
        control->send_event(RecoveryStateChanged(params.m_recoveryType, params.m_source));
    }

    void HistoryControl::onMaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context)
    {
        // Cast the context to HistoryControl pointer and update the maxHistory
        HistoryControl *control = const_cast<HistoryControl *>(static_cast<const HistoryControl *>(context));
        // Update the maximum history records
        control->maxHistory = params.m_maxRecords;
        control->storage.resize(control->maxHistory);
        control->lastUpdate = t_now;
    }

    /// @brief Init state.
    class Init : public CommonHistoryControlState
    {
    };

    /// @brief Common state for connectivity checks and recovery failures.
    class ConnectivityCheck : public CommonHistoryControlState
    {
        void entry() override
        {
            CreateHistoryItem(recoverySource);
        }
    };

    /// @brief Connectivity check state while in a failure state.
    class ConnectivityCheckWhileInFailure : public ConnectivityCheck
    {
        void exit() override
        {
            AddToHistory();
        }
    };

    /// @brief State for handling recovery failures.
    class RecoveryFailure : public CommonHistoryControlState
    {
        void entry() override
        {
            AddToHistoryStorage(RecoveryStatus::RecoveryFailure);
        }

        void react(RecoveryStateChanged const &event) override
        {
            if (event.m_recoveryType == RecoveryTypes::ConnectivityCheck)
                transit<ConnectivityCheckWhileInFailure>();
            else
                CommonHistoryControlState::react(event);
        }

        void exit() override
        {
            AddToHistory();
        }
    };

    /// @brief State for handling hardware failures.
    /// @todo This state actually never gets entered. Should remove it all together
    class HWFailure : public CommonHistoryControlState
    {
        void entry() override
        {
            delete currStorageItem;
            currStorageItem = NULL;
            lastUpdate = t_now;
        }
    };

    /// @brief State for modem recovery.
    class ModemRecovery : public CommonHistoryControlState
    {
        void entry() override
        {
            CreateHistoryItem(recoverySource);
            currStorageItem->modemRecoveries()++;
            lastUpdate = t_now;
        }
    };

    /// @brief State for when the device is connected.
    class Connected : public CommonHistoryControlState
    {
        void entry() override
        {
            if (currStorageItem != NULL)
            {
                if (currStorageItem->routerRecoveries() > 0 || currStorageItem->modemRecoveries() > 0)
                {
                    AddToHistoryStorage(RecoveryStatus::RecoverySuccess);
                }
                else
                {
                    delete currStorageItem;
                    currStorageItem = NULL;
                }
            }
        }
    };

    /// @brief State for router recovery.
    class RouterRecovery : public CommonHistoryControlState
    {
        void entry() override
        {
            CreateHistoryItem(recoverySource);
            currStorageItem->routerRecoveries()++;
            lastUpdate = t_now;
        }
    };

    /// @brief State for periodic restarts.
    class PeriodicRestart : public CommonHistoryControlState
    {
        void entry() override
        {
            CreateHistoryItem(recoverySource);
            if (AppConfig::getPeriodicallyRestartRouter())
                currStorageItem->routerRecoveries()++;
            if (AppConfig::getPeriodicallyRestartModem())
                currStorageItem->modemRecoveries()++;
            lastUpdate = t_now;
        }
    };

    void HistoryControl::AddToHistory()
    {
        if (recoveryType == RecoveryTypes::NoRecovery)
        {
            CreateHistoryItem(recoverySource);
            AddToHistoryStorage(RecoveryStatus::RecoverySuccess, false);
        }
    }

    bool HistoryControl::CreateHistoryItem(RecoverySource recoverySource)
    {
        if (currStorageItem != NULL)
            return false;

        currStorageItem = new HistoryStorageItem(recoverySource, t_now, INT32_MAX, 0, 0, RecoveryStatus::OnGoingRecovery);
        lastUpdate = t_now;
        return true;
    }

    void HistoryControl::AddToHistoryStorage(RecoveryStatus status, bool withEndTime)
    {
        currStorageItem->recoveryStatus() = status;
        if (withEndTime)
        {
            currStorageItem->endTime() = t_now;
        }
        storage.addHistory(*currStorageItem);
        delete currStorageItem;
        currStorageItem = NULL;
        lastUpdate = t_now;
    }

    int HistoryControl::Available()
    {
        if (storage.available() < maxHistory)
        {
            return storage.available() + (currStorageItem == NULL ? 0 : 1);
        }

        return storage.available();
    }

    const HistoryStorageItem HistoryControl::GetHistoryItem(int index)
    {
        if (currStorageItem == NULL)
            return storage.getItem(index);

        if (index == Available() - 1)
            return *currStorageItem;

        return storage.getItem(index + 1 - (storage.available() < maxHistory));
    }
}

/// @brief Global instance of HistoryControl.
historycontrol::HistoryControl historyControl;

// Initial state of the HistoryControl FSM.
FSM_INITIAL_STATE(historycontrol::HistoryControl, historycontrol::Init)
