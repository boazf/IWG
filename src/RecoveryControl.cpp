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
		checkConnectivityWasTriggered(false),
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
	// Initialize the state machine with the defined states and transitions
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

	// Start the recovery control task
	// This task will run the state machine and handle recovery cycles
	// It is pinned to the second core to allow it to run concurrently with other tasks
	// The stack size is set to 8KB, which should be sufficient for the recovery control operations
    xTaskCreatePinnedToCore(
        RecoveryControlTask,
        "RecoveryControlTask",
        1024*8,
        this,
        1,
        NULL,
        1 - xPortGetCoreID());

	/// @brief Register the AppConfigChanged observer to handle configuration changes
	/// @note This observer will be notified when the application configuration changes, allowing the recovery control to adapt to new settings.
	/// It also calls observers for auto-recovery state changes and maximum history records changes, if there was a change.
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

	// if auto-recovery is now enabled after it was disabled, start a connectivity check to start recovery cycles, if there is no connectivity.
	if (autoRecovery && !this->autoRecovery)
		StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);

	// If there was a change in the auto-recovery state, notify observers
	if (this->autoRecovery != autoRecovery)
		m_autoRecoveryStateChanged.callObservers(AutoRecoveryStateChangedParams(autoRecovery));

	// Update the current value of auto-recovery state
	// This will ensure that the recovery control reflects the latest configuration settings
	// and can adapt to changes in auto-recovery settings.
	this->autoRecovery = autoRecovery;

	int maxHistory = AppConfig::getMaxHistory();
	// If the maximum history records has changed, notify observers.
	if (maxHistory != this->maxHistory)
	{
		m_maxHistoryRecordsChanged.callObservers(MaxHistoryRecordChangedParams(maxHistory));
		this->maxHistory = maxHistory;
	}

	// Recalculate the next periodic restart time.
	time_t nextPeriodicRestart = calcNextPeriodicRestart();
	// if periodic restart is now enabled and it was previously disabled, or if the next periodic restart time is earlier than previous one,
	// signal the recovery wait semaphore so that the wait state can be updated with the new periodic restart time.
	bool shouldWakeRecoveryThread = 
		nextPeriodicRestart != -1 && (this->nextPeriodicRestart == -1 || this->nextPeriodicRestart > nextPeriodicRestart);
	// Update the next periodic restart time
	this->nextPeriodicRestart = nextPeriodicRestart;
	// Signal the wait semaphore if needed
	if (shouldWakeRecoveryThread)
		xSemaphoreGive(waitSem);
}

void RecoveryControl::PerformCycle()
{
	m_pSM->HandleState();
}

/// @brief The connection check state is actually a state machine by itself. Only it is not implemented using states and transitions,
/// but with a state class that manages the various states of the connectivity check process. The state class is CheckConnectivityStateParam.
/// The state of the connectivity check is determined by the CheckConnectivityStages enum, which manages the current stage of the connectivity check process.
/// Each time the connectivity check should progress to the next stage, the state is exited with RecoveryMessages::Done, then the state is re-entered
/// and the next stage is performed, until the state becomes CheckConnectivityStages::ChecksCompleted.
enum class CheckConnectivityStages
{
	CheckLAN,
	CheckServer1,
	CheckServer2,
	ChecksCompleted
};

/// @brief The parameters for the check connectivity state.
/// This class holds the state of the connectivity check process, including the current stage, the ping object for performing pings,
/// and the status of the connectivity check.
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

