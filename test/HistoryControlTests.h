#ifndef HistoryControlTests_h
#define HistoryControlTests_h

#include <FakeLock.h>
#include <FakeEEPROMEx.h>
#include <HistoryControl.h>

class FakeRecoveryControl
{
public:
    void Init()
    {
        delete m_instance;
        m_instance = new FakeRecoveryControl();
    }

    Observers<RecoveryStateChangedParams> &GetRecoveryStateChanged()
    {
        return m_instance->m_recoveryStateChanged;
    }

	Observers<MaxHistoryRecordChangedParams> &GetMaxHistoryRecordsChanged()
	{
		return m_instance->m_maxHistoryRecordsChanged;
	}
private:
	Observers<RecoveryStateChangedParams> m_recoveryStateChanged;
	Observers<MaxHistoryRecordChangedParams> m_maxHistoryRecordsChanged;
    FakeRecoveryControl *m_instance = NULL;
};

void historyControlBasicTests();
void historyControlResizeTests();
void historyControlRouterRecoveryTests();
void historyControlModemRecoveryTests();
void historyControlPeriodicRestartTests();
void historyControlConnectivityCheckWhileInFailureTests();

#define recoveryControl fakeRecoveryControl
extern FakeRecoveryControl fakeRecoveryControl;

#endif // HistoryControlTests_h
