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
	m_currentRecoveryState(RecoveryTypes::ConnectivityCheck),
		lanConnected(false),
		lastRecoveryType(RecoveryTypes::NoRecovery),
		requestedRecovery(RecoveryMessages::Done),
		updateConnState(false),
		cycles(0),
		stateParam(NULL),
		nextPeriodicRestart(-1)
{
	waitSem = xSemaphoreCreateBinary();
}

typedef Transition<RecoveryMessages, RecoveryStates> RecoveryTransition;

void RecoveryControl::Init()
{
	RecoveryTransition initTrans[] =
	{
		{ RecoveryMessages::Connected, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::Disconnected , RecoveryStates::CheckConnectivity }
	};

	RecoveryTransition checkConnecticityTrans[] =
	{
		{ RecoveryMessages::Disconnected, RecoveryStates::WaitWhileRecoveryFailure },
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem }
	};

	RecoveryTransition waitWhileConnectedTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
	};

	RecoveryTransition startCheckConnectivityTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivity }
	};

	RecoveryTransition disconnectRouterTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterRouterRecovery },
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	RecoveryTransition waitAfterRouterRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery }
	};

	RecoveryTransition periodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterPeriodicRestart },
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	RecoveryTransition waitAfterPeriodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart }
	};

	RecoveryTransition checkConnectivityAfterPeriodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckPeriodicRestartTimeout }
	};

	RecoveryTransition checkPeriodicRestartTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, RecoveryStates::CheckMaxCyclesExceeded },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterPeriodicRestart }
	};

	RecoveryTransition checkConnectivityAfterRouterRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckRouterRecoveryTimeout }
	};

	RecoveryTransition checkRouterRecoveryTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, RecoveryStates::CheckMaxCyclesExceeded },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterRouterRecovery }
	};

	RecoveryTransition disconnectModemTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::WaitAfterModemRecovery},
		{ RecoveryMessages::HWError, RecoveryStates::HWError }
	};

	RecoveryTransition waitAfterModemRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery }
	};

	RecoveryTransition checkConnectivityAfterModemRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected},
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckModemRecoveryTimeout }
	};

	RecoveryTransition checkModemRecoveryTimeoutTrans[] =
	{
		{ RecoveryMessages::Timeout, RecoveryStates::CheckMaxCyclesExceeded },
		{ RecoveryMessages::NoTimeout, RecoveryStates::WaitAfterModemRecovery }
	};

	RecoveryTransition checkMaxCyclesExceededTrans[] =
	{
		{ RecoveryMessages::Exceeded, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::NotExceeded, RecoveryStates::DisconnectRouter }
	};

	RecoveryTransition checkConnectivityAfterRecoveryFailureTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::WaitWhileRecoveryFailure }
	};

	RecoveryTransition waitWhileRecoveryFailureTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::StartCheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
	};

	RecoveryTransition hwErrorTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::StartCheckConnectivity }
	};

	typedef SMState<RecoveryMessages, RecoveryStates, RecoveryControl> RecoveryState;

	RecoveryState states[]
	{
		RecoveryState(
			RecoveryStates::Init, 
			OnEnterInit, 
			OnInit, 
			UpdateRecoveryState, 
			TRANSITIONS(initTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			TRANSITIONS(checkConnecticityTrans)),
		RecoveryState(
			RecoveryStates::WaitWhileConnected, 
			RecoveryState::OnEnterDoNothing,
			OnWaitConnectionTestPeriod, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(waitWhileConnectedTrans)),
		RecoveryState(
			RecoveryStates::StartCheckConnectivity, 
			RecoveryState::OnEnterDoNothing, 
			OnStartCheckConnectivity, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(startCheckConnectivityTrans)),
		RecoveryState(
			RecoveryStates::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(disconnectRouterTrans)),
		RecoveryState(
			RecoveryStates::WaitAfterRouterRecovery, 
			RecoveryState::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(waitAfterRouterRecoveryTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRouterRecoveryTrans)),
		RecoveryState(
			RecoveryStates::CheckRouterRecoveryTimeout, 
			RecoveryState::OnEnterDoNothing, 
			OnCheckRouterRecoveryTimeout, 
			OnExitCheckRouterRecoveryTimeout, 
			TRANSITIONS(checkRouterRecoveryTimeoutTrans)),
		RecoveryState(
			RecoveryStates::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(disconnectModemTrans)),
		RecoveryState(
			RecoveryStates::WaitAfterModemRecovery, 
			RecoveryState::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(waitAfterModemRecoveryTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterModemRecoveryTrans)),
		RecoveryState(
			RecoveryStates::CheckModemRecoveryTimeout, 
			RecoveryState::OnEnterDoNothing, 
			OnCheckModemRecoveryTimeout, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(checkModemRecoveryTimeoutTrans)),
		RecoveryState(
			RecoveryStates::CheckMaxCyclesExceeded, 
			RecoveryState::OnEnterDoNothing, 
			OnCheckMaxCyclesExceeded, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(checkMaxCyclesExceededTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRecoveryFailureTrans)),
		RecoveryState(
			RecoveryStates::WaitWhileRecoveryFailure, 
			RecoveryState::OnEnterDoNothing, 
			OnWaitConnectionTestPeriod, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(waitWhileRecoveryFailureTrans)),
		RecoveryState(
			RecoveryStates::PeriodicRestart,
			OnEnterPeriodicRestart,
			OnPeriodicRestart,
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(periodicRestartTrans)),
		RecoveryState(
			RecoveryStates::WaitAfterPeriodicRestart, 
			RecoveryState::OnEnterDoNothing, 
			OnWaitWhileRecovering, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(waitAfterPeriodicRestartTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterPeriodicRestart, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterPeriodicRestartTrans)),
		RecoveryState(
			RecoveryStates::CheckPeriodicRestartTimeout, 
			RecoveryState::OnEnterDoNothing, 
			OnCheckPeriodicRestartTimeout, 
			DecideUponPeriodicRestartTimeout, 
			TRANSITIONS(checkPeriodicRestartTimeoutTrans)),
		RecoveryState(
			RecoveryStates::HWError, 
			RecoveryState::OnEnterDoNothing, 
			OnHWError, 
			RecoveryState::OnExitDoNothing, 
			TRANSITIONS(hwErrorTrans))
	};

	m_recoverySource = RecoverySource::Auto,
	lastRecovery = historyControl.getLastRecovery();
	autoRecovery = AppConfig::getAutoRecovery();
	maxHistory = AppConfig::getMaxHistory();
	m_pSM = new StateMachine<RecoveryMessages, RecoveryStates, RecoveryControl>(states, NELEMS(states), this
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
	delete m_pSM;
}

void RecoveryControl::AppConfigChanged(const AppConfigChangedParam &param, const void *context)
{
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Configuration changed");
#endif
	RecoveryControl *recoveryControl = (RecoveryControl *)context;
	bool autoRecovery = AppConfig::getAutoRecovery();

	if (autoRecovery && !recoveryControl->autoRecovery)
		recoveryControl->StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);

	if (recoveryControl->autoRecovery != autoRecovery)
		recoveryControl->m_autoRecoveryStateChanged.callObservers(AutoRecoveryStateChangedParams(autoRecovery));

	recoveryControl->autoRecovery = autoRecovery;

	int maxHistory = AppConfig::getMaxHistory();
	if (maxHistory != recoveryControl->maxHistory)
	{
		recoveryControl->m_maxHistoryRecordsChanged.callObservers(MaxHistoryRecordChangedParams(maxHistory));
		recoveryControl->maxHistory = maxHistory;
	}

	time_t nextPeriodicRestart = calcNextPeriodicRestart();
	if (recoveryControl->nextPeriodicRestart != nextPeriodicRestart && nextPeriodicRestart != -1)
	{
		bool shouldWakeRecoveryThread = 
			recoveryControl->nextPeriodicRestart == -1 ||
			recoveryControl->nextPeriodicRestart > nextPeriodicRestart;
		recoveryControl->nextPeriodicRestart = nextPeriodicRestart;
		if (shouldWakeRecoveryThread)
			xSemaphoreGive(recoveryControl->waitSem);
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

void RecoveryControl::OnEnterCheckConnectivity(RecoveryControl *control)
{
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)control->stateParam;
	if (stateParam == NULL)
	{
		stateParam = new CheckConnectivityStateParam();
		control->stateParam = stateParam;
	}

	IPAddress address;

	if (stateParam->stage == CheckConnectivityStages::CheckLAN)
	{
		address = AppConfig::getLANAddr();
		if (IsZeroIPAddress(address))
		{
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			control->lanConnected = true;
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

void RecoveryControl::OnEnterInit(RecoveryControl *control)
{
	control->t0 = max<int>(AppConfig::getRReconnect(), AppConfig::getMReconnect()) * 1000 + millis();
	control->nextPeriodicRestart = calcNextPeriodicRestart();
	control->cycles = millis() / 1000;
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

RecoveryMessages RecoveryControl::OnInit(RecoveryControl *control)
{
	if (millis() > control->t0)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Timeout: could not establish connectivity upon initialization, starting recovery cycles");
#endif
		control->cycles = 0;
		return RecoveryMessages::Disconnected;
	}

	if (millis() > 1000 * control->cycles)
	{
		String server;
		if (AppConfig::getServer1().isEmpty() || AppConfig::getServer2().isEmpty())
		{
			server = AppConfig::getServer1().isEmpty() ? AppConfig::getServer2() : AppConfig::getServer1();
		}
		else
		{
			server = (control->cycles % 2 == 0) ? AppConfig::getServer1() : AppConfig::getServer2();
		}
		IPAddress address;

		if (TryGetHostAddress(address, server))
		{
			control->cycles = 0;
			return RecoveryMessages::Connected;
		}
		control->cycles++;
	}

	return RecoveryMessages::None;
}

RecoveryMessages RecoveryControl::OnCheckConnectivity(RecoveryControl *control)
{
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)control->stateParam;
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
		control->lanConnected = status == RecoveryMessages::Connected;
		if (control->lanConnected)
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
		control->stateParam = NULL;
	}

    return  status; 
}

RecoveryMessages RecoveryControl::UpdateRecoveryState(RecoveryMessages message, RecoveryControl *control)
{
	if (message == RecoveryMessages::Done)
		return message;

	if (message == RecoveryMessages::Connected)
	{
		if (control->lastRecovery == INT32_MAX)
			control->lastRecovery = t_now;
		control->RaiseRecoveryStateChanged(RecoveryTypes::NoRecovery, control->m_recoverySource);
	}
	else
	{
		control->lastRecovery = INT32_MAX;
	}
	
	return message;
}

RecoveryMessages RecoveryControl::DecideRecoveryPath(RecoveryMessages message, RecoveryControl *control)
{
	if (message != RecoveryMessages::Done)
	{
		if (message != RecoveryMessages::Connected)
		{
			control->m_recoverySource = RecoverySource::Auto;
            if (!AppConfig::getAutoRecovery() && !control->updateConnState)
            {
                control->lastRecovery = INT32_MAX;
                control->RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, control->m_recoverySource);
                return message;
            }

			if (Config::singleDevice ||
				!control->lanConnected || 
				control->lastRecovery == INT32_MAX || 
				t_now - control->lastRecovery > Config::skipRouterTime)
				message = RecoveryMessages::DisconnectRouter;
			else
				message = control->lastRecoveryType == RecoveryTypes::Router ? RecoveryMessages::DisconnectModem : RecoveryMessages::DisconnectRouter;
		}
		if (control->updateConnState)
		{
			UpdateRecoveryState(message, control);
			control->updateConnState = false;
		}
	}

	return message;
}

void RecoveryControl::RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource)
{
	m_currentRecoveryState = recoveryType;
	if (m_currentRecoveryState == RecoveryTypes::RouterSingleDevice)
		m_currentRecoveryState = RecoveryTypes::Router;
    RecoveryStateChangedParams params(recoveryType, recoverySource);
	m_recoveryStateChanged.callObservers(params);
}