/// @brief This method is called when entering the CheckConnectivity state.
/// It initializes the CheckConnectivityStateParam state parameter, which holds the current stage of the connectivity check process.
/// The method checks the LAN address first, then attempts to ping the first server, and if that fails, it tries the second server.
/// If both servers are unreachable, it marks the stage as ChecksCompleted and the status as Disconnected.
void RecoveryControl::OnEnterCheckConnectivity()
{
	CheckConnectivityStateParam *stateParam = static_cast<CheckConnectivityStateParam *>(this->stateParam);
	// If stateParam is set to NULL, it means this is the first time we are entering this state.
	// We need to initialize it with a new instance of CheckConnectivityStateParam.
	// If it is not NULL, it means we are re-entering the state after a previous exit,
	// and we can reuse the existing state parameter.
	// This allows us to keep the state of the connectivity check process across the various stages of the connectivity check.
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
			// If the LAN address is zero, it means we are not configured to check LAN connectivity.
			// We skip this stage and set lanConnected to true to simplify the logic.
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			lanConnected = true;
		}
	}

	String server;

	if (stateParam->stage == CheckConnectivityStages::CheckServer1)
	{
		server = AppConfig::getServer1();
		// Try to get the host address of the first server.
		// If it fails, we move to the next stage to check the second server.
		if (!TryGetHostAddress(address, server))
			stateParam->stage = CheckConnectivityStages::CheckServer2;
	}

	if (stateParam->stage == CheckConnectivityStages::CheckServer2)
	{
		server = AppConfig::getServer2();
		// Try to get the host address of the second server.
		// If it fails, we mark the stage as ChecksCompleted, indicating that we have completed the connectivity checks.
		// This will allow us to exit the state and handle the result of the connectivity check.
		// If it succeeds, we will proceed to ping the server in the state handler.
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

	// If we are in the ChecksCompleted stage, we do not need to ping any address.
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

/// @brief This method is called when entering the Init state
void RecoveryControl::OnEnterInit()
{
	// Calculate the time until when to wait for connectivity to be established.
	// It is calculated by finding the maximum between modem and router reconnect times in milliseconds and adding the current time in milliseconds.
	t0 = max<int>(AppConfig::getRReconnect(), AppConfig::getMReconnect()) * 1000 + millis();
	// Calclate the next periodic restart time.
	nextPeriodicRestart = calcNextPeriodicRestart();
	// cycles is generally used to count the number of recovery attempt made to reestablish connectivity. However in the init state it is used to 
	// count the number of seconds since the start of the application.
	cycles = millis() / 1000;
}

/// @brief This method is called when the Init state is executed.
RecoveryMessages RecoveryControl::OnInit()
{
	if (millis() > t0)
	{
		// If maximum time to wait for connectivity has passed, we exit the init state with a Disconnected message.
#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Timeout: could not establish connectivity upon initialization, starting recovery cycles");
#endif
		return RecoveryMessages::Disconnected;
	}

	// Wait one second in a busy loop.
	if (millis() > 1000 * cycles)
	{
		String server;
		if (AppConfig::getServer1().isEmpty() || AppConfig::getServer2().isEmpty())
		{
			// If only one server is configured, we use it for connectivity checks.
			server = AppConfig::getServer1().isEmpty() ? AppConfig::getServer2() : AppConfig::getServer1();
		}
		else
		{
			// If both servers are configured, we alternate between them for connectivity checks.
			server = (cycles % 2 == 0) ? AppConfig::getServer1() : AppConfig::getServer2();
		}

		IPAddress address;

		if (TryGetHostAddress(address, server))
		{
			// If we successfully got the host address, we exit the init state.
			return RecoveryMessages::Connected;
		}
		cycles++;
	}

	// Re-enter the init state to continue waiting for connectivity.
	return RecoveryMessages::None;
}

/// @brief This method is called when exiting the Init state.
RecoveryMessages RecoveryControl::OnExitInit(RecoveryMessages message)
{
	cycles = 0;
	return UpdateRecoveryState(message);
}

bool RecoveryControl::isPeriodicRestartEnabled()
{
	// Periodic restart is enabled if either the modem or router is set to periodically restart,
	// and auto-recovery is enabled.
	return (AppConfig::getPeriodicallyRestartModem() || AppConfig::getPeriodicallyRestartRouter()) && AppConfig::getAutoRecovery();
}

#define SECONDS_IN_24HOURS (24 * 60 * 60)

time_t RecoveryControl::calcNextPeriodicRestart()
{
	// If periodic restart is not enabled, return -1 to indicate no periodic restart is scheduled.
	if (!isPeriodicRestartEnabled())
		return -1;

	// Calculate the last midnight time in seconds since epoch.
	time_t now = t_now;
	tm tr;
	localtime_r(&now, &tr);
	tr.tm_hour = 0;
	tr.tm_min = 0;
	tr.tm_sec = 0;
	time_t lastMidnight = mktime(&tr);
	// Calculate the next periodic restart time based on the last midnight time and the configured periodic restart time.
	// The periodic restart time is retrieved from the application configuration.
	// If the next periodic restart time is less than or equal to the current time, it means we need to schedule it for the next day.
	// Otherwise, we keep the calculated time.
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

// @brief This method is called when executing the CheckConnectivity state.
RecoveryMessages RecoveryControl::OnCheckConnectivity()
{
	CheckConnectivityStateParam *stateParam = static_cast<CheckConnectivityStateParam *>(this->stateParam);
	RecoveryMessages status = RecoveryMessages::Disconnected;

	if (stateParam->stage != CheckConnectivityStages::ChecksCompleted)
	{
		// Ping the address based on the current stage of the connectivity check.
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

	// Update the status based on the ping result. If we're in the ChecksCompleted stage, the status is set to Disconnected.
	stateParam->status = status;

	switch (stateParam->stage)
	{
	case CheckConnectivityStages::CheckLAN:
		lanConnected = status == RecoveryMessages::Connected;
		if (lanConnected)
		{
			// If LAN is connected, we proceed to check the first server.
			stateParam->stage = CheckConnectivityStages::CheckServer1;
			status = RecoveryMessages::Done;
		}
		else
		{
			// If LAN is not connected, we skip to the ChecksCompleted stage. This means that connectivity check failed.
			stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		}
		break;
	case CheckConnectivityStages::CheckServer1:
		if (status == RecoveryMessages::Connected)
		{
			// If the first server is reachable, we can stop the connectivity checks.
			stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		}
		else
		{
			// If the first server is not reachable, we proceed to check the second server.
			stateParam->stage = CheckConnectivityStages::CheckServer2;
			status = RecoveryMessages::Done;
		}
		break;
	case CheckConnectivityStages::CheckServer2:
		// Whether the second server is reachable or not, we mark the stage as ChecksCompleted.
		stateParam->stage = CheckConnectivityStages::ChecksCompleted;
		break;
	case CheckConnectivityStages::ChecksCompleted:
		break;
	}

	// Do not move this code to the switch
	if (stateParam->stage == CheckConnectivityStages::ChecksCompleted)
	{
		// If we reached the ChecksCompleted stage, we need to free the state param object.
		delete stateParam;
		this->stateParam = NULL;
	}

    return status; 
}

/// @brief This method is called when exiting the CheckConnectivity state after the connectivity checks are completed.
/// It updates the recovery state based on the connectivity check result.
/// @param message The message indicating the result of the connectivity check.
/// @return Same as the message parameter.
RecoveryMessages RecoveryControl::UpdateRecoveryState(RecoveryMessages message)
{
	if (message == RecoveryMessages::Done)
		// This means that we should continue to the next stage of the connectivity check.
		return message;

	if (message == RecoveryMessages::Connected)
	{
		if (lastRecovery == INT32_MAX)
			lastRecovery = t_now;
		RaiseRecoveryStateChanged(RecoveryTypes::NoRecovery, m_recoverySource);
	}
	else
	{
		// We get here if connectivity check has failed.
		lastRecovery = INT32_MAX;
	}
	
	return message;
}

/// @brief This method is called when exiting the CheckConnectivity state.
RecoveryMessages RecoveryControl::DecideRecoveryPath(RecoveryMessages message)
{
	if (message == RecoveryMessages::Done)
		// This means that we should continue to the next stage of the connectivity check.
		return message;

	if (message != RecoveryMessages::Connected)
	{
		// If we are not connected, we need to decide what to do next based on the last recovery type and the current connectivity state.
		m_recoverySource = RecoverySource::Auto;
		if (!AppConfig::getAutoRecovery() && !checkConnectivityWasTriggered)
		{
			// If auto-recovery is disabled and the user did not trigger a connectivity check, we just signal that we are disconnected
			// and do not perform any recovery actions.
			lastRecovery = INT32_MAX;
			RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, m_recoverySource);
			return message;
		}

		if (Config::singleDevice ||
			!lanConnected || 
			lastRecovery == INT32_MAX || 
			t_now - lastRecovery > Config::skipRouterTime)
			// In case of a single device, or if LAN is not connected, or if the last recovery was long enough ago, Do a router recovery.
			message = RecoveryMessages::DisconnectRouter;
		else
			// Otherwise, alternate between modem and router recovery.
			message = lastRecoveryType == RecoveryTypes::Router ? RecoveryMessages::DisconnectModem : RecoveryMessages::DisconnectRouter;
	}
	if (checkConnectivityWasTriggered)
	{
		// If the connectivity check was triggered by the user, we need to update the connectivity state.
		UpdateRecoveryState(message);
		checkConnectivityWasTriggered = false;
	}

	return message;
}

// @brief This method is called when the observers for recovery state change should be called.
// It raises the recovery state changed event with the current recovery type and source.
// @param recoveryType The type of recovery that is currently being performed.
// @param recoverySource The source of the recovery request, indicating whether it was periodic, user-initiated or automatic.
void RecoveryControl::RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource)
{
	m_currentRecoveryState = recoveryType;
	if (m_currentRecoveryState == RecoveryTypes::RouterSingleDevice)
		m_currentRecoveryState = RecoveryTypes::Router;
    RecoveryStateChangedParams params(recoveryType, recoverySource);
	m_recoveryStateChanged.callObservers(params);
}

/// @brief This method is called in order to wait the time between connection tests.
/// It waits for the specified time or until a recovery is requested by the user.
/// @return The recovery message that was requested, if any.
RecoveryMessages RecoveryControl::OnWaitConnectionTestPeriod()
{
	RecoveryMessages requestedRecovery = RecoveryMessages::None;
	{
		// See if there is already a requested recovery from the user.
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

	// If there is no requested recovery, we wait for the next periodic restart time
	// or the connection test period whichever comes first.
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

	// Wait for the specified time or until a recovery is requested by the user.
	bool isSemObtained = xSemaphoreTake(waitSem, (tWait * 1000) / portTICK_PERIOD_MS) == pdTRUE;
	{
		Lock lock(csLock);
		requestedRecovery = this->requestedRecovery;
		if (isSemObtained && requestedRecovery == RecoveryMessages::Done)
		{
			// We get here if periodic restart time has changed. In this case, we need to recalculate the wait time.
			// We do it by triggering connection check. This will cause the connection test to
			// happen sooner than the usual connection test period. However, if we'd just recalculate
			// the waiting time and periodic restart is not the next scheduled operation then next
			// connection check will happen longer than the configured connection test period. It is
			// possible to recalculate the waiting time without triggering connection check sooner
			// than needed, but this will complicate the code.
			return RecoveryMessages::CheckConnectivity;
		}
		// So, there is a requested recovery from the user.
		requestedRecovery = this->requestedRecovery;
		this->requestedRecovery = RecoveryMessages::Done;
	}
	
	if (requestedRecovery == RecoveryMessages::Done && 
		isPeriodicRestartEnabled() && 
		t_now >= nextPeriodicRestart)
	{
		// It is time for a periodic restart.
		nextPeriodicRestart = calcNextPeriodicRestart();
		requestedRecovery = RecoveryMessages::PeriodicRestart;
		m_recoverySource = RecoverySource::Periodic;
	}
	else
	{
		// If we are here, it means that the user requested a recovery or it is time for another connectivity check cycle.
		m_recoverySource = 
			requestedRecovery != RecoveryMessages::Done ? RecoverySource::UserInitiated : RecoverySource::Auto;
	}

	return requestedRecovery;
}

/// @brief This method is called when exiting the state of waiting between connection tests.
/// @param message  The message indicating the cause for exiting the wait state.
/// @return Same as the message parameter.
RecoveryMessages RecoveryControl::OnExitWaitConnectionTestPeriod(RecoveryMessages message)
{
	if (message == RecoveryMessages::CheckConnectivity)
	{
		RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, m_recoverySource);
		checkConnectivityWasTriggered = true;
	}

	return message;
}

/// @brief This method is called when entering the state of disconnecting the router.
void RecoveryControl::OnEnterDisconnectRouter()
{
	OnEnterDisconnectRouter(true);
}

/// @brief This method is called when entering the state of disconnecting the router.
/// @param signalStateChanged If true, it signals that the recovery state has changed.
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

/// @brief 	This method is called when the router is disconnected.
/// @param shouldDelay If true, it delays for 1 second before returning.
/// @return RecoveryMessages::None if the router is still disconnecting, RecoveryMessages::Done if the router is reconnected.
RecoveryMessages RecoveryControl::OnDisconnectRouter(bool shouldDelay)
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		if (t_now - recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
		{
			// If disconnection time did not yet pass then keep on waiting in a loop.
			if (shouldDelay) delay(1000);
			return RecoveryMessages::None;
		}

#ifdef DEBUG_RECOVERY_CONTROL
		Traceln("Reconnecting Router");
#endif
		// If disconnection time has passed, we can reconnect the router.
		SetRouterPowerState(PowerState::POWER_ON);
	}

	return RecoveryMessages::Done;
}

