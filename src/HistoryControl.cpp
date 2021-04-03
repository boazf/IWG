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
        { HM_RecoverySuccess, HS_Connected }
    };

    Transition<H_Message, H_State> connectedTrans[] =
    {
        { HM_RecoverySuccess, HS_Connected },
        { HM_CheckConnectivity, HS_CheckingConnectivity },
        { HM_ModemRecovery, HS_RecoveringModem },
        { HM_RouterRecovery, HS_RecoveringRouter },
        { HM_HWFailure, HS_HWFailure }
    };

    Transition<H_Message, H_State> checkingConnectivityTrans[] =
    {
        { HM_RecoverySuccess, HS_Connected },
        { HM_ModemRecovery, HS_RecoveringModem },
        { HM_RouterRecovery, HS_RecoveringRouter}
    };

    Transition<H_Message, H_State> recoveringModemTrans[] =
    {
        { HM_RouterRecovery, HS_RecoveringRouter },
        { HM_RecoveryFailure, HS_RecoveryFailed },
        { HM_RecoverySuccess, HS_Connected },
        { HM_HWFailure, HS_HWFailure},
    };

    Transition<H_Message, H_State> recoveringRouterTrans[] =
    { 
        { HM_ModemRecovery, HS_RecoveringModem },
        { HM_RecoveryFailure, HS_RecoveryFailed },
        { HM_RecoverySuccess, HS_Connected },
        { HM_HWFailure, HS_HWFailure}
    };

    Transition<H_Message, H_State> recoveryFailedTrans[] =
    {
        { HM_ModemRecovery, HS_RecoveringModem },
        { HM_RouterRecovery, HS_RecoveringRouter },
        { HM_RecoverySuccess, HS_Connected },
        { HM_CheckConnectivity, HS_CheckingConnectivityWhileInFailure}
    };

    Transition<H_Message, H_State> checkingConnectivityWhileInFailureTrans[] =
    {
        { HM_ModemRecovery, HS_RecoveringModem },
        { HM_RouterRecovery, HS_RecoveringRouter },
        { HM_RecoverySuccess, HS_Connected}
    };

    Transition<H_Message, H_State> HWFailureTrans[] =
    { 
        { HM_CheckConnectivity, HS_CheckingConnectivity }
    };

    SMState<H_Message, H_State> states[] =
    {
        SMState<H_Message, H_State>(
            HS_Init, 
            SMState<H_Message, H_State>::OnEnterDoNothing, 
            OnInit,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(initTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_Init"
#endif
			),
        SMState<H_Message, H_State>(
            HS_Connected, 
            OnConnected, 
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(connectedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_Connected"
#endif
			),
        SMState<H_Message, H_State>(
            HS_CheckingConnectivity, 
            OnCheckingConnectivity,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(checkingConnectivityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_CheckingConnectivity"
#endif
			),
        SMState<H_Message, H_State>(
            HS_RecoveringModem, 
            OnRecoveringModem,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(recoveringModemTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_RecoveringModem"
#endif
			),
        SMState<H_Message, H_State>(
            HS_RecoveringRouter, 
            OnRecoveringRouter,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(recoveringRouterTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_RecoveringRouter"
#endif
			),
        SMState<H_Message, H_State>(
            HS_RecoveryFailed, 
            OnRecoveryFailed,
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(recoveryFailedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_RecoveryFailed"
#endif
			),
        SMState<H_Message, H_State>(
            HS_CheckingConnectivityWhileInFailure, 
            OnCheckingConnectivity, 
            OnStateDoNotihng,
            AddToHistory,
            TRANSITIONS(checkingConnectivityWhileInFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_CheckingConnectivityWhileInFailure"
#endif
			),
        SMState<H_Message, H_State>(
            HS_HWFailure, 
            OnHWFailure,
            OnStateDoNotihng,
   			SMState<H_Message, H_State>::OnExitDoNothing,
            TRANSITIONS(HWFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HS_HWFailure"
#endif
			)
    };
  	m_pSM = new StateMachine<H_Message, H_State>(states, NELEMS(states), this
#ifdef DEBUG_STATE_MACHINE
			, "History"
#endif
      );
    storage.init(maxHistory);
}

HistoryControl::~HistoryControl()
{
    delete m_pSM;
}

time_t HistoryControl::getLastRecovery()
{ 
    return storage.getLastRecovery(); 
}

void HistoryControl::PerformCycle()
{
    m_pSM->HandleState();
}

void HistoryControl::RecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context)
{
    HistoryControl *historyControl = (HistoryControl *)context;

    if (historyControl->m_pSM->current()->State() == HS_Init)
        return;

    historyControl->byUser = params.m_byUser;

    switch(params.m_recoveryType)
    {
    case ConnectivityCheck:
        historyControl->m_pSM->ApplyVerb(HM_CheckConnectivity);
        break;
    case Failed:
        historyControl->m_pSM->ApplyVerb(HM_RecoveryFailure);
        break;
    case Modem:
        historyControl->m_pSM->ApplyVerb(HM_ModemRecovery);
        break;
    case Router:
        historyControl->m_pSM->ApplyVerb(HM_RouterRecovery);
        break;
    case HWFailure:
        historyControl->m_pSM->ApplyVerb(HM_HWFailure);
        break;
    case NoRecovery:
        historyControl->m_pSM->ApplyVerb(HM_RecoverySuccess);
        break;
    case Disconnected:
        break;
    }
}

H_Message HistoryControl::OnInit(void *param)
{
    if (recoveryControl.GetRecoveryState() == NoRecovery)
        return HM_RecoverySuccess;
    
    return HM_None;
}

void HistoryControl::MaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context)
{
    HistoryControl *historyControl = (HistoryControl *)context;
    historyControl->maxHistory = params.m_maxRecords;
    historyControl->storage.resize(historyControl->maxHistory);
}

H_Message HistoryControl::OnStateDoNotihng(void *param)
{
    return HM_None;
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
            control->AddToHistoryStorage(RecoverySuccess);
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

    control->CreateHistoryItem(control->byUser ? UserInitiatedRecovery : AutoRecovery);
}

void HistoryControl::OnRecoveringModem(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddHistoryItem(control->byUser);
    control->currStorageItem->modemRecoveries()++;
}

void HistoryControl::OnRecoveringRouter(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddHistoryItem(control->byUser);
    control->currStorageItem->routerRecoveries()++;
}

void HistoryControl::OnRecoveryFailed(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    control->AddToHistoryStorage(RecoveryFailure);
}

void HistoryControl::OnHWFailure(void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    HistoryControl *control = (HistoryControl *)param;

    delete control->currStorageItem;
    control->currStorageItem = NULL;
}

H_Message HistoryControl::AddToHistory(H_Message message, void *param)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    if (message == HM_RecoverySuccess)
    {
        HistoryControl *control = (HistoryControl *)param;
        control->AddHistoryItem(control->byUser);
        control->AddToHistoryStorage(RecoverySuccess, false);
    }

    return message;
}

void HistoryControl::AddHistoryItem(bool byUser)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    CreateHistoryItem(byUser ? UserInitiatedRecovery : AutoRecovery);
}

bool HistoryControl::CreateHistoryItem(RecoverySource recoverySource)
{
#ifdef DEBUG_HISTORY
    Traceln(__func__);
#endif
    if (currStorageItem != NULL)
        return false;

    currStorageItem = new HistoryStorageItem(recoverySource, t_now, INT32_MAX, 0, 0, OnGoingRecovery);
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
