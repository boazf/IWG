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

#ifndef HistoryControl_h
#define HistoryControl_h

#include <CommonFsmDefs.h>
#include <HistoryStorage.h>

namespace historycontrol
{
    /// @brief HistoryControl class.
    /// This class is responsible for managing the history of recovery events.
    class HistoryControl : public tinyfsm::Fsm<HistoryControl>
    {
    public:
        HistoryControl() {}

    public:
        /* Default reaction for unhandled events */
        void react(tinyfsm::Event const &) { };

        /// @brief React to RecoveryStateChanged event.
        /// This event is triggered when the recovery state changes.
        virtual void react(RecoveryStateChanged const &) {};

        virtual void entry(void) { };  /* entry actions in some states */
        virtual void exit(void)  { };  /* entry actions in some states */

        /// @brief Initialize the history control.
        /// This method initializes the history control by setting up the storage and loading existing history items.
        /// It also registers observers for recovery state changes and maximum history record changes.
        /// It should be called once at the start of the application.
        void init();
        /// @brief Used to get the number of available history items.
        /// @return The number of available history items.
        int Available();
        /// @brief Get a history item by index.
        /// @param index 
        /// @return The history item at the specified index.
        const HistoryStorageItem GetHistoryItem(int index);
        /// @brief Get the last recovery time.
        /// @return The time of the last recovery event.
        time_t getLastRecovery();
        /// @brief Get the last update time.
        /// @return The time of the last update to the history.
        time_t getLastUpdate();

    protected:
        /// @brief Maximum number of history records.
        static int maxHistory;
        /// @brief Current recovery type.
        static RecoveryTypes recoveryType;
        /// @brief Current recovery source.
        static RecoverySource recoverySource;
        /// @brief Static instance of HistoryStorage.
        static HistoryStorage storage;
        /// @brief Current storage item being processed.
        static HistoryStorageItem *currStorageItem;
        /// @brief Last update time of the history.
        static time_t lastUpdate;
        
    protected:
        /// @brief Add a new history item to the history.
        /// Upon successful recovery completion, create a new history item with the current recovery source
        /// and add it to the history storage. This method is called when connectivity is resumed after a
        /// recovery failure.
        void AddToHistory();
        /// @brief Create a history item with the current recovery source.
        /// @param recoverySource The source of the recovery event (User Initiated or Auto).
        bool CreateHistoryItem(RecoverySource recoverySource);
        /// @brief Add the current history item to the history storage (EEPROM).
        /// @param status The status of the recovery event (success, failure, etc.).
        /// @param withEndTime If true, the end time of the recovery will be set to the current time.
        /// The end time is not written when connectivity is resumed after a recovery failure. This 
        /// type of log indicates the time when connectivity resumed. But there weren't any actual 
        /// recovery cycles.
        void AddToHistoryStorage(RecoveryStatus status, bool withEndTime = true);

    protected:
        /// @brief Send an event to the FSM.
        /// @tparam E The type of the event to send.
        /// @param event The event to send to the FSM.
        template<typename E>
        void send_event(E const &event)
        {
            tinyfsm::FsmList<HistoryControl>::template dispatch<E>(event);
        }

    private:
        /// @brief Observer for RecoveryStateChanged events.
        /// @param params The parameters of the recovery state change event.
        /// @param context The context pointer to the HistoryControl instance.
        static void onRecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context);
        /// @brief Observer for MaxHistoryRecordChanged events.
        /// @param params The parameters of the max history record change event.
        /// @param context The context pointer to the HistoryControl instance.
        static void onMaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context);
    };
}

/// @brief Global instance of HistoryControl.
extern historycontrol::HistoryControl historyControl;

#endif // HistoryControl_h