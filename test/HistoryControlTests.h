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

    int addRecoveryStateChangedObserver(Observers<RecoveryStateChangedParams>::Handler handler, void *context)
    {
        return m_instance->m_recoveryStateChanged.addObserver(handler, context);
    }

    int addMaxHistoryRecordChangedObserver(Observers<MaxHistoryRecordChangedParams>::Handler handler, void *context)
    {
        return m_instance->m_maxHistoryRecordsChanged.addObserver(handler, context);
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
