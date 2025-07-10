#include <unity.h>
#include <Arduino.h>
#include <FakeLock.h>
#include <FakeEEPROMEx.h>
#include "HistoryStorageTests.h"
#include <Trace.h>
#include <HistoryStorage.h>
#include <HistoryStorage.cpp>

/// @brief Add history item to the history storage.
/// @param historyStorage The history storage to add the item to.
/// @param source The recovery source (User Initiated or Auto Recovery).
/// @param now The start time of the recovery.
/// @param endTime The end time of the recovery.
/// @param modemRecoveries The number of modem recovery attempts made.
/// @param routerRecoveries The number of router recovery attempts made.
/// @param status The recovery status (RecoverySuccess, or RecoveryFailure).
void AddHistory(HistoryStorage &historyStorage, 
                RecoverySource source, 
                time_t &now, 
                time_t endTime,
                int modemRecoveries, 
                int routerRecoveries, 
                RecoveryStatus status)
{
    // Create a new history item and add it to the history storage.
    HistoryStorageItem item(source, now, endTime, modemRecoveries, routerRecoveries, status);
    historyStorage.addHistory(item);
    now += 1; // Increment time for the next item
}

/// @brief Add history item to the history storage with a default end time.
/// This function is a convenience wrapper that sets the end time to now + 1.
/// @param historyStorage The history storage to add the item to.
/// @param source The recovery source (User Initiated or Auto Recovery).
/// @param now The start time of the recovery.
/// @param modemRecoveries The number of modem recovery attempts made.
/// @param routerRecoveries The number of router recovery attempts made.
/// @param status The recovery status (RecoverySuccess, or RecoveryFailure).
void AddHistory(HistoryStorage &historyStorage, 
                RecoverySource source, 
                time_t &now, 
                int modemRecoveries, 
                int routerRecoveries, 
                RecoveryStatus status)
{
    AddHistory(historyStorage, source, now, now + 1, modemRecoveries, routerRecoveries, status);
    now += 1; // Increment time for the next item
}

/// @brief Basic tests for the HistoryStorage class.
/// This function initializes the HistoryStorage, adds history items, and verifies the contents
/// of the storage after each addition.
/// It checks the number of available records, the contents of each record, and the last recovery
void HistoryStorageBasicTests() {
    // Initialize the HistoryStorage
    HistoryStorage historyStorage;
    historyStorage.init(10); // Initialize with a maximum of 10 records

    time_t t0 = time(NULL);
    time_t now = t0;
    // Check if the initial available records is 0
    TEST_ASSERT_EQUAL(0, historyStorage.available());

    for (int i = 0; i < 12; i++)
    {
        AddHistory(historyStorage, RecoverySource::UserInitiated, now, 1, 0, RecoveryStatus::RecoverySuccess);
        TEST_ASSERT_EQUAL(i >= 9 ? 10 : i + 1, historyStorage.available());
        // Verify the contents of the history storage
        for (int j = 0; j < historyStorage.available(); j++)
        {
            HistoryStorageItem retrievedItem = historyStorage.getItem(j);
            time_t t = now - (historyStorage.available() -j) * 2;
            TEST_ASSERT_EQUAL(t + 1, retrievedItem.recoveryTime());
            TEST_ASSERT_EQUAL(t, retrievedItem.startTime());
            TEST_ASSERT_EQUAL(t + 1, retrievedItem.endTime());
            TEST_ASSERT_EQUAL(1, retrievedItem.modemRecoveries());
            TEST_ASSERT_EQUAL(0, retrievedItem.routerRecoveries());
            TEST_ASSERT_EQUAL(RecoverySource::UserInitiated, retrievedItem.recoverySource());
            TEST_ASSERT_EQUAL(RecoveryStatus::RecoverySuccess, retrievedItem.recoveryStatus());
        }
    }

    // Check if the last recovery time is INT32_MAX
    TEST_ASSERT_EQUAL_INT32(t0 + 23, historyStorage.getLastRecovery());
}

/// @brief This function fills the history storage with a specified number of items.
/// It adds items with a fixed recovery source, start time, end time, and status.
/// @param nItems The number of items to add to the history storage.
/// @param historyStorage The history storage to fill with items.
/// @param t0 The start time for the first item.
/// The start time for each subsequent item is incremented by 2 seconds.
void FillHistoryStorage(int nItems, HistoryStorage &historyStorage, time_t &t0)
{
    for (int i = 0; i < nItems; i++)
    {
        AddHistory(historyStorage, RecoverySource::Auto, t0, 1, 0, RecoveryStatus::RecoverySuccess);
    }
}

