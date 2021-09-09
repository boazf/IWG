#include <HistoryControl.h>
#include <AppConfig.h>
#include <Common.h>
#include <TimeUtil.h>

HistoryControl::HistoryControl() :
    currStorageItem(NULL)
{
}

void HistoryControl::Init()
{
    recoveryControl.GetRecoveryStateChanged().addObserver(RecoveryStateChanged, this);
    maxHistory = AppConfig::getMaxHistory();
    recoveryControl.GetMaxHistoryRecordsChanged().addObserver(MaxHistoryChanged, this);

    Transition<H_Message, H_State> initTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected }
    };

    Transition<H_Message, H_State> connectedTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::CheckConnectivity, H_State::CheckingConnectivity },
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::HWFailure, H_State::HWFailure }
    };

    Transition<H_Message, H_State> checkingConnectivityTrans[] =
    {
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter}
    };

    Transition<H_Message, H_State> recoveringModemTrans[] =
    {
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoveryFailure, H_State::RecoveryFailed },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::HWFailure, H_State::HWFailure},
    };

    Transition<H_Message, H_State> recoveringRouterTrans[] =
    { 
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RecoveryFailure, H_State::RecoveryFailed },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::HWFailure, H_State::HWFailure}
    };

    Transition<H_Message, H_State> recoveryFailedTrans[] =
    {
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoverySuccess, H_State::Connected },
        { H_Message::CheckConnectivity, H_State::CheckingConnectivityWhileInFailure}
    };

    Transition<H_Message, H_State> checkingConnectivityWhileInFailureTrans[] =
    {
        { H_Message::ModemRecovery, H_State::RecoveringModem },
        { H_Message::RouterRecovery, H_State::RecoveringRouter },
        { H_Message::RecoverySuccess, H_State::Connected}
    };

    Transition<H_Message, H_State> HWFailureTrans[] =
    { 
        { H_Message::CheckConnectivity, H_State::CheckingConnectivity }
    };

    SMState<H_Message, H_State> states[] =
    {
        SMState<H_Message, H_State>(
            H_State::Init, 
            SMState<H_Message, H_State>::OnEnterDoNothing, 
            OnInit,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(initTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::Init"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::Connected, 
            OnConnected, 
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(connectedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::Connected"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::CheckingConnectivity, 
            OnCheckingConnectivity,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(checkingConnectivityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::CheckingConnectivity"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::RecoveringModem, 
            OnRecoveringModem,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(recoveringModemTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::RecoveringModem"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::RecoveringRouter, 
            OnRecoveringRouter,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(recoveringRouterTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::RecoveringRouter"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::RecoveryFailed, 
            OnRecoveryFailed,
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(recoveryFailedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::RecoveryFailed"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::CheckingConnectivityWhileInFailure, 
            OnCheckingConnectivity, 
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(checkingConnectivityWhileInFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::CheckingConnectivityWhileInFailure"
#endif
			),
        SMState<H_Message, H_State>(
            H_State::HWFailure, 
            OnHWFailure,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(HWFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "H_State::HWFailure"
#endif
			)
    };
  	m_pSM = new StateMachine<H_Message, H_State>(states, NELEMS(states), this
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

    if (historyControl->m_pSM->current()->State() == H_State::Init)
        return;

    historyControl->byUser = params.m_byUser;

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
    }
}

H_Message HistoryControl::OnInit(void *param)
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

H_Message HistoryControl::OnStateDoNotihng(void *param)
{
    return H_Message::None;
}

void HistoryControl::OnConnected(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;
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

void HistoryControl::OnCheckingConnectivity(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->CreateHistoryItem(control->byUser ? RecoverySource::UserInitiatedRecovery : RecoverySource::AutoRecovery);
}

void HistoryControl::OnRecoveringModem(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddHistoryItem(control->byUser);
    control->currStorageItem->modemRecoveries()++;
    control->lastUpdate = t_now;
}

void HistoryControl::OnRecoveringRouter(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddHistoryItem(control->byUser);
    control->currStorageItem->routerRecoveries()++;
    control->lastUpdate = t_now;
}

void HistoryControl::OnRecoveryFailed(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddToHistoryStorage(RecoveryStatus::RecoveryFailure);
}

void HistoryControl::OnHWFailure(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    delete control->currStorageItem;
    control->currStorageItem = NULL;
    control->lastUpdate = t_now;
}

H_Message HistoryControl::AddToHistory(H_Message message, void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    if (message == H_Message::RecoverySuccess)
    {
        HistoryControl *control = (HistoryControl *)param;
        control->AddHistoryItem(control->byUser);
        control->AddToHistoryStorage(RecoveryStatus::RecoverySuccess, false);
    }

    return message;
}

void HistoryControl::AddHistoryItem(bool byUser)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    CreateHistoryItem(byUser ? RecoverySource::UserInitiatedRecovery : RecoverySource::AutoRecovery);
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

HistoryControl historyControl;
