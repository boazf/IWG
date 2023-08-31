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
	X(Timeout) \
	X(NoTimeout) \
	X(Exceeded) \
	X(NotExceeded) \
	X(DisconnectRouter) \
	X(DisconnectModem) \
	X(PeriodicRestart) \
	X(CheckConnectivity) \
	X(HWError)

#define Recovery_States \
	X(Init) \
	X(CheckConnectivity) \
	X(StartCheckConnectivity) \
	X(WaitWhileConnected) \
	X(DisconnectModem) \
	X(WaitAfterModemRecovery) \
	X(CheckConnectivityAfterModemRecovery) \
	X(CheckModemRecoveryTimeout) \
	X(CheckMaxCyclesExceeded) \
	X(CheckConnectivityAfterRecoveryFailure) \
	X(WaitWhileRecoveryFailure) \
	X(DisconnectRouter) \
	X(CheckConnectivityAfterRouterRecovery) \
	X(WaitAfterRouterRecovery) \
	X(CheckRouterRecoveryTimeout) \
	X(PeriodicRestart) \
	X(WaitAfterPeriodicRestart) \
	X(CheckConnectivityAfterPeriodicRestart) \
	X(CheckPeriodicRestartTimeout) \
	X(HWError)


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

enum class RecoverySource : uint8_t
{
	UserInitiated,
	Auto,
	Periodic
};

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

class RecoveryControl
{
public:
	RecoveryControl();
	~RecoveryControl();
	void Init();
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
	static void OnEnterInit(RecoveryControl *control);
	static RecoveryMessages OnInit(RecoveryControl *control);
	static void OnEnterCheckConnectivity(RecoveryControl *control);
	static RecoveryMessages OnCheckConnectivity(RecoveryControl *control);
	static RecoveryMessages OnWaitConnectionTestPeriod(RecoveryControl *control);
	static RecoveryMessages OnStartCheckConnectivity(RecoveryControl *control);
	static void OnEnterDisconnectRouter(RecoveryControl *control);
	static void OnEnterDisconnectRouter(RecoveryControl *control, bool signalStateChanged);
	static RecoveryMessages OnDisconnectRouter(RecoveryControl *control);
	static RecoveryMessages OnWaitWhileRecovering(RecoveryControl *control);
	static RecoveryMessages OnCheckRouterRecoveryTimeout(RecoveryControl *control);
	static RecoveryMessages OnExitCheckRouterRecoveryTimeout(RecoveryMessages message, RecoveryControl *control);
	static void OnEnterDisconnectModem(RecoveryControl *control);
	static void OnEnterDisconnectModem(RecoveryControl *control, bool signalStateChanged);
	static RecoveryMessages OnDisconnectModem(RecoveryControl *control);
	static RecoveryMessages OnCheckModemRecoveryTimeout(RecoveryControl *control);
	static RecoveryMessages OnCheckMaxCyclesExceeded(RecoveryControl *control);
	static void OnEnterHWError(RecoveryControl *control);
	static RecoveryMessages OnHWError(RecoveryControl *control);
	static RecoveryMessages DecideRecoveryPath(RecoveryMessages message, RecoveryControl *control);
	static RecoveryMessages UpdateRecoveryState(RecoveryMessages message, RecoveryControl *control);
	static void OnEnterPeriodicRestart(RecoveryControl *control);
	static RecoveryMessages OnPeriodicRestart(RecoveryControl *control);
	static RecoveryMessages OnCheckPeriodicRestartTimeout(RecoveryControl *control);
	static RecoveryMessages DecideUponPeriodicRestartTimeout(RecoveryMessages message, RecoveryControl *control);
	void RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource);
	static void AppConfigChanged(const AppConfigChangedParam &param, const void *context);
	static void RecoveryControlTask(void *param);
	static bool isPeriodicRestartEnabled();
	static time_t calcNextPeriodicRestart();

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