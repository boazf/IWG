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
	Transition<RecoveryMessages, RecoveryStates> initTrans[] =
	{
		{ RecoveryMessages::Connected, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::Disconnected , RecoveryStates::CheckConnectivity }
	};

	Transition<RecoveryMessages, RecoveryStates> checkConnecticityTrans[] =
	{
		{ RecoveryMessages::Disconnected, RecoveryStates::WaitWhileRecoveryFailure },
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem }
	};

	Transition<RecoveryMessages, RecoveryStates> waitWhileConnectedTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity }
	};

	Transition<RecoveryMessages, RecoveryStates> startCheckConnectivityTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity }
	};

	Transition<RecoveryMessages, RecoveryStates> disconnectRouterTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterRouterRecovery },
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	Transition<RecoveryMessages, RecoveryStates> waitAfterRouterRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery }
	};

	Transition<RecoveryMessages, RecoveryStates> checkConnectivityAfterRouterRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckRouterRecoveryTimeout }
	};

	Transition<RecoveryMessages, RecoveryStates> checkRouterRecoveryTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, Config::singleDevice ? RecoveryStates::CheckMaxCyclesExceeded : RecoveryStates::DisconnectModem },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterRouterRecovery }
	};

	Transition<RecoveryMessages, RecoveryStates> disconnectModemTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterModemRecovery},
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	Transition<RecoveryMessages, RecoveryStates> waitAfterModemRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery }
	};

	Transition<RecoveryMessages, RecoveryStates> checkConnectivityAfterModemRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected},
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckModemRecoveryTimeout }
	};

	Transition<RecoveryMessages, RecoveryStates> checkModemRecoveryTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, RecoveryStates::CheckMaxCyclesExceeded },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterModemRecovery }
	};

	Transition<RecoveryMessages, RecoveryStates> checkMaxCyclesExceededTrans[] =
	{
		{ RecoveryMessages::Exceeded, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::NotExceeded, RecoveryStates::DisconnectRouter }
	};

	Transition<RecoveryMessages, RecoveryStates> checkConnectivityAfterRecoveryFailureTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::WaitWhileRecoveryFailure }
	};

	Transition<RecoveryMessages, RecoveryStates> waitWhileRecoveryFailureTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity }
	};

	Transition<RecoveryMessages, RecoveryStates> hwErrorTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::StartCheckConnectivity }
	};

	SMState<RecoveryMessages, RecoveryStates> states[]
	{
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::Init, 
			OnEnterInit, 
			OnInit, 
			UpdateRecoveryState, 
			TRANSITIONS(initTrans)
#ifdef DEBUG_STATE_MACHINE
			, "Init"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			TRANSITIONS(checkConnecticityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivity"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitWhileConnected, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing,
			OnWaitConnectionTestPeriod, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitWhileConnectedTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitWhileConnected"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::StartCheckConnectivity, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnStartCheckConnectivity, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(startCheckConnectivityTrans)
#ifdef DEBUG_STATE_MACHINE
			, "StartCheckConnectivity"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(disconnectRouterTrans)
#ifdef DEBUG_STATE_MACHINE
			, "DisconnectRouter"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitAfterRouterRecovery, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitAfterRouterRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitAfterRouterRecovery"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRouterRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterRouterRecovery"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckRouterRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckRouterRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkRouterRecoveryTimeoutTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckRouterRecoveryTimeout"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(disconnectModemTrans)
#ifdef DEBUG_STATE_MACHINE
			, "DisconnectModem"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitAfterModemRecovery, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitAfterModemRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitAfterModemRecovery"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterModemRecoveryTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterModemRecovery"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckModemRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckModemRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkModemRecoveryTimeoutTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckModemRecoveryTimeout"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckMaxCyclesExceeded, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckMaxCyclesExceeded, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkMaxCyclesExceededTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckMaxCyclesExceeded"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRecoveryFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "CheckConnectivityAfterRecoveryFailure"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitWhileRecoveryFailure, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitConnectionTestPeriod, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitWhileRecoveryFailureTrans)
#ifdef DEBUG_STATE_MACHINE
			, "WaitWhileRecoveryFailure"
#endif
			),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::HWError, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnHWError, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(hwErrorTrans)
#ifdef DEBUG_STATE_MACHINE
			, "HWError"
