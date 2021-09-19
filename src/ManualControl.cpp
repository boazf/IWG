#include <ManualControl.h>
#include <TimeUtil.h>
#include <EthernetUtil.h>
#include <AppConfig.h>
#include <RecoveryControl.h>

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

    SMState<MC_Message, MC_State> states[] =
    {
        SMState<MC_Message, MC_State>
        {
            MC_State::Init,
            SMState<MC_Message, MC_State>::OnEnterDoNothing,
            OnInit,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(initTrans)
#ifdef DEBUG_STATE_MACHINE
			, "Init"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::Connected,
            OnEnterConnected,
            OnConnected,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "Connected"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::CheckConnectivity,
            OnEnterCheckConnectivity,
            OnCheckConnectivity,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnctivity"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::ModemRecovery,
            OnEnterModemRecovery,
            OnModemRecovery,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "ModemRecovery"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::RouterRecovery,
            OnEnterRouterRecovery,
            OnRouterRecovery,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "RouterRecovery"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::PeriodicRestart,
            OnEnterPeriodicRestart,
            OnPeriodicRestart,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "RouterRecovery"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::Unlock,
            OnEnterUnlock,
            OnUnlock,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "Unlock"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::RecoveryFailure,
            OnEnterRecoveryFailure,
            OnRecoveryFailure,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "RecoveryFailure"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::Disconnected,
            OnEnterDisconnected,
            OnDisconnected,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "Disconnected"
#endif
        },

        SMState<MC_Message, MC_State>
        {
            MC_State::HWFailure,
            OnEnterHWFailure,
            OnHWFailure,
            SMState<MC_Message, MC_State>::OnExitDoNothing,
            TRANSITIONS(transitions)
#ifdef DEBUG_STATE_MACHINE
			, "HWFailure"
#endif
        }
    };

	m_pSM = new StateMachine<MC_Message, MC_State>(states, NELEMS(states), this
#ifdef DEBUG_STATE_MACHINE
			, "MenualControl"
#endif
        );
}

void ManualControl::PerformCycle()
{
	m_pSM->HandleState();
}

MC_Message ManualControl::OnInit(void *param)
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


void ManualControl::OnEnterState(void *param)
{
    ManualControl *mc = (ManualControl *)param;
    mc->t0 = millis();
    mc->rrState = rr.state();
    mc->mrState = mr.state();
    mc->ulState = ul.state();
    mc->ccState = cc.state();
    opi.set(ledState::LED_IDLE);
    uli.set(ledState::LED_IDLE);
    mri.set(ledState::LED_IDLE);
    rri.set(ledState::LED_IDLE);
    delay(5);
}

void ManualControl::OnEnterConnected(void *param)
{
    OnEnterState(param);
    opi.set(ledState::LED_ON);
}

MC_Message ManualControl::CheckUnlock(void *param)
{
    ManualControl *mc = (ManualControl *)param;

    if (ul.state() == buttonState::BUTTON_ON)
    {
        if (mc->ulState == buttonState::BUTTON_OFF && mr.state() == buttonState::BUTTON_OFF && rr.state() == buttonState::BUTTON_OFF)
        {
            return MC_Message::Unlock;
        }
    }
    else
        mc->ulState = buttonState::BUTTON_OFF;

    return MC_Message::None;
}

MC_Message ManualControl::CheckCheckConnectivity(void *param)
{
    ManualControl *mc = (ManualControl *)param;

    if (cc.state() == buttonState::BUTTON_ON)
    {
        if (mc->ccState == buttonState::BUTTON_OFF)
        {
            mc->ccState = buttonState::BUTTON_ON;
            recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
        }
    }
    else
    {
        mc->ccState = buttonState::BUTTON_OFF;
    }

    return MC_Message::None;
}

MC_Message ManualControl::CheckButtons(void *param)
{
    MC_Message ulMsg = CheckUnlock(param);
    if (ulMsg != MC_Message::None)
        return ulMsg;

    return CheckCheckConnectivity(param);
}

MC_Message ManualControl::OnConnected(void *param)
{
    DO_TRANSITION(MC_State::Connected);

    return CheckButtons(param);
}

void ManualControl::OnEnterCheckConnectivity(void *param)
{
    OnEnterState(param);
    opi.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnCheckConnectivity(void *param)
{
    DO_TRANSITION(MC_State::CheckConnectivity);
    return MC_Message::None;
}

void ManualControl::OnEnterRecovery(void *param)
{
    OnEnterState(param);
    opi.set(ledState::LED_OFF);
    uli.set(ledState::LED_OFF);
    mri.set(ledState::LED_OFF);
    rri.set(ledState::LED_OFF);
    delay(5);
}

void ManualControl::OnEnterModemRecovery(void *param)
{
    OnEnterRecovery(param);
    mri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnModemRecovery(void *param)
{
    DO_TRANSITION(MC_State::ModemRecovery);

    return MC_Message::None;
}

void ManualControl::OnEnterRouterRecovery(void *param)
{
    OnEnterRecovery(param);
    rri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnRouterRecovery(void *param)
{
    DO_TRANSITION(MC_State::RouterRecovery);

    return MC_Message::None;
}

void ManualControl::OnEnterPeriodicRestart(void *param)
{
    OnEnterRecovery(param);
    if (AppConfig::getPeriodicallyRestartRouter())
        rri.set(ledState::LED_BLINK);
    if (AppConfig::getPeriodicallyRestartModem())
        mri.set(ledState::LED_BLINK);
}

MC_Message ManualControl::OnPeriodicRestart(void *param)
{
    DO_TRANSITION(MC_State::PeriodicRestart);

    return MC_Message::None;
}

void ManualControl::OnEnterDisconnected(void *param)
{
    OnEnterState(param);
    opi.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnDisconnected(void *param)
{
    DO_TRANSITION(MC_State::Disconnected);

    return CheckButtons(param);
}

void ManualControl::OnEnterHWFailure(void *param)
{
    OnEnterRecovery(param);
}

MC_Message ManualControl::OnHWFailure(void *param)
{
    DO_TRANSITION(MC_State::HWFailure);
    mri.set(ledState::LED_BLINK);
    rri.set(ledState::LED_BLINK);
    return MC_Message::None;
}

void ManualControl::OnEnterRecoveryFailure(void *param)
{
    OnEnterState(param);
    opi.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnRecoveryFailure(void *param)
{
    DO_TRANSITION(MC_State::RecoveryFailure);

    return CheckButtons(param);
}

void ManualControl::OnEnterUnlock(void *param)
{
    ledState opiState = opi.get();
    OnEnterState(param);
    opi.set(opiState);
    uli.set(ledState::LED_OFF);
}

MC_Message ManualControl::OnUnlock(void *param)
{
    ManualControl *mc = (ManualControl *)param;
        
    if (millis() - mc->t0 < 1000)
    {
        if (ul.state() == buttonState::BUTTON_OFF || rr.state() == buttonState::BUTTON_ON || mr.state() == buttonState::BUTTON_ON)
            return MC_Message::Connected;
    }
    else if (millis() - mc->t0 < 4000)
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


ManualControl manualControl;