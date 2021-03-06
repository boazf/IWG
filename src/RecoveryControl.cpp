#include <SPI.h>
#include <RecoveryControl.h>
#include <Observers.h>
#include <Common.h>
#include <Config.h>
#ifndef USE_WIFI
#include <ICMPPing.h>
#else
#include <ping.h>
#include <EthernetUtil.h>
#endif
#include <AppConfig.h>
#include <TimeUtil.h>
#include <HistoryControl.h>

RecoveryControl::RecoveryControl() :
	m_currentRecoveryState(RecoveryTypes::ConnectivityCheck)
{
}

void RecoveryControl::Init()
{
	Transition<Message, State> initTrans[] =
	{
		{ Message::M_Connected, State::CheckConnectivity },
		{ Message::M_Disconnected , State::CheckConnectivity }
	};

	Transition<Message, State> checkConnecticityTrans[] =
	{
		{ Message::M_Disconnected, State::WaitWhileRecoveryFailure },
		{ Message::M_Done, State::CheckConnectivity },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem }
	};

	Transition<Message, State> waitWhileConnectedTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivity },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem },
		{ Message::M_CheckConnectivity, State::StartCheckConnectivity }
	};

	Transition<Message, State> startCheckConnectivityTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivity }
	};

	Transition<Message, State> disconnectRouterTrans[] =
	{
		{ Message::M_Done, State::WaitAfterRouterRecovery },
		{ Message::M_HWError, State::HWError }
	};

	Transition<Message, State> waitAfterRouterRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRouterRecovery }
	};

	Transition<Message, State> checkConnectivityAfterRouterRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRouterRecovery },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_Disconnected, State::CheckRouterRecoveryTimeout }
	};

	Transition<Message, State> checkRouterRecoveryTimeoutTrans[] =
	{
		{ Message::M_Timeout, State::DisconnectModem },
		{ Message::M_NoTimeout, State::WaitAfterRouterRecovery }
	};

	Transition<Message, State> disconnectModemTrans[] =
	{
		{ Message::M_Done, State::WaitAfterModemRecovery},
		{ Message::M_HWError, State::HWError }
	};

	Transition<Message, State> waitAfterModemRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterModemRecovery }
	};

	Transition<Message, State> checkConnectivityAfterModemRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterModemRecovery },
		{ Message::M_Connected, State::WaitWhileConnected},
		{ Message::M_Disconnected, State::CheckModemRecoveryTimeout }
	};

	Transition<Message, State> checkModemRecoveryTimeoutTrans[] =
	{
		{ Message::M_Timeout, State::CheckMaxCyclesExceeded },
		{ Message::M_NoTimeout, State::WaitAfterModemRecovery }
	};

	Transition<Message, State> checkMaxCyclesExceededTrans[] =
	{
		{ Message::M_Exceeded, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_NotExceeded, State::DisconnectRouter }
	};

	Transition<Message, State> checkConnectivityAfterRecoveryFailureTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_Disconnected, State::WaitWhileRecoveryFailure }
	};

	Transition<Message, State> waitWhileRecoveryFailureTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem },
		{ Message::M_CheckConnectivity, State::StartCheckConnectivity }
	};

	Transition<Message, State> hwErrorTrans[] =
	{
		{ Message::M_Done, State::StartCheckConnectivity }
	};

	SMState<Message, State> states[]
	{
		SMState<Message, State>(
			State::Init, 
			OnEnterInit, 
			OnInit, 
			UpdateRecoveryState, 
			TRANSITIONS(initTrans)
#ifdef DEBUG_STATE_MACHINE
			, "Init"
#endif
			),
		SMState<Message, State>(
			State::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			TRANSITIONS(checkConnecticityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivity"
#endif
			),
		SMState<Message, State>(
			State::WaitWhileConnected, 
			SMState<Message, State>::OnEnterDoNothing,
			OnWaitConnectionTestPeriod, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(waitWhileConnectedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitWhileConnected"
#endif
			),
		SMState<Message, State>(
			State::StartCheckConnectivity, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnStartCheckConnectivity, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(startCheckConnectivityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "StartCheckConnectivity"
#endif
			),
		SMState<Message, State>(
			State::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(disconnectRouterTrans)
#ifdef DEBUG_STATE_MACHINE
			, "DisconnectRouter"
#endif
			),
		SMState<Message, State>(
			State::WaitAfterRouterRecovery, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(waitAfterRouterRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitAfterRouterRecovery"
#endif
			),
		SMState<Message, State>(
			State::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRouterRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterRouterRecovery"
#endif
			),
		SMState<Message, State>(
			State::CheckRouterRecoveryTimeout, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckRouterRecoveryTimeout, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(checkRouterRecoveryTimeoutTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckRouterRecoveryTimeout"
#endif
			),
		SMState<Message, State>(
			State::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(disconnectModemTrans)
#ifdef DEBUG_STATE_MACHINE
			, "DisconnectModem"
#endif
			),
		SMState<Message, State>(
			State::WaitAfterModemRecovery, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(waitAfterModemRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitAfterModemRecovery"
#endif
			),
		SMState<Message, State>(
			State::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterModemRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterModemRecovery"
#endif
			),
		SMState<Message, State>(
			State::CheckModemRecoveryTimeout, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckModemRecoveryTimeout, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(checkModemRecoveryTimeoutTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckModemRecoveryTimeout"
#endif
			),
		SMState<Message, State>(
			State::CheckMaxCyclesExceeded, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckMaxCyclesExceeded, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(checkMaxCyclesExceededTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckMaxCyclesExceeded"
#endif
			),
		SMState<Message, State>(
			State::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRecoveryFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterRecoveryFailure"
#endif
			),
		SMState<Message, State>(
			State::WaitWhileRecoveryFailure, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnWaitConnectionTestPeriod, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(waitWhileRecoveryFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitWhileRecoveryFailure"
#endif
			),
		SMState<Message, State>(
			State::HWError, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnHWError, 
			SMState<Message, State>::OnExitDoNothing, 
			TRANSITIONS(hwErrorTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HWError"
#endif
			)
	};

	m_param = new SMParam(this, historyControl.getLastRecovery(), false);
	m_pSM = new StateMachine<Message, State>(states, NELEMS(states), m_param
#ifdef DEBUG_STATE_MACHINE
			, "Recovery"
#endif
      );

    xTaskCreatePinnedToCore(
        RecoveryControlTask,
        "RecoveryControlTask",
        1024*16,
        this,
        1,
        NULL,
        1 - xPortGetCoreID());

	AppConfig::getAppConfigChanged().addObserver(AppConfigChanged, this);
}

void RecoveryControl::RecoveryControlTask(void *param)
{
	while(true)
	{
		((RecoveryControl *)param)->PerformCycle();
	}
}

RecoveryControl::~RecoveryControl()
{
	delete m_param;
	delete m_pSM;
}

void RecoveryControl::AppConfigChanged(const AppConfigChangedParam &param, const void *context)
{
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Configuration changed");
#endif
	RecoveryControl *recoveryControl = (RecoveryControl *)context;
	SMParam *smParam = recoveryControl->m_param;
	bool autoRecovery = AppConfig::getAutoRecovery();

	if (autoRecovery && !smParam->autoRecovery)
		recoveryControl->StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);

	if (smParam->autoRecovery != autoRecovery)
		recoveryControl->m_autoRecoveryStateChanged.callObservers(AutoRecoveryStateChangedParams(autoRecovery));

	smParam->autoRecovery = autoRecovery;

	int maxHistory = AppConfig::getMaxHistory();
	if (maxHistory != smParam->maxHistory)
	{
		recoveryControl->m_maxHistoryRecordsChanged.callObservers(MaxHistoryRecordChangedParams(maxHistory));
		smParam->maxHistory = maxHistory;
	}
}

void RecoveryControl::PerformCycle()
{
	m_pSM->HandleState();
}

enum CheckConnectivityStages
{
	CheckLAN,
	CheckServer1,
	CheckServer2,
	ChecksCompleted
};

class CheckConnectivityStateParam
{
public:
	CheckConnectivityStateParam() :
#ifndef USE_WIFI
		ping(MAX_SOCK_NUM, 1),
#else
		attempts(0),
#endif
		status(Message::M_Disconnected),
		stage(CheckLAN)
	{
	}

public:
#ifndef USE_WIFI
	ICMPPing ping;
	ICMPEchoReply pingResult;
#else
	IPAddress address;
	int attempts;
#endif
	Message status;
	CheckConnectivityStages stage;
};

#define MAX_PING_ATTEMPTS 5

void RecoveryControl::OnEnterCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)smParam->stateParam;
	if (stateParam == NULL)
	{
		stateParam = new CheckConnectivityStateParam();
		smParam->stateParam = stateParam;
	}

	IPAddress address;

	if (stateParam->stage == CheckLAN)
	{
		address = AppConfig::getLANAddr();
		if (IsZeroIPAddress(address))
		{
			stateParam->stage = CheckServer1;
			smParam->lanConnected = true;
		}
	}

	String server;

	if (stateParam->stage == CheckServer1)
	{
		server = AppConfig::getServer1();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = CheckServer2;
	}

	if (stateParam->stage == CheckServer2)
	{
		server = AppConfig::getServer2();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = ChecksCompleted;
	}	

#ifdef DEBUG_RECOVERY_CONTROL
	if (stateParam->stage == CheckServer1 || stateParam->stage == CheckServer2)
	{
		LOCK_TRACE();
		Trace("About to ping ");
		Traceln(server.c_str());
	}
#endif

	if (stateParam->stage != ChecksCompleted)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		{
			LOCK_TRACE();
			Trace("Pinging address: ");
			Trace(address[0]);
			Trace(".");
			Trace(address[1]);
			Trace(".");
			Trace(address[2]);
			Trace(".");
			Traceln(address[3]);
		}
#endif
#ifndef USE_WIFI
		stateParam->ping.asyncStart(address, MAX_PING_ATTEMPTS, stateParam->pingResult);
#else
		stateParam->address = address;
		stateParam->attempts = 0;
#endif
	}
}

void RecoveryControl::OnEnterInit(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->t0 = max<int>(AppConfig::getRReconnect(), AppConfig::getMReconnect()) * 1000 + millis();
}

Message RecoveryControl::OnInit(void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (millis() > smParam->t0)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Timeout: could not establish connectivity upon initialization, starting recovery cycles");
#endif
		return M_Disconnected;
	}

	if (millis() > 1000 * smParam->cycles)
	{
		String server;
		if (AppConfig::getServer1().isEmpty() || AppConfig::getServer2().isEmpty())
		{
			server = AppConfig::getServer1().isEmpty() ? AppConfig::getServer2() : AppConfig::getServer1();
		}
		else
		{
			server = (smParam->cycles % 2 == 0) ? AppConfig::getServer1() : AppConfig::getServer2();
		}
		IPAddress address;

		if (TryGetHostAddress(address, server))
		{
			smParam->cycles = 0;
			return M_Connected;
		}
		smParam->cycles++;
	}

	return None;
}

Message RecoveryControl::OnCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)smParam->stateParam;
	Message status = Message::M_Disconnected;

	if (stateParam->stage != ChecksCompleted)
	{
#ifndef USE_WIFI
		if (!stateParam->ping.asyncComplete(stateParam->pingResult))
		 	return Message::None;

		//status = Message::M_Connected;
		status = stateParam->pingResult.status == SUCCESS ? Message::M_Connected : Message::M_Disconnected;
#else
		if (stateParam->attempts < MAX_PING_ATTEMPTS)
		{
			if (ping_start(stateParam->address, 1, 0, 0, 1000))
				status = Message::M_Connected;
			else
			{
				stateParam->attempts++;
#ifdef DEBUG_RECOVERY_CONTROL
				Tracef("Ping attempt no. %d failed, address %s\n", stateParam->attempts, stateParam->address.toString().c_str());
#endif
				return Message::None;
			}
		}
#endif
#ifdef DEBUG_RECOVERY_CONTROL
		{
			LOCK_TRACE();
			Trace("Ping result: ");
#ifndef USE_WIFI
			Traceln((unsigned int)stateParam->pingResult.status);
#else
			Traceln(status == Message::M_Connected ? "OK" : "Failed");
#endif
		}
#endif
	}

	stateParam->status = status;

	switch (stateParam->stage)
	{
	case CheckLAN:
		smParam->lanConnected = status == Message::M_Connected;
		if (smParam->lanConnected)
		{
			stateParam->stage = CheckServer1;
			status = Message::M_Done;
		}
		else
		{
			stateParam->stage = ChecksCompleted;
		}
		break;
	case CheckServer1:
		if (status == Message::M_Connected)
		{
			stateParam->stage = ChecksCompleted;
		}
		else
		{
			stateParam->stage = CheckServer2;
			status = Message::M_Done;
		}
		break;
	case CheckServer2:
		stateParam->stage = ChecksCompleted;
		break;
	case ChecksCompleted:
		break;
	}

	// Do not move this code to the switch
	if (stateParam->stage == ChecksCompleted)
	{
		delete stateParam;
		smParam->stateParam = NULL;
	}

    return  status; 
}

Message RecoveryControl::UpdateRecoveryState(Message message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message == Message::M_Done)
		return message;

	if (message == Message::M_Connected)
	{
		if (smParam->lastRecovery == INT32_MAX)
			smParam->lastRecovery = t_now;
		smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::NoRecovery, smParam->m_byUser);
	}
	else
	{
		smParam->lastRecovery = INT32_MAX;
	}
	
	return message;
}

