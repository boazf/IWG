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

#ifndef RecoveryControl_h
#define RecoveryControl_h

#include <StateMachine.h>
#include <Observers.h>
#include <time.h>
#include <Relays.h>
#include <AppConfig.h>
#include <Lock.h>

#define Recovery_Messages \
	X(None) \
	X(Connected) \
	X(Disconnected) \
	X(Done) \
	X(DisconnectRouter) \
	X(DisconnectModem) \
	X(PeriodicRestart) \
	X(CheckConnectivity)

#define Recovery_States \
	X(Init) \
	X(CheckConnectivity) \
	X(WaitWhileConnected) \
	X(DisconnectModem) \
	X(CheckConnectivityAfterModemRecovery) \
	X(CheckConnectivityAfterRecoveryFailure) \
	X(WaitWhileRecoveryFailure) \
	X(DisconnectRouter) \
	X(CheckConnectivityAfterRouterRecovery) \
	X(PeriodicRestart) \
	X(CheckConnectivityAfterPeriodicRestart)


#define X(a) a,
enum class RecoveryMessages
{
	Recovery_Messages
};

enum class RecoveryStates
{
	Recovery_States
};
#undef X

enum class RecoveryTypes
{
	NoRecovery, 		// Internet connectivity is OK, no recovery needed
	Router, 			// Recover internet connection by disconnecting the router power and then reconnecting it
	RouterSingleDevice, // Recover internet connection for a single device by disconnecting the device power and then reconnecting it
	Modem, 				// Recover internet connection by disconnecting the modem power and then reconnecting it
	ConnectivityCheck, 	// Check connectivity, if it fails, then try to recovery cycles
	Failed, 			// Recovery cycles failed, attempted maximum recovery cycles
	Disconnected, 		// Disconnected from the internet, no recovery needed because auto-recovery is disabled
	Periodic 			// Periodic restart of the router and/or modem
};

#define RecoverySources \
	X(UserInitiated) \
	X(Auto) \
	X(Periodic)

#define X(a) a,
enum class RecoverySource : uint8_t
{
	RecoverySources
};
#undef X

/// @brief Parameters for the recovery state changed event
class RecoveryStateChangedParams
{
public:
	RecoveryStateChangedParams(RecoveryTypes recoveryType, RecoverySource source)
	{
		m_recoveryType = recoveryType; // The type of recovery that occurred
		m_source = source; // The source of the recovery event (e.g., user initiated, auto, periodic)
	}

	RecoveryTypes m_recoveryType;
	RecoverySource m_source;
};

/// @brief Parameters for the power state changed event
/// This class is used to pass the power state of a device (e.g., modem or router) when the power state changes.
class PowerStateChangedParams
{
public:
	PowerStateChangedParams(PowerState state)
	{
		m_state = state; // The new power state of the device
	}

	PowerState m_state;
};

/// @brief Parameters for the auto-recovery state changed event
/// This class is used to pass the auto-recovery state when it changes.
class AutoRecoveryStateChangedParams
{
public:
	AutoRecoveryStateChangedParams(bool autoRecovery)
	{
		m_autoRecovery = autoRecovery; // The new auto-recovery state (true if auto-recovery is enabled, false otherwise)
	}

	bool m_autoRecovery;
};

/// @brief Parameters for the maximum history record changed event
/// This class is used to pass the new maximum number of history records when it changes.
class MaxHistoryRecordChangedParams
{
public:
	MaxHistoryRecordChangedParams(int maxRecords)
	{
		m_maxRecords = maxRecords; // The new maximum number of history records
	}

	int m_maxRecords;
};

#define ON_ENTRY(fnName) static void fnName(RecoveryControl *control) { control->fnName(); }; void fnName()
#define ON_STATE(fnName) static RecoveryMessages fnName(RecoveryControl *control) { return control->fnName(); }; RecoveryMessages fnName()
#define ON_EXIT(fnName) static RecoveryMessages fnName(RecoveryMessages message, RecoveryControl *control) { return control->fnName(message); }; RecoveryMessages fnName(RecoveryMessages message)
#define ON_EVENT(classT, eventDataT, fnName) \
	static void fnName(const eventDataT &param, const void *context) \
	{ \
		classT *control = const_cast<classT *>(static_cast<const classT *>(context)); \
		control->fnName(param); \
	}; \
	void fnName(const eventDataT &param)
