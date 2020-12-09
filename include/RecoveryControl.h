#ifndef RecoveryControl_h
#define RecoveryControl_h

#include <StateMachine.h>
#include <Observers.h>
#include <time.h>
#include <Relays.h>
#include <AppConfig.h>

enum Message
{
	None,
	M_Connected,
	M_Disconnected,
	M_Done,
	M_Timeout,
	M_NoTimeout,
	M_Exceeded,
	M_NotExceeded,
	M_DisconnectRouter,
	M_DisconnectModem,
	M_CheckConnectivity,
	M_HWError
};

enum State
{
	CheckConnectivity,
	StartCheckConnectivity,
	WaitWhileConnected,
	DisconnectModem,
	WaitAfterModemRecovery,
	CheckConnectivityAfterModemRecovery,
	CheckModemRecoveryTimeout,
	CheckMaxCyclesExceeded,
	CheckConnectivityAfterRecoveryFailure,
	WaitWhileRecoveryFailure,
	DisconnectRouter,
	CheckConnectivityAfterRouterRecovery,
	WaitAfterRouterRecovery,
	CheckRouterRecoveryTimeout,
	HWError
};

enum RecoveryTypes
{
	NoRecovery,
	Router,
	Modem,
	ConnectivityCheck,
	Failed,
	HWFailure,
	Disconnected
};


class RecoveryStateChangedParams
{
public:
	RecoveryStateChangedParams(RecoveryTypes recoveryType, bool byUser)
	{
		m_recoveryType = recoveryType;
		m_byUser = byUser;
	}

	RecoveryTypes m_recoveryType;
	bool m_byUser;
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
	SMParam(RecoveryControl *recoveryControl, time_t _lastRecovery, bool byUser = false) :
		m_recoveryControl(recoveryControl),
		m_byUser(byUser),
		lastRecovery(_lastRecovery),
		lanConnected(false),
		lastRecoveryType(RecoveryTypes::NoRecovery),
		requestedRecovery(Message::M_Done),
		updateConnState(false),
		cycles(0),
		autoRecovery(AppConfig::getAutoRecovery()),
		maxHistory(AppConfig::getMaxHistory()),
		stateParam(NULL)
	{
	}

	RecoveryControl *m_recoveryControl;
	bool m_byUser;
	time_t lastRecovery;
	bool lanConnected;
	RecoveryTypes lastRecoveryType;
	Message requestedRecovery;
	bool updateConnState;
	time_t recoveryStart;
	time_t t0;
	int cycles;
	bool autoRecovery;
	int maxHistory;
	void *stateParam;
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
	StateMachine<Message, State> *m_pSM;
	SMParam *m_param;
	Observers<RecoveryStateChangedParams> m_recoveryStateChanged;
	Observers<PowerStateChangedParams> m_modemPowerStateChanged;
	Observers<PowerStateChangedParams> m_routerPowerStateChanged;
	Observers<AutoRecoveryStateChangedParams> m_autoRecoveryStateChanged;
	Observers<MaxHistoryRecordChangedParams> m_maxHistoryRecordsChanged;
	RecoveryTypes m_currentRecoveryState;

private:
	static void OnEnterCheckConnectivity(void *param);
	static Message OnCheckConnectivity(void *param);
	static void OnEnterWaitConnectionPeriod(void *param);
	static Message OnWaitConnectionTestPeriod(void *param);
	static Message OnStartCheckConnectivity(void *param);
	static void OnEnterDisconnectRouter(void *param);
	static Message OnDisconnectRouter(void *param);
	static void OnEnterWaitWhileRecovering(void *param);
	static Message OnWaitWhileRecovering(void *param);
	static Message OnCheckRouterRecoveryTimeout(void *param);
	static void OnEnterDisconnectModem(void *param);
	static Message OnDisconnectModem(void *param);
	static Message OnCheckModemRecoveryTimeout(void *param);
	static Message OnCheckMaxCyclesExceeded(void *param);
	static void OnEnterHWError(void *param);
	static Message OnHWError(void *param);
	static Message DecideRecoveryPath(Message message, void *param);
	static Message UpdateRecoveryState(Message message, void *param);
	void RaiseRecoveryStateChanged(RecoveryTypes recoveryType, bool byUser);
	static void AppConfigChanged(const AppConfigChangedParam &param, const void *context);
};

extern RecoveryControl recoveryControl;

#endif // RecoveryControl_h