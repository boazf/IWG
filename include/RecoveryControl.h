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

class RecoveryControl;

class SMParam
{
public:
	SMParam(RecoveryControl *recoveryControl, time_t _lastRecovery, RecoverySource recoverySource = RecoverySource::Auto) :
		m_recoveryControl(recoveryControl),
		m_recoverySource(recoverySource),
		lastRecovery(_lastRecovery),
		lanConnected(false),
		lastRecoveryType(RecoveryTypes::NoRecovery),
		requestedRecovery(RecoveryMessages::Done),
		updateConnState(false),
		cycles(0),
		autoRecovery(AppConfig::getAutoRecovery()),
		maxHistory(AppConfig::getMaxHistory()),
		stateParam(NULL),
		nextPeriodicRestart(-1)
	{
		waitSem = xSemaphoreCreateBinary();
	}

	RecoveryControl *m_recoveryControl;
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
		return m_param->lastRecovery;
	}

	bool GetAutoRecovery()
	{
		return m_param->autoRecovery;
	}

public:
	void StartRecoveryCycles(RecoveryTypes recoveryType);

private:
	StateMachine<RecoveryMessages, RecoveryStates, SMParam> *m_pSM;
	SMParam *m_param;
	Observers<RecoveryStateChangedParams> m_recoveryStateChanged;
	Observers<PowerStateChangedParams> m_modemPowerStateChanged;
	Observers<PowerStateChangedParams> m_routerPowerStateChanged;
	Observers<AutoRecoveryStateChangedParams> m_autoRecoveryStateChanged;
	Observers<MaxHistoryRecordChangedParams> m_maxHistoryRecordsChanged;
	RecoveryTypes m_currentRecoveryState;

private:
	static void OnEnterInit(SMParam *smParam);
	static RecoveryMessages OnInit(SMParam *smParam);
	static void OnEnterCheckConnectivity(SMParam *smParam);
	static RecoveryMessages OnCheckConnectivity(SMParam *smParam);
	static RecoveryMessages OnWaitConnectionTestPeriod(SMParam *smParam);
	static RecoveryMessages OnStartCheckConnectivity(SMParam *smParam);
	static void OnEnterDisconnectRouter(SMParam *smParam);
	static void OnEnterDisconnectRouter(SMParam *smParam, bool signalStateChanged);
	static RecoveryMessages OnDisconnectRouter(SMParam *smParam);
	static RecoveryMessages OnWaitWhileRecovering(SMParam *smParam);
	static RecoveryMessages OnCheckRouterRecoveryTimeout(SMParam *smParam);
	static void OnEnterDisconnectModem(SMParam *smParam);
	static void OnEnterDisconnectModem(SMParam *smParam, bool signalStateChanged);
	static RecoveryMessages OnDisconnectModem(SMParam *smParam);
	static RecoveryMessages OnCheckModemRecoveryTimeout(SMParam *smParam);
	static RecoveryMessages OnCheckMaxCyclesExceeded(SMParam *smParam);
	static void OnEnterHWError(SMParam *smParam);
	static RecoveryMessages OnHWError(SMParam *smParam);
	static RecoveryMessages DecideRecoveryPath(RecoveryMessages message, SMParam *smParam);
	static RecoveryMessages UpdateRecoveryState(RecoveryMessages message, SMParam *smParam);
	static void OnEnterPeriodicRestart(SMParam *smParam);
	static RecoveryMessages OnPeriodicRestart(SMParam *smParam);
	static RecoveryMessages OnCheckPeriodicRestartTimeout(SMParam *smParam);
	static RecoveryMessages DecideUponPeriodicRestartTimeout(RecoveryMessages message, SMParam *smParam);
	void RaiseRecoveryStateChanged(RecoveryTypes recoveryType, RecoverySource recoverySource);
	static void AppConfigChanged(const AppConfigChangedParam &param, const void *context);
	static void RecoveryControlTask(void *param);
	static bool isPeriodicRestartEnabled();
	static time_t calcNextPeriodicRestart();
};

extern RecoveryControl recoveryControl;

#endif // RecoveryControl_h