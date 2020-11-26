#ifndef HistoryControl_h
#define HistoryControl_h

#include <StateMachine.h>
#include <RecoveryControl.h>
#include <HistoryStorage.h>

enum H_Message
{
    HM_None = 0,
    HM_CheckConnectivity,
    HM_RouterRecovery,
    HM_ModemRecovery,
    HM_RecoveryFailure,
    HM_RecoverySuccess,
    HM_HWFailure
};

enum H_State
{
    HS_Connected,
    HS_CheckingConnectivity,
    HS_RecoveringRouter,
    HS_RecoveringModem,
    HS_RecoveryFailed,
    HS_CheckingConnectivityWhileInFailure,
    HS_HWFailure
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
    time_t getLastRecovery() { return storage.getLastRecovery(); }

private:
    static void RecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context);
    static void MaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context);
    static void OnConnected(void *param);
    static void OnCheckingConnectivity(void *param);
    static void OnRecoveringModem(void *param);
    static void OnRecoveringRouter(void *param);
    static void OnRecoveryFailed(void *param);
    static void OnHWFailure(void *param);
    static H_Message AddToHistory(H_Message message, void *param);
    static H_Message OnStateDoNotihng(void *param);

private:
    StateMachine<H_Message, H_State> *m_pSM;
    int maxHistory;
    bool byUser;
    HistoryStorage storage;
    HistoryStorageItem *currStorageItem;
    void AddHistoryItem(bool byUser);
    bool CreateHistoryItem(RecoverySource recoverySource);
    void AddToHistoryStorage(RecoveryStatus status, bool withEndTime = true);
};

extern HistoryControl historyControl;

#endif // HistoryControl_h