RecoveryMessages RecoveryControl::OnWaitConnectionTestPeriod(RecoveryControl *control)
{
	RecoveryMessages requestedRecovery = RecoveryMessages::None;
	{
		Lock lock(control->csLock);
		xSemaphoreTake(control->waitSem, 0);
		if (control->requestedRecovery != RecoveryMessages::Done)
		{
			requestedRecovery = control->requestedRecovery;
			control->requestedRecovery = RecoveryMessages::Done;
			control->m_recoverySource = RecoverySource::UserInitiated;

			return requestedRecovery;
		}
	}

	time_t tWait = isPeriodicRestartEnabled() ? 
					min<time_t>(AppConfig::getConnectionTestPeriod(), control->nextPeriodicRestart - t_now) :
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

	bool isSemObtained = xSemaphoreTake(control->waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE;
	{
		Lock lock(control->csLock);
		requestedRecovery = control->requestedRecovery;
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
		requestedRecovery = control->requestedRecovery;
		control->requestedRecovery = RecoveryMessages::Done;
	}
	
	if (requestedRecovery == RecoveryMessages::Done && 
		isPeriodicRestartEnabled() && 
		t_now >= control->nextPeriodicRestart)
	{
		control->nextPeriodicRestart = calcNextPeriodicRestart();
		requestedRecovery = RecoveryMessages::PeriodicRestart;
		control->m_recoverySource = RecoverySource::Periodic;
	}
	else
	{
		control->m_recoverySource = 
				requestedRecovery != RecoveryMessages::Done ? RecoverySource::UserInitiated : RecoverySource::Auto;
	}

	return requestedRecovery;
}

RecoveryMessages RecoveryControl::OnStartCheckConnectivity(RecoveryControl *control)
{
	control->RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, control->m_recoverySource);
	control->updateConnState = true;
	return RecoveryMessages::Done;
}

