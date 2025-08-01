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
	NoRecovery,
	Router,
	RouterSingleDevice,
	Modem,
	ConnectivityCheck,
	Failed,
	HWFailure,
	Disconnected,
	Periodic
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

class RecoveryStateChangedParams
{
public:
	RecoveryStateChangedParams(RecoveryTypes recoveryType, RecoverySource source)
	{
		m_recoveryType = recoveryType;
		m_source = source;
	}

	RecoveryTypes m_recoveryType;
	RecoverySource m_source;
};

class PowerStateChangedParams
{
public:
	PowerStateChangedParams(PowerState state)
	{
		m_state = state;
	}

	PowerState m_state;
};

class AutoRecoveryStateChangedParams
{
public:
	AutoRecoveryStateChangedParams(bool autoRecovery)
	{
		m_autoRecovery = autoRecovery;
	}

	bool m_autoRecovery;
};

class MaxHistoryRecordChangedParams
{
public:
	MaxHistoryRecordChangedParams(int maxRecords)
	{
		m_maxRecords = maxRecords;
	}

	int m_maxRecords;
};

#define ON_ENTRY(fnName) static void fnName(RecoveryControl *control) { control->fnName(); }; void fnName()
#define ON_STATE(fnName) static RecoveryMessages fnName(RecoveryControl *control) { return control->fnName(); }; RecoveryMessages fnName()
#define ON_EXIT(fnName) static RecoveryMessages fnName(RecoveryMessages message, RecoveryControl *control) { return control->fnName(message); }; RecoveryMessages fnName(RecoveryMessages message)
#define ON_APP_CONFIG_CHANGED(fnName) \
	static void fnName(const AppConfigChangedParam &param, const void *context) \
	{ \
		RecoveryControl *control = const_cast<RecoveryControl *>(static_cast<const RecoveryControl *>(context)); \
		control->fnName(param); \
	}; \
	void fnName(const AppConfigChangedParam &param)

class RecoveryControl
{
public:
	RecoveryControl();
	~RecoveryControl();
	void Init();
	void Start();
	void PerformCycle();

public:
	Observers<RecoveryStateChangedParams> &GetRecoveryStateChanged()
	{
		return m_recoveryStateChanged;
	}

	Observers<PowerStateChangedParams> &GetModemPowerStateChanged()
	{
		return m_modemPowerStateChanged;
	}

	Observers<PowerStateChangedParams> &GetRouterPowerStateChanged()
	{
		return m_routerPowerStateChanged;
	}

	Observers<AutoRecoveryStateChangedParams> &GetAutoRecoveryStateChanged()
	{
		return m_autoRecoveryStateChanged;
	}

	Observers<MaxHistoryRecordChangedParams> &GetMaxHistoryRecordsChanged()
	{
		return m_maxHistoryRecordsChanged;
	}

	RecoveryTypes GetRecoveryState()
	{
		return m_currentRecoveryState;
	}

	time_t GetLastRecovery()
	{
		return lastRecovery;
	}

	bool GetAutoRecovery()
	{
		return autoRecovery;
	}

public:
	void StartRecoveryCycles(RecoveryTypes recoveryType);

private:
	StateMachine<RecoveryMessages, RecoveryStates, RecoveryControl> *m_pSM;
	Observers<RecoveryStateChangedParams> m_recoveryStateChanged;
	Observers<PowerStateChangedParams> m_modemPowerStateChanged;
	Observers<PowerStateChangedParams> m_routerPowerStateChanged;
	Observers<AutoRecoveryStateChangedParams> m_autoRecoveryStateChanged;
	Observers<MaxHistoryRecordChangedParams> m_maxHistoryRecordsChanged;

private:
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
	void RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource);
	ON_APP_CONFIG_CHANGED(OnAppConfigChanged);
	static void RecoveryControlTask(void *param);
	static bool isPeriodicRestartEnabled();
	static time_t calcNextPeriodicRestart();
	RecoveryMessages MaxCyclesExceeded();

private:
	RecoveryTypes m_currentRecoveryState;
	RecoverySource m_recoverySource;
	time_t lastRecovery;
	bool lanConnected;
	RecoveryTypes lastRecoveryType;
	RecoveryMessages requestedRecovery;
	bool updateConnState;
	time_t recoveryStart;
	time_t t0;
	int cycles;
	bool autoRecovery;
	int maxHistory;
	void *stateParam;
    xSemaphoreHandle waitSem;
	time_t nextPeriodicRestart;
	CriticalSection csLock;
};

extern RecoveryControl recoveryControl;

#endif // RecoveryControl_h