#endif
			)
	};

	m_param = new SMParam(this, historyControl.getLastRecovery(), false);
	m_pSM = new StateMachine<RecoveryMessages, RecoveryStates>(states, NELEMS(states), m_param
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
		delay(1);
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

enum class CheckConnectivityStages
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
		status(RecoveryMessages::Disconnected),
		stage(CheckConnectivityStages::CheckLAN)
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
	RecoveryMessages status;
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

	if (stateParam->stage == CheckConnectivityStages::CheckLAN)
	{
		address = AppConfig::getLANAddr();
		if (IsZeroIPAddress(address))
		{
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			smParam->lanConnected = true;
		}
	}

	String server;

	if (stateParam->stage == CheckConnectivityStages::CheckServer1)
	{
		server = AppConfig::getServer1();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = CheckConnectivityStages::CheckServer2;
	}

	if (stateParam->stage == CheckConnectivityStages::CheckServer2)
	{
		server = AppConfig::getServer2();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = CheckConnectivityStages::ChecksCompleted;
	}	

#ifdef DEBUG_RECOVERY_CONTROL
	if (stateParam->stage == CheckConnectivityStages::CheckServer1 ||
	    stateParam->stage == CheckConnectivityStages::CheckServer2)
	{
		LOCK_TRACE();
		Trace("About to ping ");
		Traceln(server.c_str());
	}
#endif

	if (stateParam->stage != CheckConnectivityStages::ChecksCompleted)
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

RecoveryMessages RecoveryControl::OnInit(void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (millis() > smParam->t0)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Timeout: could not establish connectivity upon initialization, starting recovery cycles");
#endif
		return RecoveryMessages::Disconnected;
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
			return RecoveryMessages::Connected;
		}
		smParam->cycles++;
	}

	return RecoveryMessages::None;
}

RecoveryMessages RecoveryControl::OnCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)smParam->stateParam;
	RecoveryMessages status = RecoveryMessages::Disconnected;

	if (stateParam->stage != CheckConnectivityStages::ChecksCompleted)
	{
#ifndef USE_WIFI
		if (!stateParam->ping.asyncComplete(stateParam->pingResult))
		 	return RecoveryMessages::None;

		//status = RecoveryMessages::Connected;
		status = stateParam->pingResult.status == SUCCESS ? RecoveryMessages::Connected : RecoveryMessages::Disconnected;
#else
		if (stateParam->attempts < MAX_PING_ATTEMPTS)
		{
			if (ping_start(stateParam->address, 1, 0, 0, 1000))
				status = RecoveryMessages::Connected;
			else
			{
				stateParam->attempts++;
#ifdef DEBUG_RECOVERY_CONTROL
				Tracef("Ping attempt no. %d failed, address %s\n", stateParam->attempts, stateParam->address.toString().c_str());
#endif
				return RecoveryMessages::None;
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
			Traceln(status == RecoveryMessages::Connected ? "OK" : "Failed");
#endif
		}
#endif
	}

	stateParam->status = status;

	switch (stateParam->stage)
	{
	case CheckConnectivityStages::CheckLAN:
		smParam->lanConnected = status == RecoveryMessages::Connected;
		if (smParam->lanConnected)
		{
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			status = RecoveryMessages::Done;
		}
		else
		{
			stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		}
		break;
	case CheckConnectivityStages::CheckServer1:
		if (status == RecoveryMessages::Connected)
		{
			stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		}
		else
		{
			stateParam->stage = CheckConnectivityStages::CheckServer2;
			status = RecoveryMessages::Done;
		}
		break;
	case CheckConnectivityStages::CheckServer2:
		stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		break;
	case CheckConnectivityStages::ChecksCompleted:
		break;
	}

	// Do not move this code to the switch
	if (stateParam->stage == CheckConnectivityStages::ChecksCompleted)
	{
		delete stateParam;
		smParam->stateParam = NULL;
	}

    return  status; 
}

RecoveryMessages RecoveryControl::UpdateRecoveryState(RecoveryMessages message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message == RecoveryMessages::Done)
		return message;

	if (message == RecoveryMessages::Connected)
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