void RecoveryControl::OnEnterDisconnectRouter(RecoveryControl *control)
{
	OnEnterDisconnectRouter(control, true);
}

void RecoveryControl::OnEnterDisconnectRouter(RecoveryControl *control, bool signalStateChanged)
{
	control->lastRecoveryType = RecoveryTypes::Router;
	control->lastRecovery = INT32_MAX;
	if (signalStateChanged)
		control->RaiseRecoveryStateChanged(
			Config::singleDevice? 
				RecoveryTypes::RouterSingleDevice : 
				RecoveryTypes::Router, 
			control->m_recoverySource);
	delay(500);
	control->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
	SetRouterPowerState(PowerState::POWER_OFF);
	gwConnTest.Start(AppConfig::getRDisconnect() * 1000);
}

RecoveryMessages RecoveryControl::OnDisconnectRouter(RecoveryControl *control)
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		if (t_now - control->recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
			return RecoveryMessages::None;

#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Reconnecting Router");
#endif
		SetRouterPowerState(PowerState::POWER_ON);
		control->t0 = t_now;
	}

	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnWaitWhileRecovering(RecoveryControl *control)
{
	delay(5000);
	while (!gwConnTest.IsConnected())
		delay(100);
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnCheckRouterRecoveryTimeout(RecoveryControl *control)
{
	return t_now - control->recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()) ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

RecoveryMessages RecoveryControl::OnExitCheckRouterRecoveryTimeout(RecoveryMessages message, RecoveryControl *control)
{
	if (message != RecoveryMessages::Timeout || Config::singleDevice)
		return message;
		
	return RecoveryMessages::DisconnectModem;
}

void RecoveryControl::OnEnterDisconnectModem(RecoveryControl *control)
{
	OnEnterDisconnectModem(control, true);
}

void RecoveryControl::OnEnterDisconnectModem(RecoveryControl *control, bool signalStateChanged)
{
	control->lastRecoveryType = RecoveryTypes::Modem;
	control->lastRecovery = INT32_MAX;
	SetModemPowerState(PowerState::POWER_OFF);
	if (signalStateChanged)
		control->RaiseRecoveryStateChanged(RecoveryTypes::Modem, control->m_recoverySource);
	control->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Modem");
#endif
	control->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_OFF));
}

