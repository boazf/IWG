#ifndef HistoryControl_h
#define HistoryControl_h

#include <StateMachine.h>
#include <RecoveryControl.h>
#include <HistoryStorage.h>

#define H_Messages \
    X(None) \
    X(CheckConnectivity) \
    X(RouterRecovery) \
    X(ModemRecovery) \
    X(RecoveryFailure) \
    X(RecoverySuccess) \
    X(PeriodicRestart) \
    X(HWFailure)

#define H_States \
    X(Init) \
    X(Connected) \
    X(CheckingConnectivity) \
    X(RecoveringRouter) \
    X(RecoveringModem) \
    X(RecoveryFailed) \
    X(CheckingConnectivityWhileInFailure) \
    X(PeriodicRestart) \
    X(HWFailure)

#define X(a) a,
enum class H_Message
{
    H_Messages
};

enum class H_State
{
    H_States
};
#undef X

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
    static H_Message OnInit(HistoryControl *param);
    static void OnConnected(HistoryControl *param);
    static void OnCheckingConnectivity(HistoryControl *param);
    static void OnRecoveringModem(HistoryControl *param);
    static void OnRecoveringRouter(HistoryControl *param);
    static void OnRecoveryFailed(HistoryControl *param);
    static void OnPeriodicRestart(HistoryControl *param);
    static void OnHWFailure(HistoryControl *param);
    static H_Message AddToHistory(H_Message message, HistoryControl *param);
    static H_Message OnStateDoNotihng(HistoryControl *param);

private:
    StateMachine<H_Message, H_State, HistoryControl> *m_pSM;
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