#define ON_APP_CONFIG_CHANGED(fnName) ON_EVENT(RecoveryControl, AppConfigChangedParam, fnName)

/// @brief The RecoveryControl class is responsible for managing the recovery process of the system.
/// It uses a state machine to handle different recovery states and transitions between them.
/// The class also provides methods to start recovery cycles, handle recovery events, and manage observers for state changes.
class RecoveryControl
{
public:
	/// @brief Constructor for the RecoveryControl class.
	RecoveryControl();
	/// @brief Destructor for the RecoveryControl class.
	~RecoveryControl();
	/// @brief Initializes the RecoveryControl instance.
	/// This method sets up the state machine with the defined states and transitions, initializes the recovery parameters,
	/// and starts the recovery control task.
	/// @note This method should be called once at the start of the application to set up the recovery control.
	void Init();
	/// @brief This method starts the recovery control state machine.
	/// It enters the starting state of the state machine.
	/// @note This method should be called once after the RecoveryControl instance is initialized.
	void Start();
	/// @brief Performs a single cycle of the recovery control state machine.
	/// This method is called periodically to allow the state machine to process events and transitions.
	/// It handles the current state of the state machine and performs necessary actions based on the current state.
	/// @note This method should be called in a loop to keep the recovery control active.
	void PerformCycle();

public:
	/// @brief Adds an observer for recovery state changes.
	/// @param handler The handler function to be called when the recovery state changes.
	/// @param context The context pointer that will be passed to the handler function.
	/// @return An integer token that can be used to remove the observer later.
	int addRecoveryStateChangedObserver(Observers<RecoveryStateChangedParams>::Handler handler, const void *context)
	{
		return m_recoveryStateChanged.addObserver(handler, context);
	}

	/// @brief Adds an observer for modem power state changes.
	/// @param handler The handler function to be called when the modem power state changes.
	/// @param context The context pointer that will be passed to the handler function.
	/// @return An integer token that can be used to remove the observer later.
	int addModemPowerStateChangedObserver(Observers<PowerStateChangedParams>::Handler handler, const void *context)
	{
		return m_modemPowerStateChanged.addObserver(handler, context);
	}

	/// @brief Adds an observer for router power state changes.
	/// @param handler The handler function to be called when the router power state changes.
	/// @param context The context pointer that will be passed to the handler function.
	/// @return An integer token that can be used to remove the observer later.
	int addRouterPowerStateChangedObserver(Observers<PowerStateChangedParams>::Handler handler, const void *context)
	{
		return m_routerPowerStateChanged.addObserver(handler, context);
	}

	/// @brief Adds an observer for auto-recovery state changes.
	/// @param handler The handler function to be called when the auto-recovery state changes.
	/// @param context The context pointer that will be passed to the handler function.
	/// @return An integer token that can be used to remove the observer later.
	int addAutoRecoveryStateChangedObserver(Observers<AutoRecoveryStateChangedParams>::Handler handler, const void *context)
	{
		return m_autoRecoveryStateChanged.addObserver(handler, context);
	}

	/// @brief Adds an observer for max history record changes.
	/// @param handler The handler function to be called when the max history record changes.
	/// @param context The context pointer that will be passed to the handler function.
	/// @return An integer token that can be used to remove the observer later.
	int addMaxHistoryRecordChangedObserver(Observers<MaxHistoryRecordChangedParams>::Handler handler, const void *context)
	{
		return m_maxHistoryRecordsChanged.addObserver(handler, context);
	}

	/// @brief Gets the current recovery state.
	RecoveryTypes GetRecoveryState()
	{
		return m_currentRecoveryState;
	}

	/// @brief Gets the time of the last recovery event.
	/// @return The time of the last recovery event as a time_t value.
	/// @note This method returns the time when the last recovery was completed.
	/// If recovery has failed or is currently on-going, it returns INT32_MAX.
	time_t GetLastRecovery()
	{
		return lastRecovery;
	}