RecoveryMessages RecoveryControl::OnDisconnectModem(RecoveryControl *control)
{
	if (t_now - control->recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
		return RecoveryMessages::None;

	SetModemPowerState(PowerState::POWER_ON);
	control->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnCheckModemRecoveryTimeout(RecoveryControl *control)
{
	return t_now - control->recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()) ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

RecoveryMessages RecoveryControl::OnCheckMaxCyclesExceeded(RecoveryControl *control)
{
	control->cycles++;
	if (!AppConfig::getLimitCycles() || control->cycles < AppConfig::getRecoveryCycles())
		return RecoveryMessages::NotExceeded;

	control->RaiseRecoveryStateChanged(RecoveryTypes::Failed, RecoverySource::Auto);
	control->cycles = 0;
	return RecoveryMessages::Exceeded;
}

void RecoveryControl::OnEnterPeriodicRestart(RecoveryControl *control)
{
	control->RaiseRecoveryStateChanged(RecoveryTypes::Periodic, RecoverySource::Periodic);

	if (AppConfig::getPeriodicallyRestartRouter())
		OnEnterDisconnectRouter(control, false);

	if (AppConfig::getPeriodicallyRestartModem())
		OnEnterDisconnectModem(control, false);
}

RecoveryMessages RecoveryControl::OnPeriodicRestart(RecoveryControl *control)
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectRouter(control);
	}
	
	if (GetModemPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectModem(control);
	}

	if (GetRouterPowerState() == PowerState::POWER_ON &&
		GetModemPowerState() == PowerState::POWER_ON)
		return RecoveryMessages::Done;

	return RecoveryMessages::None;
}

