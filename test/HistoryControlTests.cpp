#include <unity.h>
#include <HistoryControlTests.h>
#include <HistoryControl.cpp>
#include <AppConfig.cpp>
using namespace historycontrol;

#define WAIT_SEC [](){ for (time_t t0 = time(NULL); t0 == time(NULL); ); return time(NULL); }

void VerifyHistoryItem(
        HistoryControl &historyControl, 
        int index, 
        RecoverySource source, 
        RecoveryStatus status, 
        int modemRecoveries, 
        int routerRecoveries, 
        time_t startTime, 
        time_t endTime,
        const char *message = "")
{
    HistoryStorageItem item = historyControl.GetHistoryItem(index);
    TEST_ASSERT_EQUAL_MESSAGE(source, item.recoverySource(), message);
    TEST_ASSERT_EQUAL_MESSAGE(status, item.recoveryStatus(), message);
    TEST_ASSERT_EQUAL_MESSAGE(modemRecoveries, item.modemRecoveries(), message);
    TEST_ASSERT_EQUAL_MESSAGE(routerRecoveries, item.routerRecoveries(), message);
    TEST_ASSERT_EQUAL_MESSAGE(startTime, item.startTime(), message);
    TEST_ASSERT_EQUAL_MESSAGE(endTime, item.endTime(), message);
}

void SignalRecoveryStateChange(RecoveryTypes type, RecoverySource source)
{
    fakeRecoveryControl.GetRecoveryStateChanged().callObservers(RecoveryStateChangedParams(type, source));
}

void HistoryControlBasicTests()
{
    EEPROM.clear();
    fakeRecoveryControl.Init();
    AppConfig::init();
    HistoryControl historyControl;
    time_t now = WAIT_SEC();
    historyControl.init();
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    // Test the initial state of the history control
    TEST_ASSERT_EQUAL(0, historyControl.Available());
    time_t tLast = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(tLast, historyControl.getLastUpdate());
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 0, tLast, INT32_MAX, "User check connectivity");
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(0, historyControl.Available());
    TEST_ASSERT_EQUAL(tLast, historyControl.getLastUpdate());
    time_t tStart = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Modem, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Failed, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::RecoveryFailure, 1, 1, tStart, now, "Recovery failure");
    tStart = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::Auto, RecoveryStatus::OnGoingRecovery, 0, 0, tStart, INT32_MAX, "Auto check connectivity");
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::Auto, RecoveryStatus::RecoverySuccess, 0, 0, tStart, INT32_MAX, "Connectivity resume after recovery failure");
    TEST_ASSERT_EQUAL(tStart, historyControl.getLastRecovery());
}

void FillRecoverySuccessHistory(HistoryControl &historyControl, int count, RecoverySource source, time_t &now)
{
    for (int i = 0; i < count; i++)
    {
        SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, source);
        SignalRecoveryStateChange(RecoveryTypes::Modem, source);
        now = WAIT_SEC();
        SignalRecoveryStateChange(RecoveryTypes::NoRecovery, source);
    }
}

void VerifyRecoverySuccessHistory(HistoryControl &historyControl, int count, RecoverySource source, time_t &now, const char *message)
{
    TEST_ASSERT_EQUAL_MESSAGE(count, historyControl.Available(), message);
    TEST_ASSERT_EQUAL_MESSAGE(count, historyControl.Available(), message);
    for (int i = 0; i < count; i++)
        VerifyHistoryItem(
            historyControl, 
            i, 
            source, 
            RecoveryStatus::RecoverySuccess, 
            1, 
            0, 
            now - count + i,  
            now - count + i + 1, 
            message);
}   

void HistoryControlResizeTests()
{
    EEPROM.clear();
    AppConfig::init();
    fakeRecoveryControl.Init();
    TEST_ASSERT_EQUAL(10, AppConfig::getMaxHistory());
    HistoryControl historyControl;
    time_t now = WAIT_SEC();
    historyControl.init();
    fakeRecoveryControl.GetMaxHistoryRecordsChanged().callObservers(MaxHistoryRecordChangedParams(5));
    TEST_ASSERT_EQUAL(0, historyControl.Available());
    FillRecoverySuccessHistory(historyControl, 6, RecoverySource::UserInitiated, now);
    VerifyRecoverySuccessHistory(historyControl, 5, RecoverySource::UserInitiated, now, "History Size: 5");
    fakeRecoveryControl.GetMaxHistoryRecordsChanged().callObservers(MaxHistoryRecordChangedParams(3));
    VerifyRecoverySuccessHistory(historyControl, 3, RecoverySource::UserInitiated, now, "History Size: 3");
    fakeRecoveryControl.GetMaxHistoryRecordsChanged().callObservers(MaxHistoryRecordChangedParams(4));
    TEST_ASSERT_EQUAL(3, historyControl.Available());
    FillRecoverySuccessHistory(historyControl, 2, RecoverySource::UserInitiated, now);
    VerifyRecoverySuccessHistory(historyControl, 4, RecoverySource::UserInitiated, now, "History Size: 4");
}

