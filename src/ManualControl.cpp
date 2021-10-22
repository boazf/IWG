#include <ManualControl.h>
#include <TimeUtil.h>
#include <EthernetUtil.h>
#include <AppConfig.h>
#include <RecoveryControl.h>
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#endif

void ManualControl::Init()
{
    Transition<MC_Message, MC_State> initTrans[] =
    {
        { MC_Message::Connected, MC_State::Connected }
    };

    Transition<MC_Message, MC_State> transitions[] =
    {
        { MC_Message::CheckConnectivity, MC_State::CheckConnectivity },
        { MC_Message::ModemRecovery, MC_State::ModemRecovery },
        { MC_Message::RouterRecovery, MC_State::RouterRecovery },
        { MC_Message::Unlock, MC_State::Unlock },
        { MC_Message::Disconnected, MC_State::Disconnected },
        { MC_Message::HWFailure, MC_State::HWFailure },
        { MC_Message::RecoveryFailure, MC_State::RecoveryFailure },
        { MC_Message::Connected, MC_State::Connected },
        { MC_Message::PeriodicRestart, MC_State::PeriodicRestart }
    };

    typedef SMState<MC_Message, MC_State, ManualControl> ManualState;
    
    ManualState states[] =
    {
        ManualState
        {
            MC_State::Init,
            ManualState::OnEnterDoNothing,
            OnInit,
            ManualState::OnExitDoNothing,
            TRANSITIONS(initTrans)
        },

        ManualState
        {
            MC_State::Connected,
            OnEnterConnected,
            OnConnected,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::CheckConnectivity,
            OnEnterCheckConnectivity,
            OnCheckConnectivity,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::ModemRecovery,
            OnEnterModemRecovery,
            OnModemRecovery,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::RouterRecovery,
            OnEnterRouterRecovery,
            OnRouterRecovery,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::PeriodicRestart,
            OnEnterPeriodicRestart,
            OnPeriodicRestart,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::Unlock,
            OnEnterUnlock,
            OnUnlock,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::RecoveryFailure,
            OnEnterRecoveryFailure,
            OnRecoveryFailure,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::Disconnected,
            OnEnterDisconnected,
            OnDisconnected,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        },

        ManualState
        {
            MC_State::HWFailure,
            OnEnterHWFailure,
            OnHWFailure,
            ManualState::OnExitDoNothing,
            TRANSITIONS(transitions)
        }
    };

	m_pSM = new StateMachine<MC_Message, MC_State, ManualControl>(states, NELEMS(states), this
#ifdef DEBUG_STATE_MACHINE
			, "MenualControl"
#endif
        );
}

void ManualControl::PerformCycle()
{
	m_pSM->HandleState();
}

MC_Message ManualControl::OnInit(ManualControl *control)
{
    if (recoveryControl.GetRecoveryState() != RecoveryTypes::ConnectivityCheck)
        return MC_Message::Connected;

    return MC_Message::None;
}

MC_Message ManualControl::transitionMessage(MC_State currState)
{
    switch(recoveryControl.GetRecoveryState())
    {
        case RecoveryTypes::ConnectivityCheck:
            if (currState != MC_State::CheckConnectivity)
                return MC_Message::CheckConnectivity;
            break;

        case RecoveryTypes::Disconnected:
            if (currState != MC_State::Disconnected)
                return MC_Message::Disconnected;
            break;

        case RecoveryTypes::Failed:
            if (currState != MC_State::RecoveryFailure)
                return MC_Message::RecoveryFailure;
            break;

        case RecoveryTypes::HWFailure:
            if (currState != MC_State::HWFailure)
                return MC_Message::HWFailure;
            break;

        case RecoveryTypes::Modem:
            if (currState != MC_State::ModemRecovery)
                return MC_Message::ModemRecovery;
            break;

        case RecoveryTypes::NoRecovery:
            if (currState != MC_State::Connected)
                return MC_Message::Connected;
            break;

        case RecoveryTypes::Router:
            if (currState != MC_State::RouterRecovery)
                return MC_Message::RouterRecovery;
            break;

        case RecoveryTypes::Periodic:
            if (currState != MC_State::PeriodicRestart)
                return MC_Message::PeriodicRestart;
            break;
    }

    return MC_Message::None;
}

#define DO_TRANSITION(curr) { MC_Message msg = transitionMessage(curr); if (msg != MC_Message::None) return msg; }


void ManualControl::OnEnterState(ManualControl *control)
{
    control->t0 = millis();
    control->rrState = rr.state();
    control->mrState = mr.state();
    control->ulState = ul.state();
    control->ccState = cc.state();
    opi.set(ledState::LED_IDLE);
    uli.set(ledState::LED_IDLE);
    mri.set(ledState::LED_IDLE);
    rri.set(ledState::LED_IDLE);
    delay(5);
}

void ManualControl::OnEnterConnected(ManualControl *control)
{
    OnEnterState(control);
    opi.set(ledState::LED_ON);
}

