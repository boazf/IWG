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

#include <Arduino.h>
#include <EEPROM.h>
#include <HistoryStorage.h>
#include <common.h>
#ifdef DEBUG_HISTORY
#include <Trace.h>
#endif

HistoryStorage::HistoryStorage()
{
}

time_t HistoryStorage::getLastRecovery()
{ 
    return lastRecovery; 
}

#ifdef DEBUG_HISTORY
void HistoryStorage::ReportInitializationResult()
{
    LOCK_TRACE;
    Trace("startIndex=");
    Traceln(startIndex);
    tm tr;
    char buff[64];
    localtime_r(&lastRecovery, &tr);
    strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M:%S", &tr);
    Trace("Last Recovery: ");
    Traceln(buff);
}
#endif

void HistoryStorage::init(int _maxRecords)
{
#ifdef RESET_HISTORY
    availableRecords = 0;
    putAvailableRecords();
    EEPROM.commit();
#endif
    maxRecords = _maxRecords;
    getAvailableRecords(); // Retrieve the number of available records from EEPROM
    if (availableRecords == -1)
    {
        // Reset history on first time the board is running
        availableRecords = 0;
        putAvailableRecords();
    }
    if (availableRecords < maxRecords)
    {
        // If the number of available records is less than the maximum records, set startIndex to 
        // the number of available records and set lastRecovery to the recovery time of the last record.
        startIndex = availableRecords;
        if (availableRecords > 0)
        {
            HistoryStorageItem historyStorageItem;
            historyStorageItem.get(startIndex - 1);
            lastRecovery = historyStorageItem.recoveryTime();
        }
        else
            // If there are no available records, set lastRecovery to INT32_MAX.
            lastRecovery = INT32_MAX;
#ifdef DEBUG_HISTORY
        ReportInitializationResult();
#endif
        return;
    }

    // If the number of available records is equal to the maximum records, find the earliest record
    // and set startIndex to the index of that record.
    // The lastRecovery to the recovery time of the oldest record.
    time_t minT = INT32_MAX;
    startIndex = 0;
    bool first = true;
    lastRecovery = INT32_MAX;

    for (int i = 0; i < maxRecords; i++)
    {
        HistoryStorageItem historyStorageItem;
        historyStorageItem.get(i);
        time_t startTime = historyStorageItem.startTime();
        if (minT > startTime)
        {
            // We always get here with record index 0.
            // Next time that we'll get here, we will actually hit the earliest record.
            // This is because we are using a circular buffer.
            // If don't get here again, this means that all records are arranged in ascending order
            // and the first record is the earliest one.
            minT = startTime;
            startIndex = i;
            if (!first)
                break;
        }
        lastRecovery = historyStorageItem.recoveryTime();
        first = false;
    }

#ifdef DEBUG_HISTORY
        ReportInitializationResult();
#endif
}

void HistoryStorage::addHistory(HistoryStorageItem &item)
{
    // Store the item in the EEPROM at the current startIndex.
    item.put(startIndex);
    if (availableRecords < maxRecords)
    {
        // If the circular buffer is not full yet, increment the availableRecords count.
        availableRecords++;
        putAvailableRecords();
    }
    // Increment the startIndex in a circular manner.
    startIndex = (startIndex + 1) % maxRecords;
    // Update last recovery time.
    lastRecovery = item.recoveryTime();
    // Commit changes to EEPROM
    EEPROM.commit();
}