	/// @brief Gets the current auto-recovery state.
	bool GetAutoRecovery()
	{
		return autoRecovery;
	}

public:
	/// @brief Starts the recovery cycles for a specific recovery type.
	/// @param recoveryType The type of recovery to start.
	/// The recovery type can be one of the defined RecoveryTypes, such as Router, Modem, or ConnectivityCheck.
	/// @return True if the recovery cycles were started successfully, false otherwise.
	/// @note This method is typically called when a user initiates a recovery action through the web interface or physical buttons.
	/// It will trigger the state machine to handle the recovery process accordingly.
	bool StartRecoveryCycles(RecoveryTypes recoveryType);

private:
	// @brief The state machine that manages the recovery process.
	StateMachine<RecoveryMessages, RecoveryStates, RecoveryControl> *m_pSM;
	/// @brief Observers object for handling recovery state changes.
	/// This object allows adding and removing observers that will be notified when the recovery state changes.
	/// Observers can be used to update the UI or perform other actions based on the recovery state.
	Observers<RecoveryStateChangedParams> m_recoveryStateChanged;
	/// @brief Observers object for handling modem power state changes.
	/// This object allows adding and removing observers that will be notified when the modem power state changes.
	/// Observers can be used to update the UI or perform other actions based on the modem power state.
	Observers<PowerStateChangedParams> m_modemPowerStateChanged;
	/// @brief Observers object for handling router power state changes.
	/// This object allows adding and removing observers that will be notified when the router power state changes.
	/// Observers can be used to update the UI or perform other actions based on the router power state.
	Observers<PowerStateChangedParams> m_routerPowerStateChanged;
	/// @brief Observers object for handling auto-recovery state changes.
	/// This object allows adding and removing observers that will be notified when the auto-recovery state changes.
	/// Observers can be used to update the UI or perform other actions based on the auto-recovery state.
	Observers<AutoRecoveryStateChangedParams> m_autoRecoveryStateChanged;
	/// @brief Observers object for handling maximum history record changes.
	/// This object allows adding and removing observers that will be notified when the maximum number of history records changes.
	/// Observers can be used to update the history storage size.
	Observers<MaxHistoryRecordChangedParams> m_maxHistoryRecordsChanged;

private:
	// The various entry/state/exit methods of various states in the state machine.
	ON_ENTRY(OnEnterInit);
	ON_STATE(OnInit);
	ON_EXIT(OnExitInit);
	ON_ENTRY(OnEnterCheckConnectivity);
	ON_STATE(OnCheckConnectivity);
	ON_STATE(OnWaitConnectionTestPeriod);
	ON_EXIT(OnExitWaitConnectionTestPeriod);
	ON_ENTRY(OnEnterDisconnectRouter);
	void OnEnterDisconnectRouter(bool signalStateChanged);
	RecoveryMessages OnDisconnectRouter(bool shouldDelay);
	static RecoveryMessages OnDisconnectRouter(RecoveryControl *control) { return control->OnDisconnectRouter(true); };
	ON_EXIT(OnWaitWhileRecovering);
	ON_EXIT(OnExitCheckConnectivityAfterModemRecovery);
	ON_EXIT(OnExitCheckConnectivityAfterRouterRecovery);
	ON_EXIT(OnExitCheckConnectivityAfterPeriodicRestart);
	ON_ENTRY(OnEnterDisconnectModem);
	void OnEnterDisconnectModem(bool signalStateChanged);
	RecoveryMessages OnDisconnectModem(bool shouldDelay);
	static RecoveryMessages OnDisconnectModem(RecoveryControl *control) { return control->OnDisconnectModem(true); }
	ON_EXIT(DecideRecoveryPath);
	ON_EXIT(UpdateRecoveryState);
	ON_ENTRY(OnEnterPeriodicRestart);
	ON_STATE(OnPeriodicRestart);