Message RecoveryControl::DecideRecoveryPath(Message message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message != Message::M_Done)
	{
		if (message != Message::M_Connected)
		{
			smParam->m_byUser = false;
            if (!AppConfig::getAutoRecovery() && !smParam->updateConnState)
            {
                smParam->lastRecovery = INT32_MAX;
                smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, param);
                return message;
            }

			if (!smParam->lanConnected || smParam->lastRecovery == INT32_MAX || t_now - smParam->lastRecovery > 3600)
				message = Message::M_DisconnectRouter;
			else
				message = smParam->lastRecoveryType == RecoveryTypes::Router ? Message::M_DisconnectModem : Message::M_DisconnectRouter;
		}
		if (smParam->updateConnState)
		{
			UpdateRecoveryState(message, param);
			smParam->updateConnState = false;
		}
	}

	return message;
}

void RecoveryControl::RaiseRecoveryStateChanged(RecoveryTypes recoveryType, bool byUser)
{
	m_currentRecoveryState = recoveryType;
    RecoveryStateChangedParams params(recoveryType, byUser);
	m_recoveryStateChanged.callObservers(params);
}

Message RecoveryControl::OnWaitConnectionTestPeriod(void *param)
{
#ifdef DEBUG_RECOVERY_CONTROL
	{
		LOCK_TRACE();
		Trace(__func__);
		Trace(": Waiting for ");
		Trace(AppConfig::getConnectionTestPeriod());
		Traceln(" Sec.");
	}
#endif
	SMParam *smParam = (SMParam *)param;
	Message requestedRecovery = Message::None;
	{
		Lock lock(smParam->csLock);
		xSemaphoreTake(smParam->waitSem, 0);
		if (smParam->requestedRecovery != M_Done)
		{
			requestedRecovery = smParam->requestedRecovery;
			smParam->requestedRecovery = M_Done;
			smParam->m_byUser = true;

			return requestedRecovery;
		}
	}

	xSemaphoreTake(smParam->waitSem, (AppConfig::getConnectionTestPeriod() * 1000) / portTICK_PERIOD_MS);
	{
		Lock lock(smParam->csLock);
		requestedRecovery = smParam->requestedRecovery;
		smParam->requestedRecovery = Message::M_Done;
	}
	
	smParam->m_byUser = requestedRecovery != M_Done;

	return requestedRecovery;
}