void HistoryStorage::resize(int _maxRecords)
{
    if (_maxRecords <= 0)
        return;

    if (availableRecords < maxRecords)
    {
        // We haven't reached the maximum records yet, so we can just set the new maxRecords.
        if (availableRecords <= _maxRecords)
        {
            // If the new maxRecords is greater than or equal to the available records,
            // we can just set maxRecords to _maxRecords and return.
            maxRecords = _maxRecords;
            return;
        }
        else
        {
            // If the new maxRecords is less than the available records, we need to reshuffle the records.
            maxRecords = availableRecords;
        }
    }

    // Calculate the difference between the current maxRecords and the new _maxRecords.
    // If the difference is zero, we don't need to do anything.
    int delta = _maxRecords - maxRecords;
    if (delta == 0)
        return;

    HistoryStorageItem item;

    if (delta < 0)
    {
        // If we are reducing the number of records, we need to shift the records.
        if (startIndex <= _maxRecords)
        {
            // maxRecords = 10
            // startIndex = 5
            // availableRecords = 10
            // _maxRecords = 6
            // delta = -4
            // +-+-+-+-+-+-+-+-+-+-+
            // |5|6|7|8|9|0|1|2|3|4|
            // +-+-+-+-+-+-+-+-+-+-+
            //            ↑
            // startIndex ┘
            // Shift from the end of the buffer to the startIndex.
            for(int i = startIndex; i < _maxRecords; i++)
                item.get(i - delta).put(i);
            maxRecords = _maxRecords;
            if (startIndex >= maxRecords)
                startIndex = 0;
            // +-+-+-+-+-+-+
            // |5|6|7|8|9|4|
            // +-+-+-+-+-+-+
            //            ↑
            // startIndex ┘
        }
        else
        {
            // maxRecords = 10
            // startIndex = 7
            // availableRecords = 10
            // _maxRecords = 6
            // delta = -4
            // +-+-+-+-+-+-+-+-+-+-+
            // |3|4|5|6|7|8|9|0|1|2|
            // +-+-+-+-+-+-+-+-+-+-+
            //                ↑
            //     startIndex ┘
            // Shift the latest _maxRecords to the start of the buffer.
            maxRecords = _maxRecords;
            delta = startIndex - maxRecords;
            startIndex = 0;
            for (int i = 0; i < maxRecords; i++)
                item.get(i + delta).put(i);
            // maxRecords = 6
            // delta = 7 - 6 = 1
            // +-+-+-+-+-+-+
            // |4|5|6|7|8|9|
            // +-+-+-+-+-+-+
            //  ↑
            //  └─ startIndex
        }
    }
    else
    {
        // maxRecords = 10
        // startIndex = 7
        // availableRecords = 10
        // _maxRecords = 12
        // delta = 2
        // +-+-+-+-+-+-+-+-+-+-+
        // |3|4|5|6|7|8|9|0|1|2|
        // +-+-+-+-+-+-+-+-+-+-+
        //                ↑
        //     startIndex ┘
        // Bubble  sort
        for (int i = 0; i < maxRecords - 1; i++)
        {
            bool swapped = false;
            HistoryStorageItem sj;
            sj.get(0);
            for (int j = 0; j < maxRecords - i - 1; j++)
            {
                HistoryStorageItem sj1;
                if (sj > sj1.get(j+1))
                {
                    sj.put(j+1);
                    sj1.put(j);
                    swapped = true;
                }
                else
                    sj = sj1;
            }
            if (!swapped)
                break;
        }
        maxRecords = _maxRecords;
        startIndex = availableRecords < maxRecords ? availableRecords : 0; 
        // +-+-+-+-+-+-+-+-+-+-+-+-+
        // |0|1|2|3|4|5|6|7|8|9| | |
        // +-+-+-+-+-+-+-+-+-+-+-+-+
        //                      ↑
        //           startIndex ┘
        // startIndex = 10
        // maxRecords = 12
        // availableRecords = 10
    }

    if (availableRecords > maxRecords)
    {
        availableRecords = maxRecords;
        putAvailableRecords();
    }
    EEPROM.commit();
}

int HistoryStorage::available()
{
    return availableRecords;
}

void HistoryStorage::putAvailableRecords()
{
    EEPROM.put<int>(HISTORY_EEPROM_START_ADDRESS, availableRecords);
#ifdef DEBUG_HISTORY
    LOCK_TRACE;
    Trace("Stored availableRecords: ");
    Traceln(availableRecords);
#endif
}

void HistoryStorage::getAvailableRecords()
{
    availableRecords = EEPROM.get<int>(HISTORY_EEPROM_START_ADDRESS, availableRecords);
#ifdef DEBUG_HISTORY
    LOCK_TRACE;
    Trace("availableRecords: ");
    Traceln(availableRecords);
#endif
}

const HistoryStorageItem HistoryStorage::getItem(int index)
{
    index = (startIndex + index) % availableRecords;
    HistoryStorageItem item;
    return item.get(index);
}
