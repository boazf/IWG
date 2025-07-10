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

#ifndef HistoryStorage_h
#define HistoryStorage_h

#include <EEPROM.h>
#include <time.h>
#include <Common.h>
#include <RecoveryControl.h>
#ifdef DEBUG_HISTORY
#include <Trace.h>
#endif

#define HISTORY_EEPROM_START_ADDRESS 512

#define RecoveryStatuses \
    X(OnGoingRecovery) \
    X(RecoverySuccess) \
    X(RecoveryFailure)

#define X(a) a,
/// @brief RecoveryStatus enum class.
enum class RecoveryStatus
{
    RecoveryStatuses
};
#undef X

// Forward declaration of HistoryStorage class
class HistoryStorage;

/// @brief HistoryStorageItem class.
/// This class represents a single item in the history storage.
class HistoryStorageItem
{
public:
    /// @brief Default constructor for HistoryStorageItem.
    HistoryStorageItem()
    {
        data.endTime = INT32_MAX;
        data.modemRecoveries = 0;
        data.recoverySource = RecoverySource::Auto;
        data.recoveryStatus = RecoveryStatus::RecoverySuccess;
        data.routerRecoveries = 0;
        data.startTime = INT32_MAX;
    }

    /// @brief Constructor for HistoryStorageItem.
    /// @param _recoverySource The recovery source (User Initiated or Auto Recovery)
    /// @param _startTime The start time of the first recovery cycle.
    /// @param _endTime The end time of the last recovery cycle. If the recovery is ongoing, this is set to INT32_MAX.
    /// @param _modemRecoveries Number of modem recovery cycles that were performed.
    /// @param _routerRecoveries Number of router recovery cycles that were performed.
    /// @param _recoveryStatus The status of the recovery (OnGoingRecovery, RecoverySuccess, or RecoveryFailure).
    HistoryStorageItem(
        RecoverySource _recoverySource, 
        time_t _startTime, 
        time_t _endTime,
        int _modemRecoveries,
        int _routerRecoveries,
        RecoveryStatus _recoveryStatus)
    {
        data.endTime = _endTime;
        data.modemRecoveries = _modemRecoveries;
        data.recoverySource = _recoverySource;
        data.recoveryStatus = _recoveryStatus;
        data.routerRecoveries = _routerRecoveries;
        data.startTime = _startTime;
    }
    
    /// @brief Copy constructor for HistoryStorageItem.
    /// @param source The source HistoryStorageItem to copy from.
    HistoryStorageItem(const HistoryStorageItem &source)
    {
        *this = source;
    }
    
    /// @brief Comparison operator for HistoryStorageItem.
    /// @param other The other HistoryStorageItem to compare with.
    /// @return True if the start time of the current item is greater than the other item's start time, false otherwise.
    /// This operator is used to sort history items in ascending order based on their start time.
    bool operator >(const HistoryStorageItem &other)
    {
        return data.startTime > other.data.startTime;
    }

    /// @brief Assignment operator for HistoryStorageItem.
    /// @param source The source HistoryStorageItem to assign from.
    /// @return A reference to the current HistoryStorageItem instance.
    HistoryStorageItem &operator=(const HistoryStorageItem& source)
    {
        data.endTime = source.data.endTime;
        data.modemRecoveries = source.data.modemRecoveries;
        data.recoverySource = source.data.recoverySource;
        data.recoveryStatus = source.data.recoveryStatus;
        data.routerRecoveries = source.data.routerRecoveries;
        data.startTime = source.data.startTime;

        return *this;
    }

private:
    /// @brief Put the history item data into EEPROM at the specified index.
    /// @param i The index in the EEPROM where the history item data should be stored.
    void put(int i) const
    {
#ifdef DEBUG_HISTORY
        LOCK_TRACE;
        Trace("Put History(");
        Trace(i);
        Trace("): source=");
        Trace(static_cast<int>(data.recoverySource));
        Trace(", status=");
        Trace(static_cast<int>(data.recoveryStatus));
        Trace(", router=");
        Trace(data.routerRecoveries);
        Trace(", modem=");
        Trace(data.modemRecoveries);
        Trace(", start=");
        Trace((unsigned int)data.startTime);
        Trace(", end=");
        Traceln((unsigned int)data.endTime);
#endif
        EEPROM.put<HistoryStorageItemData>(HISTORY_EEPROM_START_ADDRESS + sizeof(int) + sizeof(HistoryStorageItemData) * i, data);
    }