Message RecoveryControl::OnStartCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, smParam->m_byUser);
	smParam->updateConnState = true;
	return Message::M_Done;
}

void RecoveryControl::OnEnterDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Router;
	smParam->lastRecovery = INT32_MAX;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Router, smParam->m_byUser);
	delay(500);
	smParam->recoveryStart = t_now;
	SetRouterPowerState(POWER_OFF);
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
}

Message RecoveryControl::OnDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
		return Message::None;

	SetRouterPowerState(POWER_ON);
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Router");
#endif
	return Message::M_Done;
}

Message RecoveryControl::OnWaitWhileRecovering(void *param)
{
	delay(5000);
	return Message::M_Done;
}

Message RecoveryControl::OnCheckRouterRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()) ? Message::M_Timeout : Message::M_NoTimeout;
}

void RecoveryControl::OnEnterDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Modem;
	smParam->lastRecovery = INT32_MAX;
	SetModemPowerState(POWER_OFF);
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Modem, smParam->m_byUser);
	smParam->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Modem");
#endif
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(POWER_OFF));
}

Message RecoveryControl::OnDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
		return Message::None;

	SetModemPowerState(POWER_ON);
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return Message::M_Done;
}

Message RecoveryControl::OnCheckModemRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()) ? Message::M_Timeout : Message::M_NoTimeout;
}

