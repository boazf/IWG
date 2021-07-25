#include <ManualControl.h>
#include <TimeUtil.h>
#include <EthernetUtil.h>
#include <AppConfig.h>
#include <RecoveryControl.h>

void ManualControl::Init()
{
    Transition<MC_Message, MC_State> initTrans[] =
    {
        { MCM_Connected, MCS_Connected }
    };

    Transition<MC_Message, MC_State> transitions[] =
    {
        { MCM_CheckConnectivity, MCS_CheckConnectivity },
        { MCM_ModemRecovery, MCS_ModemRecovery },
        { MCM_RouterRecovery, MCS_RouterRecovery },
        { MCM_Unlock, MCS_Unlock },
        { MCM_Disconnected, MCS_Disconnected },
        { MCM_HWFailure, MCS_HWFailure },
        { MCM_RecoveryFailure, MCS_RecoveryFailure },
        { MCM_Connected, MCS_Connected }
    };

    SMState<MC_Message, MC_State> states[] =
    {
        SMState<MC_Message, MC_State>
        {
            MCS_Init,
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
            MCS_Connected,
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
            MCS_CheckConnectivity,
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
            MCS_ModemRecovery,
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
            MCS_RouterRecovery,
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
            MCS_Unlock,
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
            MCS_RecoveryFailure,
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
            MCS_Disconnected,
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
            MCS_HWFailure,
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
        return MCM_Connected;

    return MCM_None;
}

MC_Message ManualControl::transitionMessage(MC_State currState)
{
    switch(recoveryControl.GetRecoveryState())
    {
        case RecoveryTypes::ConnectivityCheck:
            if (currState != MCS_CheckConnectivity)
                return MCM_CheckConnectivity;
            break;

        case RecoveryTypes::Disconnected:
            if (currState != MCS_Disconnected)
                return MCM_Disconnected;
            break;

        case RecoveryTypes::Failed:
            if (currState != MCS_RecoveryFailure)
                return MCM_RecoveryFailure;
            break;

        case RecoveryTypes::HWFailure:
            if (currState != MCS_HWFailure)
                return MCM_HWFailure;
            break;

        case RecoveryTypes::Modem:
            if (currState != MCS_ModemRecovery)
                return MCM_ModemRecovery;
            break;

        case RecoveryTypes::NoRecovery:
            if (currState != MCS_Connected)
                return MCM_Connected;
            break;

        case RecoveryTypes::Router:
            if (currState != MCS_RouterRecovery)
                return MCM_RouterRecovery;
            break;
    }

    return MCM_None;
}

#define DO_TRANSITION(curr) { MC_Message msg = transitionMessage(curr); if (msg != MCM_None) return msg; }


void ManualControl::OnEnterState(void *param)
{
    ManualControl *mc = (ManualControl *)param;
    mc->t0 = millis();
    mc->rrState = rr.state();
    mc->mrState = mr.state();
    mc->ulState = ul.state();
    mc->ccState = cc.state();
    opi.set(LED_IDLE);
    uli.set(LED_IDLE);
    mri.set(LED_IDLE);
    rri.set(LED_IDLE);
    delay(5);
}

void ManualControl::OnEnterConnected(void *param)
{
    OnEnterState(param);
    opi.set(LED_ON);
}

MC_Message ManualControl::CheckUnlock(void *param)
{
    ManualControl *mc = (ManualControl *)param;

    if (ul.state() == BUTTON_ON)
    {
        if (mc->ulState == BUTTON_OFF && mr.state() == BUTTON_OFF && rr.state() == BUTTON_OFF)
        {
            return MCM_Unlock;
        }
    }
    else
        mc->ulState = BUTTON_OFF;

    return MCM_None;
}

MC_Message ManualControl::CheckCheckConnectivity(void *param)
{
    ManualControl *mc = (ManualControl *)param;

    if (cc.state() == BUTTON_ON)
    {
        if (mc->ccState == BUTTON_OFF)
        {
            mc->ccState = BUTTON_ON;
            recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
        }
    }
    else
    {
        mc->ccState = BUTTON_OFF;
    }

    return MCM_None;
}

MC_Message ManualControl::CheckButtons(void *param)
{
    MC_Message ulMsg = CheckUnlock(param);
    if (ulMsg != MCM_None)
        return ulMsg;

    return CheckCheckConnectivity(param);
}

MC_Message ManualControl::OnConnected(void *param)
{
    DO_TRANSITION(MCS_Connected);

    return CheckButtons(param);
}

void ManualControl::OnEnterCheckConnectivity(void *param)
{
    OnEnterState(param);
    opi.set(LED_BLINK);
}

MC_Message ManualControl::OnCheckConnectivity(void *param)
{
    DO_TRANSITION(MCS_CheckConnectivity);
    return MCM_None;
}

void ManualControl::OnEnterRecovery(void *param)
{
    OnEnterState(param);
    opi.set(LED_OFF);
    uli.set(LED_OFF);
    mri.set(LED_OFF);
    rri.set(LED_OFF);
    delay(5);
}

void ManualControl::OnEnterModemRecovery(void *param)
{
    OnEnterRecovery(param);
    mri.set(LED_BLINK);
}

MC_Message ManualControl::OnModemRecovery(void *param)
{
    DO_TRANSITION(MCS_ModemRecovery);

    return MCM_None;
}

void ManualControl::OnEnterRouterRecovery(void *param)
{
    OnEnterRecovery(param);
    rri.set(LED_BLINK);
}

MC_Message ManualControl::OnRouterRecovery(void *param)
{
    DO_TRANSITION(MCS_RouterRecovery);

    return MCM_None;
}

void ManualControl::OnEnterDisconnected(void *param)
{
    OnEnterState(param);
    opi.set(LED_OFF);
}

MC_Message ManualControl::OnDisconnected(void *param)
{
    DO_TRANSITION(MCS_Disconnected);

    return CheckButtons(param);
}

void ManualControl::OnEnterHWFailure(void *param)
{
    OnEnterRecovery(param);
}

MC_Message ManualControl::OnHWFailure(void *param)
{
    DO_TRANSITION(MCS_HWFailure);
    mri.set(LED_BLINK);
    rri.set(LED_BLINK);
    return MCM_None;
}

void ManualControl::OnEnterRecoveryFailure(void *param)
{
    OnEnterState(param);
    opi.set(LED_OFF);
}

MC_Message ManualControl::OnRecoveryFailure(void *param)
{
    DO_TRANSITION(MCS_RecoveryFailure);

    return CheckButtons(param);
}

void ManualControl::OnEnterUnlock(void *param)
{
    ledState opiState = opi.get();
    OnEnterState(param);
    opi.set(opiState);
    uli.set(LED_OFF);
}

MC_Message ManualControl::OnUnlock(void *param)
{
    ManualControl *mc = (ManualControl *)param;
        
    if (millis() - mc->t0 < 1000)
    {
        if (ul.state() == BUTTON_OFF || rr.state() == BUTTON_ON || mr.state() == BUTTON_ON)
            return MCM_Connected;
    }
    else if (millis() - mc->t0 < 4000)
    {
        mri.set(LED_ON);
        rri.set(LED_ON);
        uli.set(LED_ON);
        if (rr.state() == BUTTON_ON)
        {
            recoveryControl.StartRecoveryCycles(RecoveryTypes::Router);
            return transitionMessage(MCS_Unlock);
        }
        else if (mr.state() == BUTTON_ON)
        {
            recoveryControl.StartRecoveryCycles(RecoveryTypes::Modem);        
            return transitionMessage(MCS_Unlock);
        }
    }
    else
    {
        return MCM_Connected;
    }

    return MCM_None;
}


ManualControl manualControl;