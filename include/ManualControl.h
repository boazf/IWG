#ifndef ManualControl_h
#define ManualControl_h

#include <StateMachine.h>
#include <Buttons.h>
#include <Indicators.h>


#define MC_Messages \
    X(None) \
    X(Connected) \
    X(CheckConnectivity) \
    X(ModemRecovery) \
    X(RouterRecovery) \
    X(PeriodicRestart) \
    X(RecoveryFailure) \
    X(Unlock) \
    X(Disconnected) \
    X(HWFailure)

#define MC_States \
    X(Init) \
    X(Connected) \
    X(CheckConnectivity) \
    X(ModemRecovery) \
    X(RouterRecovery) \
    X(PeriodicRestart) \
    X(RecoveryFailure) \
    X(Disconnected) \
    X(HWFailure) \
    X(Unlock)

#define X(a) a,
enum class MC_Message
{
    MC_Messages
};

enum class MC_State
{
    MC_States
};
#undef X

class ManualControl
{
public:
    ManualControl() : m_pSM(NULL) {};
    ~ManualControl() { delete m_pSM; };
    void Init();
    void PerformCycle();

private:
    void OnEnterState();
    MC_Message CheckUnlock();
    MC_Message CheckCheckConnectivity();
    MC_Message CheckButtons();

private:
    static MC_Message OnInit(ManualControl *control);
    static void OnEnterConnected(ManualControl *control);
    static MC_Message OnConnected(ManualControl *control);
    static void OnEnterCheckConnectivity(ManualControl *control);
    static MC_Message OnCheckConnectivity(ManualControl *control);
    static void OnEnterRecovery(ManualControl *control);
    static void OnEnterModemRecovery(ManualControl *control);
    static MC_Message OnModemRecovery(ManualControl *control);
    static void OnEnterRouterRecovery(ManualControl *control);
    static MC_Message OnRouterRecovery(ManualControl *control);
    static void OnEnterPeriodicRestart(ManualControl *control);
    static MC_Message OnPeriodicRestart(ManualControl *control);
    static void OnEnterUnlock(ManualControl *control);
    static MC_Message OnUnlock(ManualControl *control);
    static void OnEnterDisconnected(ManualControl *control);
    static MC_Message OnDisconnected(ManualControl *control);
    static void OnEnterHWFailure(ManualControl *control);
    static MC_Message OnHWFailure(ManualControl *control);
    static void OnEnterRecoveryFailure(ManualControl *control);
    static MC_Message OnRecoveryFailure(ManualControl *control);
    static MC_Message transitionMessage(MC_State currState);

private:
	StateMachine<MC_Message, MC_State, ManualControl> *m_pSM;
    unsigned long t0;
    buttonState mrState;
    buttonState ulState;
    buttonState rrState;
    buttonState ccState;
};

extern ManualControl manualControl;

#endif // ManualControl_h