MC_Message ManualControl::CheckUnlock(ManualControl *control)
{
    if (ul.state() == buttonState::BUTTON_ON)
    {
        if (control->ulState == buttonState::BUTTON_OFF && mr.state() == buttonState::BUTTON_OFF && rr.state() == buttonState::BUTTON_OFF)
        {
            return MC_Message::Unlock;
        }
    }
    else
        control->ulState = buttonState::BUTTON_OFF;

    return MC_Message::None;
}

MC_Message ManualControl::CheckCheckConnectivity(ManualControl *control)
{
    if (cc.state() == buttonState::BUTTON_ON)
    {
        if (control->ccState == buttonState::BUTTON_OFF)
        {
            control->ccState = buttonState::BUTTON_ON;
            recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
        }
    }
    else
    {
        control->ccState = buttonState::BUTTON_OFF;
    }

    return MC_Message::None;
}

MC_Message ManualControl::CheckButtons(ManualControl *control)
{
    MC_Message ulMsg = CheckUnlock(control);
    if (ulMsg != MC_Message::None)
        return ulMsg;

    return CheckCheckConnectivity(control);
}

MC_Message ManualControl::OnConnected(ManualControl *control)
{
    DO_TRANSITION(MC_State::Connected);

    return CheckButtons(control);
}

void ManualControl::OnEnterCheckConnectivity(ManualControl *control)
{
    OnEnterState(control);
    opi.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnCheckConnectivity(ManualControl *control)
{
    DO_TRANSITION(MC_State::CheckConnectivity);
    return MC_Message::None;
}

void ManualControl::OnEnterRecovery(ManualControl *control)
{
    OnEnterState(control);
    opi.set(ledState::LED_OFF);
    uli.set(ledState::LED_OFF);
    mri.set(ledState::LED_OFF);
    rri.set(ledState::LED_OFF);
    delay(5);
}

void ManualControl::OnEnterModemRecovery(ManualControl *control)
{
    OnEnterRecovery(control);
    mri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnModemRecovery(ManualControl *control)
{
    DO_TRANSITION(MC_State::ModemRecovery);

    return MC_Message::None;
}

void ManualControl::OnEnterRouterRecovery(ManualControl *control)
{
    OnEnterRecovery(control);
    rri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnRouterRecovery(ManualControl *control)
{
    DO_TRANSITION(MC_State::RouterRecovery);

    return MC_Message::None;
}

void ManualControl::OnEnterPeriodicRestart(ManualControl *control)
{
    OnEnterRecovery(control);
    if (AppConfig::getPeriodicallyRestartRouter())
        rri.set(ledState::LED_BLINK);
    if (AppConfig::getPeriodicallyRestartModem())
        mri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnPeriodicRestart(ManualControl *control)
{
    DO_TRANSITION(MC_State::PeriodicRestart);

    return MC_Message::None;
}

void ManualControl::OnEnterDisconnected(ManualControl *control)
{
    OnEnterState(control);
    opi.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnDisconnected(ManualControl *control)
{
    DO_TRANSITION(MC_State::Disconnected);

    return CheckButtons(control);
}

void ManualControl::OnEnterHWFailure(ManualControl *control)
{
    OnEnterRecovery(control);
}

MC_Message ManualControl::OnHWFailure(ManualControl *control)
{
    DO_TRANSITION(MC_State::HWFailure);
    mri.set(ledState::LED_BLINK);
    rri.set(ledState::LED_BLINK);
    return MC_Message::None;
}

void ManualControl::OnEnterRecoveryFailure(ManualControl *control)
{
    OnEnterState(control);
    opi.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnRecoveryFailure(ManualControl *control)
{
    DO_TRANSITION(MC_State::RecoveryFailure);

    return CheckButtons(control);
}

void ManualControl::OnEnterUnlock(ManualControl *control)
{
    ledState opiState = opi.get();
    OnEnterState(control);
    opi.set(opiState);
    uli.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnUnlock(ManualControl *control)
{
    if (millis() - control->t0 < 1000)
    {
        if (ul.state() == buttonState::BUTTON_OFF || rr.state() == buttonState::BUTTON_ON || mr.state() == buttonState::BUTTON_ON)
            return MC_Message::Connected;
    }
    else if (millis() - control->t0 < 4000)
    {
        mri.set(ledState::LED_ON);
        rri.set(ledState::LED_ON);
        uli.set(ledState::LED_ON);
        if (rr.state() == buttonState::BUTTON_ON)
        {
            recoveryControl.StartRecoveryCycles(RecoveryTypes::Router);
            return transitionMessage(MC_State::Unlock);
        }
        else if (mr.state() == buttonState::BUTTON_ON)
        {
            recoveryControl.StartRecoveryCycles(RecoveryTypes::Modem);        
            return transitionMessage(MC_State::Unlock);
        }
    }
    else
    {
        return MC_Message::Connected;
    }

    return MC_Message::None;
}

#ifdef DEBUG_STATE_MACHINE
#define X(a) {MC_State::a, #a},
template<>
const std::map<MC_State, std::string> StringableEnum<MC_State>::strMap = 
{
    MC_States
};
#undef X

#define X(a) {MC_Message::a, #a},
template<>
const std::map<MC_Message, std::string> StringableEnum<MC_Message>::strMap = 
{
    MC_Messages
};
#undef X
#endif

ManualControl manualControl;