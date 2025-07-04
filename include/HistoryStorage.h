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
enum class RecoveryStatus
{
    RecoveryStatuses
};
#undef X

class HistoryStorage;

class HistoryStorageItem
{
public:
    HistoryStorageItem()
    {
        data.endTime = INT32_MAX;
        data.modemRecoveries = 0;
        data.recoverySource = RecoverySource::Auto;
        data.recoveryStatus = RecoveryStatus::RecoverySuccess;
        data.routerRecoveries = 0;
        data.startTime = INT32_MAX;
    }

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
    
    HistoryStorageItem(const HistoryStorageItem &source)
    {
        *this = source;
    }
    

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
        Trace(data.startTime);
        Trace(", end=");
        Traceln(data.endTime);
#endif
        EEPROM.put<HistoryStorageItemData>(HISTORY_EEPROM_START_ADDRESS + sizeof(int) + sizeof(HistoryStorageItemData) * i, data);
    }

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
        Trace(data.startTime);
        Trace(", end=");
        Traceln(data.endTime);
#endif
        return *this;
    }

public:
    RecoverySource &recoverySource() { return data.recoverySource; }

    time_t & startTime() { return data.startTime; }

    time_t &endTime() { return data.endTime; }

    int &modemRecoveries() { return data.modemRecoveries; }

    int &routerRecoveries() { return data.routerRecoveries; }

    RecoveryStatus &recoveryStatus() { return data.recoveryStatus; }

    time_t recoveryTime() 
    { 
        if (data.recoveryStatus == RecoveryStatus::RecoverySuccess)
            return data.endTime == INT32_MAX ? data.startTime : data.endTime;
        else
            return INT32_MAX;
    }

private:
    struct HistoryStorageItemData
    {
        RecoverySource recoverySource;
        time_t startTime;
        time_t endTime;
        int modemRecoveries;
        int routerRecoveries;
        RecoveryStatus recoveryStatus;
    };

    HistoryStorageItemData data;

    friend class HistoryStorage;
};

class HistoryStorage
{
public:
    HistoryStorage();

    void init(int maxRecords);
    void addHistory(HistoryStorageItem &item);
    int available();
    void resize(int maxRecords);
    const HistoryStorageItem getItem(int index);
    time_t getLastRecovery();

private:
    int maxRecords;
    int availableRecords;
    int startIndex;
    time_t lastRecovery;

private:
    void putAvailableRecords();
    void getAvailableRecords();
#ifdef DEBUG_HISTORY
    void ReportInitializationResult();
#endif
};

#endif // HistoryStorage_h