    /// @brief Get the history item data from EEPROM at the specified index.
    /// @param i The index in the EEPROM from which the history item data should be retrieved.
    /// @return A reference to the current HistoryStorageItem instance with the retrieved data.
    HistoryStorageItem &get(int i)
    {
        EEPROM.get<HistoryStorageItemData>(HISTORY_EEPROM_START_ADDRESS + sizeof(int) + sizeof(HistoryStorageItemData) * i, data);
#ifdef DEBUG_HISTORY
        LOCK_TRACE;
        Trace("Get History(");
        Trace(i);
        Trace("): source=");
        Trace(static_cast<int>(data.recoverySource));
        Trace(", status=");
        Trace(static_cast<int>(data.recoveryStatus));
        Trace(", router=");
        Trace(data.routerRecoveries);
        Trace(", modem=");
        Trace(static_cast<int>(data.modemRecoveries));
        Trace(", start=");
        Trace((unsigned int)data.startTime);
        Trace(", end=");
        Traceln((unsigned int)data.endTime);
#endif
        return *this;
    }

public:
    /// @brief Get the recovery source of the history item.
    /// @return A reference to the recovery source of the history item.
    RecoverySource &recoverySource() { return data.recoverySource; }

    /// @brief Get the start time of the history item.
    /// @return A reference to the start time of the history item.
    time_t & startTime() { return data.startTime; }

    /// @brief Get the end time of the history item.
    /// @return A reference to the end time of the history item.
    time_t &endTime() { return data.endTime; }

    /// @brief Get the number of modem recoveries performed.
    /// @return A reference to the number of modem recoveries.
    int &modemRecoveries() { return data.modemRecoveries; }

    /// @brief Get the number of router recoveries performed.
    /// @return A reference to the number of router recoveries.
    int &routerRecoveries() { return data.routerRecoveries; }

    /// @brief Get the recovery status of the history item.
    /// @return A reference to the recovery status of the history item.
    RecoveryStatus &recoveryStatus() { return data.recoveryStatus; }

    /// @brief Get the recovery time of the history item.
    /// @return The time of the last recovery event. If the recovery was successful, it
    /// returns the end time of the recovery. If the recovery is ongoing or failed, it returns INT32_MAX.
    time_t recoveryTime() 
    { 
        if (data.recoveryStatus == RecoveryStatus::RecoverySuccess)
            return data.endTime == INT32_MAX ? data.startTime : data.endTime;
        else
            return INT32_MAX;
    }

private:
    /// @brief Data structure to hold the history item data.
    /// This structure is used to store the history item data in EEPROM.
    struct HistoryStorageItemData
    {
        RecoverySource recoverySource;
        time_t startTime;
        time_t endTime;
        int modemRecoveries;
        int routerRecoveries;
        RecoveryStatus recoveryStatus;
    };

    /// @brief The data of the history item.
    /// This is the actual data that will be stored in EEPROM.
    HistoryStorageItemData data;

    /// @brief Friend class declaration for HistoryStorage.
    friend class HistoryStorage;
};

/// @brief HistoryStorage class.
/// This class is responsible for managing the history storage in EEPROM.
/// The records in EEPROM are stored in a circular buffer fashion.
/// It allows adding new history items, resizing the storage, and retrieving history items.
class HistoryStorage
{
public:
    HistoryStorage();

    /// @brief Initialize the history storage.
    /// This function finds the latest recovery item in the cyclical history storage.
    /// This location will be used later for adding new history items.
    /// @param maxRecords The maximum number of history records to store.
    void init(int maxRecords);
    /// @brief Add a history item to the storage.
    /// @param item The history item to add to the storage.
    /// The history items are added in a circular buffer fashion.
    /// If the storage is full, the oldest item will be overwritten.
    void addHistory(HistoryStorageItem &item);
    /// @brief Get the number of available history records.
    /// @return The number of available history records in the storage.
    int available();
    /// @brief Resize the history storage.
    /// @param maxRecords The new maximum number of history records to store.
    void resize(int maxRecords);
    /// @brief Get a history item by index.
    /// @param index The index of the history item to retrieve.
    const HistoryStorageItem getItem(int index);
    /// @brief Get the last recovery time.
    /// @return The time of the last recovery event. If no recovery has occurred, it
    /// returns INT32_MAX.
    /// If the last recovery was successful, it returns the end time of the recovery.
    time_t getLastRecovery();

private:
    /// @brief The maximum number of history records to store.
    int maxRecords;
    /// @brief The number of available history records in the storage.
    int availableRecords;
    /// @brief The index of the start of the history records in the storage.
    /// This is the index where the next history item will be added.
    int startIndex;
    /// @brief The last recovery time.
    /// This is the time of the last recovery event. If no recovery has occurred, it
    /// is set to INT32_MAX. 
    /// If the last recovery was successful, it is set to the end time of the recovery.
    time_t lastRecovery;

private:
    /// @brief Store the number of available records in EEPROM.
    void putAvailableRecords();
    /// @brief Retrieve the number of available records from EEPROM.
    void getAvailableRecords();
#ifdef DEBUG_HISTORY
    /// @brief Report the initialization result of the history storage.
    /// This function is used for debugging purposes to report the initialization result of the history storage.
    void ReportInitializationResult();
#endif
};

#endif // HistoryStorage_h