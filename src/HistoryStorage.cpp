#include <Arduino.h>
#include <EEPROM.h>
#include <HistoryStorage.h>
#include <common.h>

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
    LOCK_TRACE();
    Trace("startIndex=");
    Traceln(startIndex);
    tm tr;
    char buff[64];
    localtime_r(&lastRecovery, &tr);
    strftime(buff, sizeof(buff), "%d/%m/%Y %T", &tr);
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
    getAvailableRecords();
    if (availableRecords == -1)
    {
        // Reset history on first time the board is running
        availableRecords = 0;
        putAvailableRecords();
    }
    if (availableRecords < maxRecords)
    {
        startIndex = availableRecords;
        if (availableRecords > 0)
        {
            HistoryStorageItem historyStorageItem;
            historyStorageItem.get(startIndex - 1);
            lastRecovery = historyStorageItem.recoveryTime();
        }
        else
            lastRecovery = INT32_MAX;
#ifdef DEBUG_HISTORY
        ReportInitializationResult();
#endif
        return;
    }

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
    item.put(startIndex);
    if (startIndex == availableRecords && availableRecords < maxRecords)
    {
        availableRecords++;
        putAvailableRecords();
    }
    startIndex = (startIndex + 1) % maxRecords;
    if (item.recoveryStatus() != RecoveryStatus::RecoverySuccess)
        lastRecovery = INT32_MAX;
    else
        lastRecovery = item.endTime();
    EEPROM.commit();
}

void HistoryStorage::resize(int _maxRecords)
{
    if (_maxRecords <= 0)
        return;

    if (availableRecords < maxRecords)
    {
        if (availableRecords <= _maxRecords)
        {
            maxRecords = _maxRecords;
            return;
        }
        else
        {
            maxRecords = availableRecords;
        }
    }

    int delta = _maxRecords - maxRecords;
    if (delta == 0)
        return;

    HistoryStorageItem item;

    if (delta < 0)
    {
        if (startIndex - delta <= maxRecords)
        {

            for(int i = startIndex; i < maxRecords + delta; i++)
                item.get(i - delta).put(i);
            maxRecords = _maxRecords;
            if (startIndex >= maxRecords)
                startIndex = 0;
        }
        else
        {
            maxRecords = _maxRecords;
            delta = startIndex - maxRecords;
            startIndex = 0;
            for (int i = 0; i < maxRecords; i++)
                item.get(i + delta).put(i);
        }
    }
    else
    {
        // Bubble  sort
        for (int i = 0; i < maxRecords - 1; i++)
        {
            bool swapped = false;
            HistoryStorageItem sj;
            sj.get(0);
            for (int j = 0; j < maxRecords - i - 1; j++)
            {
                HistoryStorageItem sj1;
                sj1.get(j+1);
                if (sj.startTime() > sj1.startTime())
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
    LOCK_TRACE();
    Trace("Stored availableRecords: ");
    Traceln(availableRecords);
#endif
}

void HistoryStorage::getAvailableRecords()
{
    availableRecords = EEPROM.get<int>(HISTORY_EEPROM_START_ADDRESS, availableRecords);
#ifdef DEBUG_HISTORY
    LOCK_TRACE();
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
