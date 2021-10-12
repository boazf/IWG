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
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#endif
#include <GWConnTest.h>

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
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
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

	Transition<RecoveryMessages, RecoveryStates> periodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterPeriodicRestart },
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	Transition<RecoveryMessages, RecoveryStates> waitAfterPeriodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart }
	};

	Transition<RecoveryMessages, RecoveryStates> checkConnectivityAfterPeriodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckPeriodicRestartTimeout }
	};

	Transition<RecoveryMessages, RecoveryStates> checkPeriodicRestartTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, RecoveryStates::CheckMaxCyclesExceeded },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterPeriodicRestart }
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
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
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
			TRANSITIONS(initTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			TRANSITIONS(checkConnecticityTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitWhileConnected, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing,
			OnWaitConnectionTestPeriod, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitWhileConnectedTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::StartCheckConnectivity, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnStartCheckConnectivity, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(startCheckConnectivityTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(disconnectRouterTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitAfterRouterRecovery, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitAfterRouterRecoveryTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRouterRecoveryTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckRouterRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckRouterRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkRouterRecoveryTimeoutTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(disconnectModemTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitAfterModemRecovery, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitAfterModemRecoveryTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterModemRecoveryTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckModemRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckModemRecoveryTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkModemRecoveryTimeoutTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckMaxCyclesExceeded, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckMaxCyclesExceeded, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(checkMaxCyclesExceededTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRecoveryFailureTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitWhileRecoveryFailure, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitConnectionTestPeriod, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitWhileRecoveryFailureTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::PeriodicRestart,
			OnEnterPeriodicRestart,
			OnPeriodicRestart,
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(periodicRestartTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::WaitAfterPeriodicRestart, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(waitAfterPeriodicRestartTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckConnectivityAfterPeriodicRestart, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterPeriodicRestartTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::CheckPeriodicRestartTimeout, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnCheckPeriodicRestartTimeout, 
			DecideUponPeriodicRestartTimeout, 
			TRANSITIONS(checkPeriodicRestartTimeoutTrans)),
		SMState<RecoveryMessages, RecoveryStates>(
			RecoveryStates::HWError, 
			SMState<RecoveryMessages, RecoveryStates>::OnEnterDoNothing, 
			OnHWError, 
			SMState<RecoveryMessages, RecoveryStates>::OnExitDoNothing, 
			TRANSITIONS(hwErrorTrans))
	};

	m_param = new SMParam(this, historyControl.getLastRecovery(), RecoverySource::Auto);
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

	time_t nextPeriodicRestart = calcNextPeriodicRestart();
	if (smParam->nextPeriodicRestart != nextPeriodicRestart && nextPeriodicRestart != -1)
	{
		bool shouldWakeRecoveryThread = 
			smParam->nextPeriodicRestart == -1 ||
			smParam->nextPeriodicRestart > nextPeriodicRestart;
		smParam->nextPeriodicRestart = nextPeriodicRestart;
		if (shouldWakeRecoveryThread)
			xSemaphoreGive(smParam->waitSem);
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
	smParam->nextPeriodicRestart = calcNextPeriodicRestart();
	smParam->cycles = millis() / 1000;
}

bool RecoveryControl::isPeriodicRestartEnabled()
{
	return (AppConfig::getPeriodicallyRestartModem() || AppConfig::getPeriodicallyRestartRouter()) && AppConfig::getAutoRecovery();
}

#define SECONDS_IN_24HOURS (24 * 60 * 60)

time_t RecoveryControl::calcNextPeriodicRestart()
{
	if (!isPeriodicRestartEnabled())
		return -1;

	time_t now = t_now;
	tm tr;
	localtime_r(&now, &tr);
	tr.tm_hour = 0;
	tr.tm_min = 0;
	tr.tm_sec = 0;
	time_t lastMidnight = mktime(&tr);
	time_t nextPeriodicRestart = lastMidnight + AppConfig::getPeriodicRestartTime();
	if (nextPeriodicRestart <= now)
		nextPeriodicRestart += SECONDS_IN_24HOURS;

#ifdef DEBUG_RECOVERY_CONTROL
	char buff[128];
	localtime_r(&nextPeriodicRestart, &tr);
	strftime(buff, sizeof(buff), "Next periodic restart: %a %d/%m/%Y %T%n", &tr);
	Trace(buff);
#endif

	return nextPeriodicRestart;
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
		smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::NoRecovery, smParam->m_recoverySource);
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
			smParam->m_recoverySource = RecoverySource::Auto;
            if (!AppConfig::getAutoRecovery() && !smParam->updateConnState)
            {
                smParam->lastRecovery = INT32_MAX;
                smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, smParam->m_recoverySource);
                return message;
            }

			if (Config::singleDevice ||
				!smParam->lanConnected || 
				smParam->lastRecovery == INT32_MAX || 
				t_now - smParam->lastRecovery > Config::skipRouterTime)
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

void RecoveryControl::RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource)
{
	m_currentRecoveryState = recoveryType;
    RecoveryStateChangedParams params(recoveryType, recoverySource);
	m_recoveryStateChanged.callObservers(params);
}

RecoveryMessages RecoveryControl::OnWaitConnectionTestPeriod(void *param)
{
	SMParam *smParam = (SMParam *)param;
	RecoveryMessages requestedRecovery = RecoveryMessages::None;
	{
		Lock lock(smParam->csLock);
		xSemaphoreTake(smParam->waitSem, 0);
		if (smParam->requestedRecovery != RecoveryMessages::Done)
		{
			requestedRecovery = smParam->requestedRecovery;
			smParam->requestedRecovery = RecoveryMessages::Done;
			smParam->m_recoverySource = RecoverySource::UserInitiated;

			return requestedRecovery;
		}
	}

	time_t tWait = isPeriodicRestartEnabled() ? 
					min<time_t>(AppConfig::getConnectionTestPeriod(), smParam->nextPeriodicRestart - t_now) :
							    AppConfig::getConnectionTestPeriod();

#ifdef DEBUG_RECOVERY_CONTROL
	{
		LOCK_TRACE();
		Trace(__func__);
		Trace(": Waiting for ");
		Trace(tWait);
		Traceln(" Sec.");
	}
#endif

	bool isSemObtained = xSemaphoreTake(smParam->waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE;
	{
		Lock lock(smParam->csLock);
		requestedRecovery = smParam->requestedRecovery;
		if (isSemObtained && requestedRecovery == RecoveryMessages::Done)
		{
			// Periodic restart time had changed, we need to recalculate wait time.
			// We do it by triggering connection check. This will cause the connection test to
			// happne sonner than the usual connection test period. However, if we'd just recalculate
			// the waiting time and periodic restart is not the next scheduled operation then nect
			// connection check will happen longer than the configured connection test period. It is
			// possible to recalculate the waiting time without triggering connection check sooner
			// than needed, but this will complicate the code.
			return RecoveryMessages::CheckConnectivity;
		}
		requestedRecovery = smParam->requestedRecovery;
		smParam->requestedRecovery = RecoveryMessages::Done;
	}
	
	if (requestedRecovery == RecoveryMessages::Done && 
		isPeriodicRestartEnabled() && 
		t_now >= smParam->nextPeriodicRestart)
	{
		smParam->nextPeriodicRestart = calcNextPeriodicRestart();
		requestedRecovery = RecoveryMessages::PeriodicRestart;
		smParam->m_recoverySource = RecoverySource::Periodic;
	}
	else
	{
		smParam->m_recoverySource = 
				requestedRecovery != RecoveryMessages::Done ? RecoverySource::UserInitiated : RecoverySource::Auto;
	}

	return requestedRecovery;
}

RecoveryMessages RecoveryControl::OnStartCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, smParam->m_recoverySource);
	smParam->updateConnState = true;
	return RecoveryMessages::Done;
}

void RecoveryControl::OnEnterDisconnectRouter(void *param)
{
	OnEnterDisconnectRouter(param, true);
}

void RecoveryControl::OnEnterDisconnectRouter(void *param, bool signalStateChanged)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Router;
	smParam->lastRecovery = INT32_MAX;
	if (signalStateChanged)
		smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Router, smParam->m_recoverySource);
	delay(500);
	smParam->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
	SetRouterPowerState(PowerState::POWER_OFF);
	gwConnTest.Start(AppConfig::getRDisconnect() * 1000);
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

	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnWaitWhileRecovering(void *param)
{
	delay(5000);
	while (!gwConnTest.IsConnected())
		delay(100);
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnCheckRouterRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()) ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

void RecoveryControl::OnEnterDisconnectModem(void *param)
{
	OnEnterDisconnectModem(param, true);
}

void RecoveryControl::OnEnterDisconnectModem(void *param, bool signalStateChanged)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Modem;
	smParam->lastRecovery = INT32_MAX;
	SetModemPowerState(PowerState::POWER_OFF);
	if (signalStateChanged)
		smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Modem, smParam->m_recoverySource);
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

	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Failed, RecoverySource::Auto);
	return RecoveryMessages::Exceeded;
}