RecoveryMessages RecoveryControl::OnCheckPeriodicRestartTimeout(RecoveryControl *control)
{
	time_t tWait = 0;
	if (AppConfig::getPeriodicallyRestartRouter())
		tWait = static_cast<time_t>(AppConfig::getRReconnect());
	if (AppConfig::getPeriodicallyRestartModem())
		tWait = max<time_t>(tWait, AppConfig::getMReconnect());

	return t_now - control->recoveryStart > tWait ? RecoveryMessages::Timeout : RecoveryMessages::NoTimeout;
}

RecoveryMessages RecoveryControl::DecideUponPeriodicRestartTimeout(RecoveryMessages message, RecoveryControl *control)
{
	if (message == RecoveryMessages::NoTimeout || // Continue waiting
		Config::singleDevice || // Incase of a single device, check max cycles
		control->lastRecoveryType == RecoveryTypes::Modem) // If last recovery type was modem, check max cycles
		return message;

	return RecoveryMessages::DisconnectModem;
}

void RecoveryControl::OnEnterHWError(RecoveryControl *control)
{
	control->RaiseRecoveryStateChanged(RecoveryTypes::HWFailure, RecoverySource::Auto);
	control->t0 = time(NULL);
}

RecoveryMessages RecoveryControl::OnHWError(RecoveryControl *control)
{
	if (time(NULL) - control->t0 >= 10)
	    return RecoveryMessages::None;

    return RecoveryMessages::Done;
}

void RecoveryControl::StartRecoveryCycles(RecoveryTypes recoveryType)
{
	Lock lock(csLock);

	if (requestedRecovery != RecoveryMessages::Done)
		return;

	cycles = 0;
	switch(recoveryType)
	{
		case RecoveryTypes::Modem:
			requestedRecovery = RecoveryMessages::DisconnectModem;
			break;
		case RecoveryTypes::Router:
			requestedRecovery = RecoveryMessages::DisconnectRouter;
			break;
		case RecoveryTypes::ConnectivityCheck:
			requestedRecovery = RecoveryMessages::CheckConnectivity;
			break;
		default:
			break;
	}
	
	xSemaphoreGive(waitSem);
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