RecoveryMessages RecoveryControl::DecideRecoveryPath(RecoveryMessages message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message != RecoveryMessages::Done)
	{
		if (message != RecoveryMessages::Connected)
		{
			smParam->m_byUser = false;
            if (!AppConfig::getAutoRecovery() && !smParam->updateConnState)
            {
                smParam->lastRecovery = INT32_MAX;
                smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, param);
                return message;
            }

			if (Config::singleDevice ||
				!smParam->lanConnected || 
				smParam->lastRecovery == INT32_MAX || 
				t_now - smParam->lastRecovery > 3600)
				message = RecoveryMessages::DisconnectRouter;
			else
				message = smParam->lastRecoveryType == RecoveryTypes::Router ? RecoveryMessages::DisconnectModem : RecoveryMessages::DisconnectRouter;
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

RecoveryMessages RecoveryControl::OnWaitConnectionTestPeriod(void *param)
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
	RecoveryMessages requestedRecovery = RecoveryMessages::None;
	{
		Lock lock(smParam->csLock);
		xSemaphoreTake(smParam->waitSem, 0);
		if (smParam->requestedRecovery != RecoveryMessages::Done)
		{
			requestedRecovery = smParam->requestedRecovery;
			smParam->requestedRecovery = RecoveryMessages::Done;
			smParam->m_byUser = true;

			return requestedRecovery;
		}
	}

	xSemaphoreTake(smParam->waitSem, (AppConfig::getConnectionTestPeriod() * 1000) / portTICK_PERIOD_MS);
	{
		Lock lock(smParam->csLock);
		requestedRecovery = smParam->requestedRecovery;
		smParam->requestedRecovery = RecoveryMessages::Done;
	}
	
	smParam->m_byUser = requestedRecovery != RecoveryMessages::Done;

	return requestedRecovery;
}

RecoveryMessages RecoveryControl::OnStartCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, smParam->m_byUser);
	smParam->updateConnState = true;
	return RecoveryMessages::Done;
}

void RecoveryControl::OnEnterDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Router;
	smParam->lastRecovery = INT32_MAX;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Router, smParam->m_byUser);
	delay(500);
	smParam->recoveryStart = t_now;
	SetRouterPowerState(PowerState::POWER_OFF);
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
}

RecoveryMessages RecoveryControl::OnDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
			return RecoveryMessages::None;

#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Reconnecting Router");
#endif
		SetRouterPowerState(PowerState::POWER_ON);
		smParam->t0 = t_now;
	}

#ifndef USE_WIFI
	if (t_now - smParam->t0 < Config::routerInitTimeSec)
		return RecoveryMessages::None;

	InitEthernet();
#endif

	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnWaitWhileRecovering(void *param)
{
	delay(5000);
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnCheckRouterRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()) ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

void RecoveryControl::OnEnterDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Modem;
	smParam->lastRecovery = INT32_MAX;
	SetModemPowerState(PowerState::POWER_OFF);
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Modem, smParam->m_byUser);
	smParam->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Modem");
#endif
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_OFF));
}

RecoveryMessages RecoveryControl::OnDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
		return RecoveryMessages::None;

	SetModemPowerState(PowerState::POWER_ON);
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnCheckModemRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()) ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

RecoveryMessages RecoveryControl::OnCheckMaxCyclesExceeded(void *param)
{
	SMParam *smParam = (SMParam *)param;

	smParam->cycles++;
	if (!AppConfig::getLimitCycles() || smParam->cycles < AppConfig::getRecoveryCycles())
		return RecoveryMessages::NotExceeded;

	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Failed, false);
	return RecoveryMessages::Exceeded;
}

void RecoveryControl::OnEnterHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::HWFailure, false);
	smParam->t0 = time(NULL);
}

RecoveryMessages RecoveryControl::OnHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (time(NULL) - smParam->t0 >= 10)
	    return RecoveryMessages::None;

    return RecoveryMessages::Done;
}

void RecoveryControl::StartRecoveryCycles(RecoveryTypes recoveryType)
{
	Lock lock(m_param->csLock);

	if (m_param->requestedRecovery != RecoveryMessages::Done)
		return;

	m_param->cycles = 0;
	switch(recoveryType)
	{
		case RecoveryTypes::Modem:
			m_param->requestedRecovery = RecoveryMessages::DisconnectModem;
			break;
		case RecoveryTypes::Router:
			m_param->requestedRecovery = RecoveryMessages::DisconnectRouter;
			break;
		case RecoveryTypes::ConnectivityCheck:
			m_param->requestedRecovery = RecoveryMessages::CheckConnectivity;
			break;
		default:
			break;
	}
	
	xSemaphoreGive(m_param->waitSem);
}

RecoveryControl recoveryControl;