	/// @brief Raises the recovery state changed event.
	/// @param recoveryType The type of recovery that is being initiated.
	/// @param recoverySource The source of the recovery request.
	void RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource);

	/// @brief An observer handler for the AppConfigChanged event.
	/// This method is called when the application configuration changes.
	/// It updates the auto-recovery state and maximum history records and next periodic restart
	/// based on the new configuration.
	ON_APP_CONFIG_CHANGED(OnAppConfigChanged);

	/// @brief Recovery control task.
	/// @param param The task parameter.
	/// @note This method runs in a separate task and handles the recovery control logic by
	/// repeatedly calling the PerformCycle method.
	static void RecoveryControlTask(void *param);

	/// @brief Checks if periodic restart is enabled.
	/// @return True if periodic restart is enabled, false otherwise.
	static bool isPeriodicRestartEnabled();

	/// @brief Calculates the next periodic restart time.
	/// @return The time of the next periodic restart. If periodic restart is disabled, it returns INT32_MAX.
	/// @note This method uses the AppConfig to determine the next periodic restart time based on the current time.
	static time_t calcNextPeriodicRestart();

	/// @brief If bumps the recovery cycles attempts count and checks if the maximum number of cycles was reached.
	/// @return RecoveryMessages::DisconnectRouter if the maximum number of cycles was not reached,
	/// RecoveryMessages::Disconnected if the maximum number of cycles was reached. This return value is then
	/// used to determine the next action to take in the recovery process.
	RecoveryMessages MaxCyclesExceeded();

private:
	/// @brief The current recovery state.
	RecoveryTypes m_currentRecoveryState;
	/// @brief The source of the recovery request.
	RecoverySource m_recoverySource;
	/// @brief The last recovery time. If recovery has failed or is currently on-going, it is set to INT32_MAX.
	time_t lastRecovery;
	/// @brief Indicates whether the LAN is connected. If LAN checks are disabled it is set to true to simplify the logic.
	bool lanConnected;
	/// @brief The last recovery type that was attempted.
	RecoveryTypes lastRecoveryType;
	/// @brief The recovery message that was requested by the user. If no recovery is requested, it is set to RecoveryMessages::Done.
	RecoveryMessages requestedRecovery;
	/// @brief Indicates whether check connectivity was triggered by the user.
	/// @note It is set to true when the observers for recovery state changes are called with RecoveryMessages::CheckConnectivity.
	/// it is set to false after recovery state was updated as the result of the check connectivity.
	bool checkConnectivityWasTriggered;
	/// @brief The time when the recovery started.
	time_t recoveryStart;
	/// @brief Indicates until what time to wait in the initial state waiting for internet connection establishment.
	/// This time is in millis(). If time exceeds and no connection is established, the initial state is exited and the
	/// state machine enters the CheckConnectivity state. In anycase, whether internet connection is established or not,
	/// the state machine will enter the CheckConnectivity state. This time is used to prevent recovery cycles from
	/// starting too early, before the internet connection can be established.
	time_t t0;
	/// @brief Counts attempts of recovery cycles.
	/// This is used to limit the number of recovery cycles that can be performed.
	/// If the maximum number of cycles is reached, the state machine will enter the recovery failure state.
	int cycles;
	/// @brief Indicates whether auto-recovery is enabled.
	/// This is set to true if auto-recovery is enabled in the AppConfig.
	/// If auto-recovery is disabled, the state machine will not perform any recovery actions automatically.
	bool autoRecovery;
	/// @brief The maximum number of history records.
	int maxHistory;
	/// @brief A pointer to specific data that a state may need to use to handle the state logic.
	/// @note it is the responsibility of the state to manage this data. The state should allocate and deallocate this data as needed.
	void *stateParam;
    /// @brief A semaphore used to wait between auto-connectivity checks. The time between checks is defined in the AppConfig
	/// and sets the timeout for the semaphore. If a user requests a recovery action, the semaphore is given to allow the state
	/// machine to proceed with the recovery action.
    xSemaphoreHandle waitSem;
	/// @brief The next periodic restart time.
	/// This is calculated based on the AppConfig settings for periodic restart of the router and/or modem.
	/// If periodic restart is disabled, it is set to INT32_MAX.
	time_t nextPeriodicRestart;
	/// @brief A critical section lock used to protect shared resources in the RecoveryControl class.
	CriticalSection csLock;
};

/// @brief Global instance of the RecoveryControl class.
/// This instance is used to manage the recovery process throughout the application.
/// It should be initialized and started at the beginning of the application.
extern RecoveryControl recoveryControl;

#endif // RecoveryControl_h