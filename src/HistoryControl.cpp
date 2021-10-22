#include <HistoryControl.h>
#include <AppConfig.h>
#include <Common.h>
#include <TimeUtil.h>
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#endif
HistoryControl::HistoryControl() :
    currStorageItem(NULL)
{
}

void HistoryControl::Init()
{
    recoveryControl.GetRecoveryStateChanged().addObserver(RecoveryStateChanged, this);
    maxHistory = AppConfig::getMaxHistory();
    recoveryControl.GetMaxHistoryRecordsChanged().addObserver(MaxHistoryChanged, this);

    typedef Transition<H_Message, H_State> HistoryTransition;

    HistoryTransition initTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected }
    };

    HistoryTransition connectedTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::CheckConnectivity, H_State::CheckingConnectivity },
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::PeriodicRestart, H_State::PeriodicRestart },
        { H_Message::HWFailure, H_State::HWFailure }
    };

    HistoryTransition checkingConnectivityTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter}
    };

    HistoryTransition recoveringModemTrans[] =
    {
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoveryFailure, H_State::RecoveryFailed },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::HWFailure, H_State::HWFailure},
    };

    HistoryTransition recoveringRouterTrans[] =
    { 
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoveryFailure, H_State::RecoveryFailed },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::HWFailure, H_State::HWFailure}
    };

    HistoryTransition periodicRestartTrans[] =
    { 
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoveryFailure, H_State::RecoveryFailed },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::HWFailure, H_State::HWFailure}
    };

    HistoryTransition recoveryFailedTrans[] =
    {
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::CheckConnectivity, H_State::CheckingConnectivityWhileInFailure}
    };

    HistoryTransition checkingConnectivityWhileInFailureTrans[] =
    {
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoverySuccess, H_State::Connected}
    };

    HistoryTransition HWFailureTrans[] =
    { 
        { H_Message::CheckConnectivity, H_State::CheckingConnectivity }
    };

    typedef SMState<H_Message, H_State, HistoryControl> HistoryState;

    HistoryState states[] =
    {
        HistoryState(
            H_State::Init, 
            HistoryState::OnEnterDoNothing, 
            OnInit,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(initTrans)),
        HistoryState(
            H_State::Connected, 
            OnConnected, 
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(connectedTrans)),
        HistoryState(
            H_State::CheckingConnectivity, 
            OnCheckingConnectivity,
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(checkingConnectivityTrans)),
        HistoryState(
            H_State::RecoveringModem, 
            OnRecoveringModem,
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(recoveringModemTrans)),
        HistoryState(
            H_State::RecoveringRouter, 
            OnRecoveringRouter,
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(recoveringRouterTrans)),
        HistoryState(
            H_State::RecoveryFailed, 
            OnRecoveryFailed,
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(recoveryFailedTrans)),
        HistoryState(
            H_State::CheckingConnectivityWhileInFailure, 
            OnCheckingConnectivity, 
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(checkingConnectivityWhileInFailureTrans)),
        HistoryState(
            H_State::PeriodicRestart, 
            OnPeriodicRestart, 
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(periodicRestartTrans)),
        HistoryState(
            H_State::HWFailure, 
            OnHWFailure,
            OnStateDoNotihng,
   			HistoryState::OnExitDoNothing,
            TRANSITIONS(HWFailureTrans)),
    };
  	m_pSM = new StateMachine<H_Message, H_State, HistoryControl>(states, NELEMS(states), this
#ifdef DEBUG_STATE_MACHINE
			, "History"
#endif
      );
    storage.init(maxHistory);
    lastUpdate = t_now;
}

HistoryControl::~HistoryControl()
{
    delete m_pSM;
}

time_t HistoryControl::getLastRecovery()
{ 
    return storage.getLastRecovery(); 
}

time_t HistoryControl::getLastUpdate()
{
    return lastUpdate;
}

void HistoryControl::PerformCycle()
{
    m_pSM->HandleState();
}

void HistoryControl::RecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context)
{
    HistoryControl *historyControl = (HistoryControl *)context;

    if (historyControl->m_pSM->current()->getState() == H_State::Init)
        return;

    historyControl->recoverySource = params.m_source;

    switch(params.m_recoveryType)
    {
    case RecoveryTypes::ConnectivityCheck:
        historyControl->m_pSM->ApplyVerb(H_Message::CheckConnectivity);
        break;
    case RecoveryTypes::Failed:
        historyControl->m_pSM->ApplyVerb(H_Message::RecoveryFailure);
        break;
    case RecoveryTypes::Modem:
        historyControl->m_pSM->ApplyVerb(H_Message::ModemRecovery);
        break;
    case RecoveryTypes::Router:
        historyControl->m_pSM->ApplyVerb(H_Message::RouterRecovery);
        break;
    case RecoveryTypes::HWFailure:
        historyControl->m_pSM->ApplyVerb(H_Message::HWFailure);
        break;
    case RecoveryTypes::NoRecovery:
        historyControl->m_pSM->ApplyVerb(H_Message::RecoverySuccess);
        break;
    case RecoveryTypes::Disconnected:
        break;
    case RecoveryTypes::Periodic:
        historyControl->m_pSM->ApplyVerb(H_Message::PeriodicRestart);
        break;
    }
}