/// @brief This method is called when exiting a recovery state. it is used to wait for the router to become connected again.
/// @param message The message indicating the result of the recovery operation.
/// @return Same as the message parameter.
RecoveryMessages RecoveryControl::OnWaitWhileRecovering(RecoveryMessages message)
{
	delay(5000);
	while (!gwConnTest.IsConnected())
		delay(100);
	return message;
}

/// @brief This method is called when exiting the state of checking connectivity after modem recovery.
/// @param message The message indicating the result of the recovery operation.
/// @return Message that indicates the transition that should be made after checking connectivity.
RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterModemRecovery(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		if (t_now - recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()))
			// If connection was not established after configured modem reconnect time,
			// we check if maximum recovery cycles were exceeded.
			return MaxCyclesExceeded();

		// We can still wait for connection to be established. So, wait for a while and restart the connection test.
		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

/// @brief This method is called when exiting the state of checking connectivity after router recovery.
/// @param message The message indicating the result of the recovery operation.
/// @return Message that indicates the transition that should be made after checking connectivity.
RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterRouterRecovery(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		if (t_now - recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()))
			// If connection was not established after configured router reconnect time,
			// we check if maximum recovery cycles were exceeded.
			if (Config::singleDevice) // If single device , check max cycles
				return MaxCyclesExceeded();
			else
				// If not single device, we should try recover the modem.
				return RecoveryMessages::DisconnectModem;

		// We can still wait for connection to be established. So, wait for a while and restart the connection test.
		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