void HistoryControlRouterRecoveryTests()
{
    EEPROM.clear();
    AppConfig::init();
    fakeRecoveryControl.Init();
    HistoryControl historyControl;
    time_t now = WAIT_SEC();
    time_t t0 = now;
    historyControl.init();
    TEST_ASSERT_EQUAL(0, historyControl.Available());
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 1, t0, INT32_MAX, "Router recovery On Going");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 2, t0, INT32_MAX, "Router recovery On Going 2");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::RecoverySuccess, 0, 2, t0, now, "Router recovery success");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    t0 = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 1, t0, INT32_MAX, "Router recovery On Going 3");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Failed, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::UserInitiated, RecoveryStatus::RecoveryFailure, 0, 1, t0, now, "Router recovery failure");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    t0 = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::RouterSingleDevice, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(3, historyControl.Available());
    VerifyHistoryItem(historyControl, 2, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 1, t0, INT32_MAX, "Router Single Device recovery On Going");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(3, historyControl.Available());
    VerifyHistoryItem(historyControl, 2, RecoverySource::UserInitiated, RecoveryStatus::RecoverySuccess, 0, 1, t0, now, "Router Single Device recovery success");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
}

void HistoryControlModemRecoveryTests()
{
    EEPROM.clear();
    AppConfig::init();
    fakeRecoveryControl.Init();
    HistoryControl historyControl;
    time_t now = WAIT_SEC();
    time_t t0 = now;
    historyControl.init();
    TEST_ASSERT_EQUAL(0, historyControl.Available());
    SignalRecoveryStateChange(RecoveryTypes::Modem, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 1, 0, t0, INT32_MAX, "Modem recovery On Going");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Modem, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 2, 0, t0, INT32_MAX, "Modem recovery On Going 2");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(1, historyControl.Available());
    VerifyHistoryItem(historyControl, 0, RecoverySource::UserInitiated, RecoveryStatus::RecoverySuccess, 2, 0, t0, now, "Modem recovery success");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    t0 = now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Modem, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 1, 0, t0, INT32_MAX, "Modem recovery On Going 3");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Failed, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::UserInitiated, RecoveryStatus::RecoveryFailure, 1, 0, t0, now, "Modem recovery failure");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
}

void DoPeriodicRestart(HistoryControl &historyControl, int n, bool modem, bool router, time_t &now)
{
    time_t t0 = now;
    AppConfig::setPeriodicallyRestartModem(modem);
    AppConfig::setPeriodicallyRestartRouter(router);
    String desc = String(router ? " Router" : "") + String(modem ? " Modem" : "");
    SignalRecoveryStateChange(RecoveryTypes::Periodic, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(n, historyControl.Available());
    VerifyHistoryItem(historyControl, n - 1, RecoverySource::Auto, RecoveryStatus::OnGoingRecovery, modem ? 1 : 0, router ? 1 : 0, t0, INT32_MAX, (String("Periodic restart On Going") + desc).c_str());
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::Auto);
    TEST_ASSERT_EQUAL(n, historyControl.Available());
    VerifyHistoryItem(historyControl, n - 1, RecoverySource::Auto, RecoveryStatus::RecoverySuccess, modem ? 1 : 0, router ? 1 : 0, t0, now, (String("Periodic restart success") + desc).c_str());
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
}

void HistoryControlPeriodicRestartTests()
{
    EEPROM.clear();
    AppConfig::init();
    fakeRecoveryControl.Init();
    HistoryControl historyControl;
    time_t now = WAIT_SEC();
    time_t t0 = now;
    historyControl.init();
    DoPeriodicRestart(historyControl, 1, true, true, now);
    DoPeriodicRestart(historyControl, 2, false, true, now);
    DoPeriodicRestart(historyControl, 3, true, false, now);
}

void DoConnectivityCheckWhileInRecoveryFailure(HistoryControl &historyControl, int n, time_t &t0)
{
    t0 = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, RecoverySource::UserInitiated);
    WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::UserInitiated);
    WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Modem, RecoverySource::UserInitiated);
    time_t now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Failed, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(n, historyControl.Available());
    VerifyHistoryItem(historyControl, n - 1, RecoverySource::UserInitiated, RecoveryStatus::RecoveryFailure, 1, 1, t0, now, "Router recovery failure");
    t0 = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::ConnectivityCheck, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(n + 1, historyControl.Available());
    VerifyHistoryItem(historyControl, n, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 0, t0, INT32_MAX, "Connectivity check after recovery failure");
    TEST_ASSERT_EQUAL(t0, historyControl.getLastUpdate());
}

void HistoryControlConnectivityCheckWhileInFailureTests()
{
    EEPROM.clear();
    AppConfig::init();
    fakeRecoveryControl.Init();
    HistoryControl historyControl;
    historyControl.init();
    time_t t0;
    DoConnectivityCheckWhileInRecoveryFailure(historyControl, 1, t0);
    time_t now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(2, historyControl.Available());
    VerifyHistoryItem(historyControl, 1, RecoverySource::UserInitiated, RecoveryStatus::RecoverySuccess, 0, 0, t0, INT32_MAX, "Connectivity resume after recovery failure");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    DoConnectivityCheckWhileInRecoveryFailure(historyControl, 3, t0);
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::Router, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(4, historyControl.Available());
    VerifyHistoryItem(historyControl, 3, RecoverySource::UserInitiated, RecoveryStatus::OnGoingRecovery, 0, 1, t0, INT32_MAX, "Router recovery after recovery failure");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
    now = WAIT_SEC();
    SignalRecoveryStateChange(RecoveryTypes::NoRecovery, RecoverySource::UserInitiated);
    TEST_ASSERT_EQUAL(4, historyControl.Available());
    VerifyHistoryItem(historyControl, 3, RecoverySource::UserInitiated, RecoveryStatus::RecoverySuccess, 0, 1, t0, now, "Recovery success after router recovery after recovery failure");
    TEST_ASSERT_EQUAL(now, historyControl.getLastUpdate());
}
FakeRecoveryControl fakeRecoveryControl;