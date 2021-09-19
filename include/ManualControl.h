#ifndef ManualControl_h
#define ManualControl_h

#include <StateMachine.h>
#include <Buttons.h>
#include <Indicators.h>

enum class MC_Message
{
    None = 0,
    Connected,
    CheckConnectivity,
    ModemRecovery,
    RouterRecovery,
    PeriodicRestart,
    RecoveryFailure,
    Unlock,
    Disconnected,
    HWFailure
};

enum class MC_State
{
    Init,
    Connected,
    CheckConnectivity,
    ModemRecovery,
    RouterRecovery,
    PeriodicRestart,
    RecoveryFailure,
    Disconnected,
    HWFailure,
    Unlock
};

class ManualControl
{
public:
    ManualControl() {};
    ~ManualControl() { delete m_pSM; };
    void Init();
    void PerformCycle();

private:
    static void OnEnterState(void *param);
    static MC_Message OnInit(void *param);
    static void OnEnterConnected(void *param);
    static MC_Message OnConnected(void *param);
    static void OnEnterCheckConnectivity(void *param);
    static MC_Message OnCheckConnectivity(void *param);
    static void OnEnterRecovery(void *param);
    static void OnEnterModemRecovery(void *param);
    static MC_Message OnModemRecovery(void *param);
    static void OnEnterRouterRecovery(void *param);
    static MC_Message OnRouterRecovery(void *param);
    static void OnEnterPeriodicRestart(void *param);
    static MC_Message OnPeriodicRestart(void *param);
    static void OnEnterUnlock(void *param);
    static MC_Message OnUnlock(void *param);
    static void OnEnterDisconnected(void *param);
    static MC_Message OnDisconnected(void *param);
    static void OnEnterHWFailure(void *param);
    static MC_Message OnHWFailure(void *param);
    static void OnEnterRecoveryFailure(void *param);
    static MC_Message OnRecoveryFailure(void *param);
    static MC_Message transitionMessage(MC_State currState);
    static MC_Message CheckUnlock(void *param);
    static MC_Message CheckCheckConnectivity(void *param);
    static MC_Message CheckButtons(void *param);

private:
	StateMachine<MC_Message, MC_State> *m_pSM;
    unsigned long t0;
    buttonState mrState;
    buttonState ulState;
    buttonState rrState;
    buttonState ccState;
};

extern ManualControl manualControl;

#endif // ManualControl_h