/// @brief This method is called when exiting the state of checking connectivity after periodic restart.
/// @param message The message indicating the result of the recovery operation.
/// @return Message that indicates the transition that should be made after checking connectivity.
RecoveryMessages RecoveryControl::OnExitCheckConnectivityAfterPeriodicRestart(RecoveryMessages message)
{
	message = UpdateRecoveryState(message);

	if (message == RecoveryMessages::Disconnected)
	{
		// Calculate the maximum time that should for a connection to be established after a periodic restart.
		time_t tWait = 0;
		if (AppConfig::getPeriodicallyRestartRouter())
			tWait = static_cast<time_t>(AppConfig::getRReconnect());
		if (AppConfig::getPeriodicallyRestartModem())
			tWait = max<time_t>(tWait, AppConfig::getMReconnect());

		// tWait holds now the maximum of modem reconnect and router reconnect times. If only one of them is enabled for periodic restart,
		// then it will hold the reconnect time for that device.
		if (t_now - recoveryStart > tWait)
			// Connection was not established after the maximum reconnect time,
			if (Config::singleDevice || lastRecoveryType == RecoveryTypes::Modem)
				// If single device or last recovery type was modem, check max cycles
				return MaxCyclesExceeded();
			else
				// If not single device and last recovery type was router, we should try recover the modem.
				return RecoveryMessages::DisconnectModem;

		// We can still wait for connection to be established. So, wait for a while and restart the connection test.
		return OnWaitWhileRecovering(RecoveryMessages::Done);
	}

	return message;
}