void RecoveryControl::OnEnterPeriodicRestart(void *param)
{
	((SMParam *)param)->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Periodic, RecoverySource::Periodic);

	if (AppConfig::getPeriodicallyRestartRouter())
		OnEnterDisconnectRouter(param, false);

	if (AppConfig::getPeriodicallyRestartModem())
		OnEnterDisconnectModem(param, false);
}

RecoveryMessages RecoveryControl::OnPeriodicRestart(void *param)
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectRouter(param);
	}
	
	if (GetModemPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectModem(param);
	}

	if (GetRouterPowerState() == PowerState::POWER_ON &&
		GetModemPowerState() == PowerState::POWER_ON)
		return RecoveryMessages::Done;

	return RecoveryMessages::None;
}

RecoveryMessages RecoveryControl::OnCheckPeriodicRestartTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	time_t tWait = 0;
	if (AppConfig::getPeriodicallyRestartRouter())
		tWait = static_cast<time_t>(AppConfig::getRReconnect());
	if (AppConfig::getPeriodicallyRestartModem())
		tWait = max<time_t>(tWait, AppConfig::getMReconnect());

	return t_now - smParam->recoveryStart > tWait ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

RecoveryMessages RecoveryControl::DecideUponPeriodicRestartTimeout(RecoveryMessages message, void *param)
{
	SMParam *smParam = reinterpret_cast<SMParam *>(param);
	
	if (message == RecoveryMessages::NoTimeout || // Continue waiting
		Config::singleDevice || // Incase of a single device, check max cycles
		smParam->lastRecoveryType == RecoveryTypes::Modem) // If last recovery type was modem, check max cycles
		return message;

	return RecoveryMessages::DisconnectModem;
}

void RecoveryControl::OnEnterHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::HWFailure, RecoverySource::Auto);
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

#ifdef DEBUG_STATE_MACHINE
#define X(a) {RecoveryStates::a, #a},
template<>
const std::map<RecoveryStates, std::string> StringableEnum<RecoveryStates>::strMap = 
{
    Recovery_States
};
#undef X

#define X(a) {RecoveryMessages::a, #a},
template<>
const std::map<RecoveryMessages, std::string> StringableEnum<RecoveryMessages>::strMap = 
{
    Recovery_Messages
};
#undef X
#endif

RecoveryControl recoveryControl;