H_Message HistoryControl::OnInit(HistoryControl *control)
{
    if (recoveryControl.GetRecoveryState() == RecoveryTypes::NoRecovery)
        return H_Message::RecoverySuccess;
    
    return H_Message::None;
}

void HistoryControl::MaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context)
{
    HistoryControl *historyControl = (HistoryControl *)context;
    historyControl->maxHistory = params.m_maxRecords;
    historyControl->storage.resize(historyControl->maxHistory);
    historyControl->lastUpdate = t_now;
}

H_Message HistoryControl::OnStateDoNotihng(HistoryControl *control)
{
    return H_Message::None;
}

void HistoryControl::OnConnected(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryStorageItem *currStorageItem = control->currStorageItem;
    if (currStorageItem != NULL)
    {
        if (currStorageItem->routerRecoveries() > 0 || currStorageItem->modemRecoveries() > 0)
        {
            control->AddToHistoryStorage(RecoveryStatus::RecoverySuccess);
        }
        else
        {
            delete currStorageItem;
            control->currStorageItem = NULL;
        }
    }
}

void HistoryControl::OnCheckingConnectivity(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    control->CreateHistoryItem(control->recoverySource);
}

void HistoryControl::OnRecoveringModem(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    control->AddHistoryItem(control->recoverySource);
    control->currStorageItem->modemRecoveries()++;
    control->lastUpdate = t_now;
}

void HistoryControl::OnRecoveringRouter(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif

    control->AddHistoryItem(control->recoverySource);
    control->currStorageItem->routerRecoveries()++;
    control->lastUpdate = t_now;
}

void HistoryControl::OnPeriodicRestart(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif

    control->AddHistoryItem(control->recoverySource);
    if (AppConfig::getPeriodicallyRestartRouter())
        control->currStorageItem->routerRecoveries()++;
    if (AppConfig::getPeriodicallyRestartModem())
        control->currStorageItem->modemRecoveries()++;
    control->lastUpdate = t_now;
}

void HistoryControl::OnRecoveryFailed(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    control->AddToHistoryStorage(RecoveryStatus::RecoveryFailure);
}

void HistoryControl::OnHWFailure(HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    delete control->currStorageItem;
    control->currStorageItem = NULL;
    control->lastUpdate = t_now;
}

H_Message HistoryControl::AddToHistory(H_Message message, HistoryControl *control)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    if (message == H_Message::RecoverySuccess)
    {
        control->AddHistoryItem(control->recoverySource);
        control->AddToHistoryStorage(RecoveryStatus::RecoverySuccess, false);
    }

    return message;
}

void HistoryControl::AddHistoryItem(RecoverySource recoverySource)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    CreateHistoryItem(recoverySource);
}

bool HistoryControl::CreateHistoryItem(RecoverySource recoverySource)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    if (currStorageItem != NULL)
        return false;

    currStorageItem = new HistoryStorageItem(recoverySource, t_now, INT32_MAX, 0, 0, RecoveryStatus::OnGoingRecovery);
    lastUpdate = t_now;
    return true;
}

void HistoryControl::AddToHistoryStorage(RecoveryStatus status, bool withEndTime)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    currStorageItem->recoveryStatus() = status;
    if (withEndTime)
    {
        currStorageItem->endTime() = t_now;
    }
    storage.addHistory(*currStorageItem);
    delete currStorageItem;
    currStorageItem = NULL;
    lastUpdate = t_now;
}

int HistoryControl::Available()
{
    if (storage.available() < maxHistory)
    {
        return storage.available() + (currStorageItem == NULL ? 0 : 1);
    }

    return storage.available();
}

const HistoryStorageItem HistoryControl::GetHistoryItem(int index)
{
    if (currStorageItem == NULL)
        return storage.getItem(index);

    if (index == Available() - 1)
        return *currStorageItem;

    return storage.getItem(index + 1 - (storage.available() < maxHistory));
}

#ifdef DEBUG_STATE_MACHINE
#define X(a) {H_State::a, #a},
template<>
const std::map<H_State, std::string> StringableEnum<H_State>::strMap = 
{
    H_States
};
#undef X

#define X(a) {H_Message::a, #a},
template<>
const std::map<H_Message, std::string> StringableEnum<H_Message>::strMap = 
{
    H_Messages
};
#undef X
#endif

HistoryControl historyControl;