/// @brief This method is called when entering the state of disconnecting the modem.
void RecoveryControl::OnEnterDisconnectModem()
{
	OnEnterDisconnectModem(true);
}

/// @brief This method is called when entering the state of disconnecting the modem.
/// @param signalStateChanged If true, it signals that the recovery state has changed.
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

/// @brief This method is called when the modem is disconnected.
/// @param shouldDelay If true, it delays for 1 second before returning.
/// @return RecoveryMessages::None if the modem should still be disconnected, RecoveryMessages::Done if the modem is reconnected.
RecoveryMessages RecoveryControl::OnDisconnectModem(bool shouldDelay)
{
	if (t_now - recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
	{
		// If disconnection time did not yet pass then keep on waiting in a loop.
		if (shouldDelay) delay(1000);
		return RecoveryMessages::None;
	}

	// If disconnection time has passed, we can reconnect the modem.
	SetModemPowerState(PowerState::POWER_ON);
	// Notify observers that the modem power state has changed to POWER_ON.
	m_modemPowerStateChanged.callObservers(PowerStateChangedParams(PowerState::POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return RecoveryMessages::Done;
}

/// @brief This method is called to check if the maximum number of recovery cycles has been exceeded.
/// @return RecoveryMessages::DisconnectRouter if the maximum number of recovery cycles has not been exceeded,
///         RecoveryMessages::Disconnected if the maximum number of recovery cycles has been exceeded.
///         This return value is used to perform the next transition in the recovery process.
/// @note We return RecoveryMessages::DisconnectRouter if the maximum number of recovery cycles has not been exceeded,
///       because we check maximum cycles after modem recovery, so if cycles did not exceed the limit,
///       we should then try to recover the router. In case of single device, we should also return RecoveryMessages::DisconnectModem.
///       since this is the only device we have to recover.
RecoveryMessages RecoveryControl::MaxCyclesExceeded()
{
	cycles++;
	if (!AppConfig::getLimitCycles() || cycles < AppConfig::getRecoveryCycles())
		// Maximum number of recovery cycles has not been exceeded, so we should try to recover the router.
		return RecoveryMessages::DisconnectRouter;

	// Maximum number of recovery cycles has been exceeded, so we should signal that recovery has failed.
	RaiseRecoveryStateChanged(RecoveryTypes::Failed, RecoverySource::Auto);
	// Reset the recovery cycle count.
	cycles = 0;
	// Continue with the recovery process by signaling that we are disconnected.
	return RecoveryMessages::Disconnected;
}

/// @brief This method is called when entering the state of periodic restart.
void RecoveryControl::OnEnterPeriodicRestart()
{
	// Call observers to notify that state has changed to periodic restart.
	RaiseRecoveryStateChanged(RecoveryTypes::Periodic, RecoverySource::Periodic);

	// If periodic restart is enabled for the router, we should disconnect power to it.
	if (AppConfig::getPeriodicallyRestartRouter())
		OnEnterDisconnectRouter(false);

	// If periodic restart is enabled for the modem, we should disconnect power to it.
	if (AppConfig::getPeriodicallyRestartModem())
		OnEnterDisconnectModem(false);
}

/// @brief This method is called when the periodic restart state is executed.
/// @return RecoveryMessages::None if the periodic restart should continue, 
///         RecoveryMessages::Done if it should restart has ended and power is back to normal.
RecoveryMessages RecoveryControl::OnPeriodicRestart()
{
	if (GetRouterPowerState() == PowerState::POWER_OFF)
	{
		// If periodic restart is enabled for the router then the power to the router is supposed to be off.
		// This was done upon entering the state. Now we need to reconnect the power to the router when the
		// configured time has passed. This is done in the OnDisconnectRouter method.
		OnDisconnectRouter(false);
	}
	
	if (GetModemPowerState() == PowerState::POWER_OFF)
	{
		// If periodic restart is enabled for the modem then the power to the modem is supposed to be off.
		// This was done upon entering the state. Now we need to reconnect the power to the modem when the
		// configured time has passed. This is done in the OnDisconnectModem method.
		OnDisconnectModem(false);
	}

	// If both modem and router are powered on, we can exit the periodic restart state.
	if (GetRouterPowerState() == PowerState::POWER_ON &&
		GetModemPowerState() == PowerState::POWER_ON)
		return RecoveryMessages::Done;

	// If we are here, it means that either the modem or the router is still powered off.
	// So wait for a one second and repeat the state execution.
	delay(1000);
	return RecoveryMessages::None;
}

bool RecoveryControl::StartRecoveryCycles(RecoveryTypes recoveryType)
{
	Lock lock(csLock);

	// If the requested recovery is not Done, it means that we are already in the process of recovering,
	// so we should not start a new recovery cycle.
	if (requestedRecovery != RecoveryMessages::Done)
		return false;

	// Reset the attempted recovery cycles count.
	cycles = 0;
	// Set the requested recovery type based on the recovery type passed to this method.
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
			return false;
	}

	// Give the wait semaphore to wake up the recovery thread.
	// In case it is in the wait state between connection tests.
	return xSemaphoreGive(waitSem) == pdTRUE;
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