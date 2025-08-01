/*
 * Copyright 2020-2025 Boaz Feldboim
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// SPDX-License-Identifier: Apache-2.0

#include <RecoveryControl.h>
#include <Observers.h>
#include <Common.h>
#include <Config.h>
#include <EthernetUtil.h>
#ifndef USE_WIFI
#include <ICMPPingEx.h>
#else
#include <ping.h>
#endif
#include <AppConfig.h>
#include <TimeUtil.h>
#include <HistoryControl.h>
#ifdef DEBUG_STATE_MACHINE
#include <StringableEnum.h>
#include <Trace.h>
#endif
#include <GWConnTest.h>

using namespace historycontrol;

RecoveryControl::RecoveryControl() :
	m_currentRecoveryState(RecoveryTypes::ConnectivityCheck),
		lanConnected(false),
		lastRecoveryType(RecoveryTypes::NoRecovery),
		requestedRecovery(RecoveryMessages::Done),
		updateConnState(false),
		cycles(0),
		stateParam(NULL),
		nextPeriodicRestart(-1),
		m_pSM(NULL)
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

	RecoveryTransition checkConnectivityTrans[] =
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
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
	};

	RecoveryTransition disconnectRouterTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery }
	};

	RecoveryTransition periodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart }
	};

	RecoveryTransition checkConnectivityAfterPeriodicRestartTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterPeriodicRestart },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
	};

	RecoveryTransition checkConnectivityAfterRouterRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterRouterRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected },
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::DisconnectModem, RecoveryStates::DisconnectModem },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
	};

	RecoveryTransition disconnectModemTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery }
	};

	RecoveryTransition checkConnectivityAfterModemRecoveryTrans[] =
	{
		{ RecoveryMessages::Done, RecoveryStates::CheckConnectivityAfterModemRecovery },
		{ RecoveryMessages::Connected, RecoveryStates::WaitWhileConnected},
		{ RecoveryMessages::Disconnected, RecoveryStates::CheckConnectivityAfterRecoveryFailure },
		{ RecoveryMessages::DisconnectRouter, RecoveryStates::DisconnectRouter },
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
		{ RecoveryMessages::CheckConnectivity, RecoveryStates::CheckConnectivity },
		{ RecoveryMessages::PeriodicRestart, RecoveryStates::PeriodicRestart }
	};

	typedef SMState<RecoveryMessages, RecoveryStates, RecoveryControl> RecoveryState;

	RecoveryState states[]
	{
		RecoveryState(
			RecoveryStates::Init, 
			OnEnterInit, 
			OnInit, 
			OnExitInit, 
			TRANSITIONS(initTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			TRANSITIONS(checkConnectivityTrans)),
		RecoveryState(
			RecoveryStates::WaitWhileConnected, 
			NULL,
			OnWaitConnectionTestPeriod, 
			OnExitWaitConnectionTestPeriod, 
			TRANSITIONS(waitWhileConnectedTrans)),
		RecoveryState(
			RecoveryStates::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			OnWaitWhileRecovering, 
			TRANSITIONS(disconnectRouterTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			OnExitCheckConnectivityAfterRouterRecovery, 
			TRANSITIONS(checkConnectivityAfterRouterRecoveryTrans)),
		RecoveryState(
			RecoveryStates::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			OnWaitWhileRecovering, 
			TRANSITIONS(disconnectModemTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			OnExitCheckConnectivityAfterModemRecovery, 
			TRANSITIONS(checkConnectivityAfterModemRecoveryTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			TRANSITIONS(checkConnectivityAfterRecoveryFailureTrans)),
		RecoveryState(
			RecoveryStates::WaitWhileRecoveryFailure, 
			NULL, 
			OnWaitConnectionTestPeriod, 
			OnExitWaitConnectionTestPeriod,
			TRANSITIONS(waitWhileRecoveryFailureTrans)),
		RecoveryState(
			RecoveryStates::PeriodicRestart,
			OnEnterPeriodicRestart,
			OnPeriodicRestart,
			OnWaitWhileRecovering, 
			TRANSITIONS(periodicRestartTrans)),
		RecoveryState(
			RecoveryStates::CheckConnectivityAfterPeriodicRestart, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			OnExitCheckConnectivityAfterPeriodicRestart, 
			TRANSITIONS(checkConnectivityAfterPeriodicRestartTrans)),
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
        1024*8,
        this,
        1,
        NULL,
        1 - xPortGetCoreID());

	AppConfig::getAppConfigChanged().addObserver(OnAppConfigChanged, this);
}

void RecoveryControl::Start()
{
	m_pSM->Start();
}

void RecoveryControl::RecoveryControlTask(void *param)
{
	RecoveryControl *recoveryControl = static_cast<RecoveryControl *>(param);

	recoveryControl->Start();
	while(true)
	{
		recoveryControl->PerformCycle();
		delay(1);
	}
}

RecoveryControl::~RecoveryControl()
{
	delete m_pSM;
}

void RecoveryControl::OnAppConfigChanged(const AppConfigChangedParam &param)
{
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Configuration changed");
#endif
	bool autoRecovery = AppConfig::getAutoRecovery();

	if (autoRecovery && !this->autoRecovery)
		StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);

	if (this->autoRecovery != autoRecovery)
		m_autoRecoveryStateChanged.callObservers(AutoRecoveryStateChangedParams(autoRecovery));

	this->autoRecovery = autoRecovery;

	int maxHistory = AppConfig::getMaxHistory();
	if (maxHistory != this->maxHistory)
	{
		m_maxHistoryRecordsChanged.callObservers(MaxHistoryRecordChangedParams(maxHistory));
		this->maxHistory = maxHistory;
	}

	time_t nextPeriodicRestart = calcNextPeriodicRestart();
	bool shouldWakeRecoveryThread = 
		this->nextPeriodicRestart != nextPeriodicRestart && 
		nextPeriodicRestart != -1 &&
		(this->nextPeriodicRestart == -1 ||
		this->nextPeriodicRestart > nextPeriodicRestart);
	this->nextPeriodicRestart = nextPeriodicRestart;
	if (shouldWakeRecoveryThread)
		xSemaphoreGive(waitSem);
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
	ICMPPingEx ping;
	ICMPEchoReply pingResult;
#else
	IPAddress address;
	int attempts;
#endif
	RecoveryMessages status;
	CheckConnectivityStages stage;
};

#define MAX_PING_ATTEMPTS 5

void RecoveryControl::OnEnterCheckConnectivity()
{
	CheckConnectivityStateParam *stateParam = static_cast<CheckConnectivityStateParam *>(this->stateParam);
	if (stateParam == NULL)
	{
		stateParam = new CheckConnectivityStateParam();
		this->stateParam = stateParam;
	}

	IPAddress address;

	if (stateParam->stage == CheckConnectivityStages::CheckLAN)
	{
		address = AppConfig::getLANAddr();
		if (IsZeroIPAddress(address))
		{
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			lanConnected = true;
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
		LOCK_TRACE;
		Trace("About to ping ");
		Traceln(server.c_str());
	}
#endif

	if (stateParam->stage == CheckConnectivityStages::ChecksCompleted)
		return;

#ifdef DEBUG_RECOVERY_CONTROL
	TRACE_BLOCK
	{
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

void RecoveryControl::OnEnterInit()
{
	t0 = max<int>(AppConfig::getRReconnect(), AppConfig::getMReconnect()) * 1000 + millis();
	nextPeriodicRestart = calcNextPeriodicRestart();
	cycles = millis() / 1000;
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

RecoveryMessages RecoveryControl::OnInit()
{
	if (millis() > t0)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Timeout: could not establish connectivity upon initialization, starting recovery cycles");
#endif
		return RecoveryMessages::Disconnected;
	}

	if (millis() > 1000 * cycles)
	{
		String server;
		if (AppConfig::getServer1().isEmpty() || AppConfig::getServer2().isEmpty())
		{
			server = AppConfig::getServer1().isEmpty() ? AppConfig::getServer2() : AppConfig::getServer1();
		}
		else
		{
			server = (cycles % 2 == 0) ? AppConfig::getServer1() : AppConfig::getServer2();
		}
		IPAddress address;

		if (TryGetHostAddress(address, server))
		{
			return RecoveryMessages::Connected;
		}
		cycles++;
	}

	return RecoveryMessages::None;
}

RecoveryMessages RecoveryControl::OnExitInit(RecoveryMessages message)
{
	cycles = 0;
	return UpdateRecoveryState(message);
}

RecoveryMessages RecoveryControl::OnCheckConnectivity()
{
	CheckConnectivityStateParam *stateParam = static_cast<CheckConnectivityStateParam *>(this->stateParam);
	RecoveryMessages status = RecoveryMessages::Disconnected;

	if (stateParam->stage != CheckConnectivityStages::ChecksCompleted)
	{
#ifndef USE_WIFI
		if (!stateParam->ping.asyncComplete(stateParam->pingResult))
		{
			delay(100);
		 	return RecoveryMessages::None;
		}

		// We get here either when ping succeeded, failed, or timedout.
		if (stateParam->pingResult.status == SUCCESS)
			status = RecoveryMessages::Connected;
#else
		if (stateParam->attempts < MAX_PING_ATTEMPTS)
		{
			if (ping_start(stateParam->address, 1, 0, 0, 1))
				status = RecoveryMessages::Connected;
			else
			{
				stateParam->attempts++;
#ifdef DEBUG_RECOVERY_CONTROL
				Tracef("Ping attempt no. %d failed, address %s\n", stateParam->attempts, stateParam->address.toString().c_str());
#endif
				delay(100);
				return RecoveryMessages::None;
			}
		}
#endif
#ifdef DEBUG_RECOVERY_CONTROL
		TRACE_BLOCK
		{
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
		lanConnected = status == RecoveryMessages::Connected;
		if (lanConnected)
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
		this->stateParam = NULL;
	}

    return status; 
}

RecoveryMessages RecoveryControl::UpdateRecoveryState(RecoveryMessages message)
{
	if (message == RecoveryMessages::Done)
		return message;

	if (message == RecoveryMessages::Connected)
	{
		if (lastRecovery == INT32_MAX)
			lastRecovery = t_now;
		RaiseRecoveryStateChanged(RecoveryTypes::NoRecovery, m_recoverySource);
	}
	else
	{
		lastRecovery = INT32_MAX;
	}
	
	return message;
}

RecoveryMessages RecoveryControl::DecideRecoveryPath(RecoveryMessages message)
{
	if (message != RecoveryMessages::Done)
	{
		if (message != RecoveryMessages::Connected)
		{
			m_recoverySource = RecoverySource::Auto;
            if (!AppConfig::getAutoRecovery() && !updateConnState)
            {
                lastRecovery = INT32_MAX;
                RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, m_recoverySource);
                return message;
            }

			if (Config::singleDevice ||
				!lanConnected || 
				lastRecovery == INT32_MAX || 
				t_now - lastRecovery > Config::skipRouterTime)
				message = RecoveryMessages::DisconnectRouter;
			else
				message = lastRecoveryType == RecoveryTypes::Router ? RecoveryMessages::DisconnectModem : RecoveryMessages::DisconnectRouter;
		}
		if (updateConnState)
		{
			UpdateRecoveryState(message);
			updateConnState = false;
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

RecoveryMessages RecoveryControl::OnWaitConnectionTestPeriod()
{
	RecoveryMessages requestedRecovery = RecoveryMessages::None;
	{
		Lock lock(csLock);
		xSemaphoreTake(waitSem, 0);
		if (this->requestedRecovery != RecoveryMessages::Done)
		{
			requestedRecovery = this->requestedRecovery;
			this->requestedRecovery = RecoveryMessages::Done;
			m_recoverySource = RecoverySource::UserInitiated;

			return requestedRecovery;
		}
	}

	time_t tWait = isPeriodicRestartEnabled() ? 
					min<time_t>(AppConfig::getConnectionTestPeriod(), nextPeriodicRestart - t_now) :
							    AppConfig::getConnectionTestPeriod();

#ifdef DEBUG_RECOVERY_CONTROL
	TRACE_BLOCK
	{
		Trace(__func__);
		Trace(": Waiting for ");
		Trace(tWait);
		Traceln(" Sec.");
	}
#endif

	bool isSemObtained = xSemaphoreTake(waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE;
	{
		Lock lock(csLock);
		requestedRecovery = this->requestedRecovery;
		if (isSemObtained && requestedRecovery == RecoveryMessages::Done)
		{
			// Periodic restart time had changed, we need to recalculate wait time.
			// We do it by triggering connection check. This will cause the connection test to
			// happen sooner than the usual connection test period. However, if we'd just recalculate
			// the waiting time and periodic restart is not the next scheduled operation then next
			// connection check will happen longer than the configured connection test period. It is
			// possible to recalculate the waiting time without triggering connection check sooner
			// than needed, but this will complicate the code.
			return RecoveryMessages::CheckConnectivity;
		}
		requestedRecovery = this->requestedRecovery;
		this->requestedRecovery = RecoveryMessages::Done;
	}
	
	if (requestedRecovery == RecoveryMessages::Done && 
		isPeriodicRestartEnabled() && 
		t_now >= nextPeriodicRestart)
	{
		nextPeriodicRestart = calcNextPeriodicRestart();
		requestedRecovery = RecoveryMessages::PeriodicRestart;
		m_recoverySource = RecoverySource::Periodic;
	}
	else
	{
		m_recoverySource = 
			requestedRecovery != RecoveryMessages::Done ? RecoverySource::UserInitiated : RecoverySource::Auto;
	}

	return requestedRecovery;
}

RecoveryMessages RecoveryControl::OnExitWaitConnectionTestPeriod(RecoveryMessages message)
{
	if (message == RecoveryMessages::CheckConnectivity)
	{
		RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, m_recoverySource);
		updateConnState = true;
	}

	return message;
}

void RecoveryControl::OnEnterDisconnectRouter()
{
	OnEnterDisconnectRouter(true);
}

void RecoveryControl::OnEnterDisconnectRouter(bool signalStateChanged)
{
	lastRecoveryType = RecoveryTypes::Router;
	lastRecovery = INT32_MAX;
	if (signalStateChanged)
		RaiseRecoveryStateChanged(
			Config::singleDevice? 
				RecoveryTypes::RouterSingleDevice : 
				RecoveryTypes::Router, 
			m_recoverySource);
	delay(500);
	recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
	SetRouterPowerState(PowerState::POWER_OFF);
	gwConnTest.Start(AppConfig::getRDisconnect() * 1000);
}

RecoveryMessages RecoveryControl::OnDisconnectRouter(bool shouldDelay)
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		if (t_now - recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
		{
			if (shouldDelay) delay(1000);
			return RecoveryMessages::None;
		}

#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Reconnecting Router");
#endif
		SetRouterPowerState(PowerState::POWER_ON);
		t0 = t_now;
	}

	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::OnWaitWhileRecovering(RecoveryMessages message)
{
	delay(5000);
	while (!gwConnTest.IsConnected())
		delay(100);
	return message;
}


RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterModemRecovery(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		if (t_now - recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()))
			return MaxCyclesExceeded();

		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterRouterRecovery(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		if (t_now - recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()))
			if (Config::singleDevice) // If single device , check max cycles
				return MaxCyclesExceeded();
			else
				return RecoveryMessages::DisconnectModem;

		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterPeriodicRestart(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		time_t tWait = 0;
		if (AppConfig::getPeriodicallyRestartRouter())
			tWait = static_cast<time_t>(AppConfig::getRReconnect());
		if (AppConfig::getPeriodicallyRestartModem())
			tWait = max<time_t>(tWait, AppConfig::getMReconnect());

		if (t_now - recoveryStart > tWait)
			if (Config::singleDevice || lastRecoveryType == RecoveryTypes::Modem) // If single device or last recovery type was modem, check max cycles
				return MaxCyclesExceeded();
			else
				return RecoveryMessages::DisconnectModem;

		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

void RecoveryControl::OnEnterDisconnectModem()
{
	OnEnterDisconnectModem(true);
}

void RecoveryControl::OnEnterDisconnectModem(bool signalStateChanged)
{
	lastRecoveryType = RecoveryTypes::Modem;
	lastRecovery = INT32_MAX;
	SetModemPowerState(PowerState::POWER_OFF);
	if (signalStateChanged)
		RaiseRecoveryStateChanged(RecoveryTypes::Modem, m_recoverySource);
	recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Modem");
#endif
	m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_OFF));
}

RecoveryMessages RecoveryControl::OnDisconnectModem(bool shouldDelay)
{
	if (t_now - recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
	{
		if (shouldDelay) delay(1000);
		return RecoveryMessages::None;
	}

	SetModemPowerState(PowerState::POWER_ON);
	m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return RecoveryMessages::Done;
}

RecoveryMessages RecoveryControl::MaxCyclesExceeded()
{
	cycles++;
	if (!AppConfig::getLimitCycles() || cycles < AppConfig::getRecoveryCycles())
		return RecoveryMessages::DisconnectRouter;

	RaiseRecoveryStateChanged(RecoveryTypes::Failed, RecoverySource::Auto);
	cycles = 0;
	return RecoveryMessages::Disconnected;
}

void RecoveryControl::OnEnterPeriodicRestart()
{
	RaiseRecoveryStateChanged(RecoveryTypes::Periodic, RecoverySource::Periodic);

	if (AppConfig::getPeriodicallyRestartRouter())
		OnEnterDisconnectRouter(false);

	if (AppConfig::getPeriodicallyRestartModem())
		OnEnterDisconnectModem(false);
}

RecoveryMessages RecoveryControl::OnPeriodicRestart()
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectRouter(false);
	}
	
	if (GetModemPowerState() == PowerState::POWER_OFF)
	{
		OnDisconnectModem(false);
	}

	if (GetRouterPowerState() == PowerState::POWER_ON &&
		GetModemPowerState() == PowerState::POWER_ON)
		return RecoveryMessages::Done;

	delay(1000);
	return RecoveryMessages::None;
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