/// @brief Verify the contents of the history storage.
/// This function checks if the number of available records matches the expected count.
/// It iterates through the records and verifies that each record has the expected start time,
/// end time, recovery source, and recovery status.
/// @param historyStorage The history storage to verify.
/// @param now The current time used to calculate expected start and end times.
/// @param nItems The expected number of items in the history storage.
void VerifyHistoryItems(HistoryStorage &historyStorage, time_t now, int nItems)
{
    TEST_ASSERT_EQUAL(nItems, historyStorage.available());

    for (int i = 0; i < historyStorage.available(); i++)
    {
        HistoryStorageItem item = historyStorage.getItem(i);
        TEST_ASSERT_EQUAL(now - 2 * (nItems - i), item.startTime());
        TEST_ASSERT_EQUAL(now - 2 * (nItems - i) + 1, item.endTime());
        TEST_ASSERT_EQUAL(RecoverySource::Auto, item.recoverySource());
        TEST_ASSERT_EQUAL(RecoveryStatus::RecoverySuccess, item.recoveryStatus());
    }
}

/// @brief Basic resize tests for the HistoryStorage class.
/// This function initializes the HistoryStorage, fills it with items, resizes it,
/// and verifies the contents after resizing.
/// This resize doesn't cause items to shuffle.
void BasicResizeHistoryStorageTests()
{
    EEPROM.clear();
    HistoryStorage historyStorage;
    historyStorage.init(8); // Initialize with a maximum of 10 records

    time_t t0 = time(NULL);
    time_t now = t0;
    FillHistoryStorage(6, historyStorage, now);

    // Check if the available records is 10 after adding 12 items
    TEST_ASSERT_EQUAL(6, historyStorage.available());

    // Resize to 5 records
    historyStorage.resize(10);
    TEST_ASSERT_EQUAL(6, historyStorage.available());

    FillHistoryStorage(6, historyStorage, now);
    VerifyHistoryItems(historyStorage, now, 10);
}

/// @brief Test history storage initialization. 
void InitHistoryStorageTests()
{
    time_t t0 = time(NULL);
    time_t now = t0;
    EEPROMEx.clear();
    time_t lastRecovery;
    {
        HistoryStorage historyStorage;
        historyStorage.init(10); // Initialize with a maximum of 10 records

        // Check if the initial available records is 0
        TEST_ASSERT_EQUAL(0, historyStorage.available());

        // Check if the last recovery time is INT32_MAX
        TEST_ASSERT_EQUAL_INT32(INT32_MAX, historyStorage.getLastRecovery());

        // Fill first 8 items
        FillHistoryStorage(8, historyStorage, now);

        // Check if the last recovery time is the end time of the last item
        lastRecovery = historyStorage.getLastRecovery();
        TEST_ASSERT_EQUAL_INT32(t0 + 15, lastRecovery);
    }
    {
        HistoryStorage historyStorage;
        historyStorage.init(10); // Initialize with a maximum of 10 records

        // Check if the last recovery time is the same as before
        TEST_ASSERT_EQUAL_INT32(lastRecovery, historyStorage.getLastRecovery());

        // Check the items after initialization
        VerifyHistoryItems(historyStorage, now, 8);
        // Fill next 2 items
        FillHistoryStorage(2, historyStorage, now);

        // Check if the last recovery time is the end time of the last item
        lastRecovery = historyStorage.getLastRecovery();
        TEST_ASSERT_EQUAL_INT32(t0 + 19, lastRecovery);
    }
    {
        HistoryStorage historyStorage;
        historyStorage.init(10); // Initialize with a maximum of 10 records

        // Check if the last recovery time is the same as before
        TEST_ASSERT_EQUAL_INT32(lastRecovery, historyStorage.getLastRecovery());

        // Check the items after initialization
        VerifyHistoryItems(historyStorage, now, 10);

        // Fill next 2 items
        FillHistoryStorage(2, historyStorage, now);

        // Check if the last recovery time is the end time of the last item
        lastRecovery = historyStorage.getLastRecovery();
        TEST_ASSERT_EQUAL_INT32(t0 + 23, lastRecovery);
    }
    {
        HistoryStorage historyStorage;
        historyStorage.init(10); // Initialize with a maximum of 10 records

        // Check if the last recovery time is the same as before
        TEST_ASSERT_EQUAL_INT32(lastRecovery, historyStorage.getLastRecovery());

        // Check the items after initialization
        VerifyHistoryItems(historyStorage, now, 10);
    }
}

/// @brief Test shrinking the history storage.
/// This function initializes the HistoryStorage, fills it with items, resizes it,
/// so that the size is reduced to a smaller size.
/// It verifies that the number of items in the storage is reduced to the new size and
/// that the items are correctly shuffled in the storage.
void TestShrinkHistoryStorage()
{
    // Test various combinations of number of history items and resize values.
    for (int resize = 1; resize <=10; resize++)
    {
        Tracef("resize=%d\n", resize);
        for (int nHistoryItems = 1; nHistoryItems <= 22; nHistoryItems++)
        {
            Tracef("nHistoryItems=%d\n", nHistoryItems);
            // Clear the EEPROM to simulate a fresh start
            EEPROMEx.clear();
            HistoryStorage historyStorage;
            historyStorage.init(10); // Initialize with a maximum of 10 records

            time_t t0 = time(NULL);
            time_t now = t0;
            // Fill the history storage with nHistoryItems
            FillHistoryStorage(nHistoryItems, historyStorage, now);

            // Resize and verify that the remaining items are correct
            historyStorage.resize(resize);
            VerifyHistoryItems(historyStorage, now, std::min<int>(nHistoryItems, resize));
        }
    }
}