Message RecoveryControl::OnCheckMaxCyclesExceeded(void *param)
{
	SMParam *smParam = (SMParam *)param;

	smParam->cycles++;
	if (!AppConfig::getLimitCycles() || smParam->cycles < AppConfig::getRecoveryCycles())
		return Message::M_NotExceeded;

	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Failed, false);
	return Message::M_Exceeded;
}

void RecoveryControl::OnEnterHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::HWFailure, false);
	smParam->t0 = time(NULL);
}

Message RecoveryControl::OnHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (time(NULL) - smParam->t0 >= 10)
	    return Message::None;

    return Message::M_Done;
}

void RecoveryControl::StartRecoveryCycles(RecoveryTypes recoveryType)
{
	Lock lock(m_param->csLock);

	if (m_param->requestedRecovery != Message::M_Done)
		return;

	m_param->cycles = 0;
	switch(recoveryType)
	{
		case RecoveryTypes::Modem:
			m_param->requestedRecovery = Message::M_DisconnectModem;
			break;
		case RecoveryTypes::Router:
			m_param->requestedRecovery = Message::M_DisconnectRouter;
			break;
		case RecoveryTypes::ConnectivityCheck:
			m_param->requestedRecovery = Message::M_CheckConnectivity;
			break;
		default:
			break;
	}
	
	xSemaphoreGive(m_param->waitSem);
}

RecoveryControl recoveryControl;