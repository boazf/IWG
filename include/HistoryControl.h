#ifndef HistoryControl_h
#define HistoryControl_h

#include <StateMachine.h>
#include <RecoveryControl.h>
#include <HistoryStorage.h>

enum class H_Message
{
    None = 0,
    CheckConnectivity,
    RouterRecovery,
    ModemRecovery,
    RecoveryFailure,
    RecoverySuccess,
    PeriodicRestart,
    HWFailure
};

enum class H_State
{
    Init,
    Connected,
    CheckingConnectivity,
    RecoveringRouter,
    RecoveringModem,
    RecoveryFailed,
    CheckingConnectivityWhileInFailure,
    PeriodicRestart,
    HWFailure
};

class HistoryControl
{
public:
    HistoryControl();
    ~HistoryControl();
	void Init();
	void PerformCycle();
    int Available();
    const HistoryStorageItem GetHistoryItem(int index);
    time_t getLastRecovery();
    time_t getLastUpdate();

private:
    static void RecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context);
    static void MaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context);
    static H_Message OnInit(void *param);
    static void OnConnected(void *param);
    static void OnCheckingConnectivity(void *param);
    static void OnRecoveringModem(void *param);
    static void OnRecoveringRouter(void *param);
    static void OnRecoveryFailed(void *param);
    static void OnPeriodicRestart(void *param);
    static void OnHWFailure(void *param);
    static H_Message AddToHistory(H_Message message, void *param);
    static H_Message OnStateDoNotihng(void *param);

private:
    StateMachine<H_Message, H_State> *m_pSM;
    int maxHistory;
    RecoverySource recoverySource;
    HistoryStorage storage;
    HistoryStorageItem *currStorageItem;
    time_t lastUpdate;
    
private:
    void AddHistoryItem(RecoverySource recoverySource);
    bool CreateHistoryItem(RecoverySource recoverySource);
    void AddToHistoryStorage(RecoveryStatus status, bool withEndTime = true);
};

extern HistoryControl historyControl;

#endif // HistoryControl_h