/// @brief Test enlarging the history storage.
/// This function initializes the HistoryStorage, fills it with items, resizes it,
/// so that the size is increased to a larger size.
/// It verifies that the number of items in the storage remains the same as before resizing
/// and that all previous items are still present in the storage.
/// It also checks that new items can be added up to the new size.
void EnlargeHistoryStorageTests()
{
    // Test various combinations of number of history items and resize values.
    for (int resize = 11; resize < 20; resize++)
    {
        Tracef("resize=%d\n", resize);
        for (int nHistoryItems = 10; nHistoryItems <= 22; nHistoryItems++)
        {
            EEPROMEx.clear();
            Tracef("nHistoryItems=%d\n", nHistoryItems);
            HistoryStorage historyStorage;
            historyStorage.init(10); // Reinitialize with a maximum of 10 records

            time_t t0 = time(NULL);
            time_t now = t0;
            FillHistoryStorage(nHistoryItems, historyStorage, now);

            // Resize
            historyStorage.resize(resize);
            // Check if the all records are still present and in the correct order
            VerifyHistoryItems(historyStorage, now, 10);

            // Add more items to fill the storage up to the new size
            FillHistoryStorage(resize - 10, historyStorage, now);
            // Verify that the number of items is now equal to the new size
            // and that all items are correctly stored
            VerifyHistoryItems(historyStorage, now, resize);

            // Fill the history storage with more items to wrap around the storage.
            FillHistoryStorage(5, historyStorage, now);
            // Verify that all items are correctly stored after wrapping around
            // and that the number of items is still equal to the new size
            VerifyHistoryItems(historyStorage, now, resize);
        }
    }
}

/// @brief Test the last recovery time in the history storage.
/// This function initializes the HistoryStorage, adds history items with different recovery statuses,
/// and verifies the last recovery time after each addition.
void TestLastRecovery()
{
    EEPROMEx.clear();
    HistoryStorage historyStorage;
    historyStorage.init(10); // Initialize with a maximum of 10 records

    time_t t0 = time(NULL);
    time_t now = t0;

    // Check last recovery with 0 history items
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, historyStorage.getLastRecovery());

    // Add a history item of recovery success
    AddHistory(historyStorage, RecoverySource::UserInitiated, now, 1, 0, RecoveryStatus::RecoverySuccess);
    // Check if the last recovery time is the end time of the last item
    TEST_ASSERT_EQUAL_INT32(t0 + 1, historyStorage.getLastRecovery());

    // Add another history item of recovery failure
    AddHistory(historyStorage, RecoverySource::UserInitiated, now, 1, 0, RecoveryStatus::RecoveryFailure);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, historyStorage.getLastRecovery());

    // Add another history item of recovery success without end time
    AddHistory(historyStorage, RecoverySource::UserInitiated, now, INT32_MAX, 1, 0, RecoveryStatus::RecoverySuccess);
    TEST_ASSERT_EQUAL_INT32(t0 + 4, historyStorage.getLastRecovery());

    AddHistory(historyStorage, RecoverySource::Auto, now, 1, 0, RecoveryStatus::RecoverySuccess);
    // Check if the last recovery time is the end time of the last item
    TEST_ASSERT_EQUAL_INT32(t0 + 6, historyStorage.getLastRecovery());

    // Add another history item of recovery failure
    AddHistory(historyStorage, RecoverySource::Auto, now, 1, 0, RecoveryStatus::RecoveryFailure);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, historyStorage.getLastRecovery());

    // Add another history item of recovery success without end time
    AddHistory(historyStorage, RecoverySource::Auto, now, INT32_MAX, 1, 0, RecoveryStatus::RecoverySuccess);
    TEST_ASSERT_EQUAL_INT32(t0 + 9, historyStorage.getLastRecovery());
}

void TestModemAndRouterRecoveryCounts()
{
    EEPROMEx.clear();
    HistoryStorage historyStorage;
    historyStorage.init(10); // Initialize with a maximum of 10 records

    time_t t0 = time(NULL);
    time_t now = t0;

    // Add a history item with modem and router recoveries
    AddHistory(historyStorage, RecoverySource::UserInitiated, now, 3, 5, RecoveryStatus::RecoverySuccess);
    HistoryStorageItem item = historyStorage.getItem(0);
    TEST_ASSERT_EQUAL(3, item.modemRecoveries());
    TEST_ASSERT_EQUAL(5, item.routerRecoveries());

    // Add another history item with different modem and router recoveries
    AddHistory(historyStorage, RecoverySource::Auto, now, 4, 2, RecoveryStatus::RecoverySuccess);
    item = historyStorage.getItem(1);
    TEST_ASSERT_EQUAL(4, item.modemRecoveries());
    TEST_ASSERT_EQUAL(2, item.routerRecoveries());
}
