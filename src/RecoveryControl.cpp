#include <SPI.h>
#include <EthernetUtil.h>
#include <RecoveryControl.h>
#include <Observers.h>
#include <Common.h>
#include <Config.h>
#ifndef ESP32
#include <Dns.h>
#include <ICMPPing.h>
#else
#include <ping.h>
#endif
#include <AppConfig.h>
#include <TimeUtil.h>
#include <HistoryControl.h>
#include <EthernetUtil.h>

RecoveryControl::RecoveryControl() :
	m_currentRecoveryState(RecoveryTypes::NoRecovery)
{
}

void RecoveryControl::Init()
{
	Transition<Message, State> checkConnecticityTrans[] =
	{
		{ Message::M_Disconnected, State::WaitWhileRecoveryFailure },
		{ Message::M_Done, State::CheckConnectivity },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem }
	};

	Transition<Message, State> waitWhileConnectedTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivity },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem },
		{ Message::M_CheckConnectivity, State::StartCheckConnectivity }
	};

	Transition<Message, State> startCheckConnectivityTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivity }
	};

	Transition<Message, State> disconnectRouterTrans[] =
	{
		{ Message::M_Done, State::WaitAfterRouterRecovery },
		{ Message::M_HWError, State::HWError }
	};

	Transition<Message, State> waitAfterRouterRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRouterRecovery }
	};

	Transition<Message, State> checkConnectivityAfterRouterRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRouterRecovery },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_Disconnected, State::CheckRouterRecoveryTimeout }
	};

	Transition<Message, State> checkRouterRecoveryTimeoutTrans[] =
	{
		{ Message::M_Timeout, State::DisconnectModem },
		{ Message::M_NoTimeout, State::WaitAfterRouterRecovery }
	};

	Transition<Message, State> disconnectModemTrans[] =
	{
		{ Message::M_Done, State::WaitAfterModemRecovery},
		{ Message::M_HWError, State::HWError }
	};

	Transition<Message, State> waitAfterModemRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterModemRecovery }
	};

	Transition<Message, State> checkConnectivityAfterModemRecoveryTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterModemRecovery },
		{ Message::M_Connected, State::WaitWhileConnected},
		{ Message::M_Disconnected, State::CheckModemRecoveryTimeout }
	};

	Transition<Message, State> checkModemRecoveryTimeoutTrans[] =
	{
		{ Message::M_Timeout, State::CheckMaxCyclesExceeded },
		{ Message::M_NoTimeout, State::WaitAfterModemRecovery }
	};

	Transition<Message, State> checkMaxCyclesExceededTrans[] =
	{
		{ Message::M_Exceeded, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_NotExceeded, State::DisconnectRouter }
	};

	Transition<Message, State> checkConnectivityAfterRecoveryFailureTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_Connected, State::WaitWhileConnected },
		{ Message::M_Disconnected, State::WaitWhileRecoveryFailure }
	};

	Transition<Message, State> waitWhileRecoveryFailureTrans[] =
	{
		{ Message::M_Done, State::CheckConnectivityAfterRecoveryFailure },
		{ Message::M_DisconnectRouter, State::DisconnectRouter },
		{ Message::M_DisconnectModem, State::DisconnectModem },
		{ Message::M_CheckConnectivity, State::StartCheckConnectivity }
	};

	Transition<Message, State> hwErrorTrans[] =
	{
		{ Message::M_Done, State::StartCheckConnectivity }
	};

	SMState<Message, State> states[]
	{
		SMState<Message, State>(
			State::CheckConnectivity, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			DecideRecoveryPath, 
			checkConnecticityTrans, 
			NELEMS(checkConnecticityTrans)),
		SMState<Message, State>(
			State::WaitWhileConnected, 
			OnEnterWaitConnectionPeriod, 
			OnWaitConnectionTestPeriod, 
			SMState<Message, State>::OnExitDoNothing, 
			waitWhileConnectedTrans , 
			NELEMS(waitWhileConnectedTrans)),
		SMState<Message, State>(
			State::StartCheckConnectivity, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnStartCheckConnectivity, 
			SMState<Message, State>::OnExitDoNothing, 
			startCheckConnectivityTrans, 
			NELEMS(startCheckConnectivityTrans)),
		SMState<Message, State>(
			State::DisconnectRouter, 
			OnEnterDisconnectRouter, 
			OnDisconnectRouter, 
			SMState<Message, State>::OnExitDoNothing, 
			disconnectRouterTrans, 
			NELEMS(disconnectRouterTrans)),
		SMState<Message, State>(
			State::WaitAfterRouterRecovery, 
			OnEnterWaitWhileRecovering, 
			OnWaitWhileRecovering, 
			SMState<Message, State>::OnExitDoNothing, 
			waitAfterRouterRecoveryTrans, 
			NELEMS(waitAfterRouterRecoveryTrans)),
		SMState<Message, State>(
			State::CheckConnectivityAfterRouterRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			checkConnectivityAfterRouterRecoveryTrans, 
			NELEMS(checkConnectivityAfterRouterRecoveryTrans)),
		SMState<Message, State>(
			State::CheckRouterRecoveryTimeout, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckRouterRecoveryTimeout, 
			SMState<Message, State>::OnExitDoNothing, 
			checkRouterRecoveryTimeoutTrans, 
			NELEMS(checkRouterRecoveryTimeoutTrans)),
		SMState<Message, State>(
			State::DisconnectModem, 
			OnEnterDisconnectModem, 
			OnDisconnectModem, 
			SMState<Message, State>::OnExitDoNothing, 
			disconnectModemTrans, 
			NELEMS(disconnectModemTrans)),
		SMState<Message, State>(
			State::WaitAfterModemRecovery, 
			OnEnterWaitWhileRecovering, 
			OnWaitWhileRecovering, 
			SMState<Message, State>::OnExitDoNothing, 
			waitAfterModemRecoveryTrans, 
			NELEMS(waitAfterModemRecoveryTrans)),
		SMState<Message, State>(
			State::CheckConnectivityAfterModemRecovery, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			checkConnectivityAfterModemRecoveryTrans, 
			NELEMS(checkConnectivityAfterModemRecoveryTrans)),
		SMState<Message, State>(
			State::CheckModemRecoveryTimeout, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckModemRecoveryTimeout, 
			SMState<Message, State>::OnExitDoNothing, 
			checkModemRecoveryTimeoutTrans, 
			NELEMS(checkModemRecoveryTimeoutTrans)),
		SMState<Message, State>(
			State::CheckMaxCyclesExceeded, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnCheckMaxCyclesExceeded, 
			SMState<Message, State>::OnExitDoNothing, 
			checkMaxCyclesExceededTrans, 
			NELEMS(checkMaxCyclesExceededTrans)),
		SMState<Message, State>(
			State::CheckConnectivityAfterRecoveryFailure, 
			OnEnterCheckConnectivity, 
			OnCheckConnectivity, 
			UpdateRecoveryState, 
			checkConnectivityAfterRecoveryFailureTrans, 
			NELEMS(checkConnectivityAfterRecoveryFailureTrans)),
		SMState<Message, State>(
			State::WaitWhileRecoveryFailure, 
			OnEnterWaitConnectionPeriod, 
			OnWaitConnectionTestPeriod, 
			SMState<Message, State>::OnExitDoNothing, 
			waitWhileRecoveryFailureTrans,
			NELEMS(waitWhileRecoveryFailureTrans)),
		SMState<Message, State>(
			State::HWError, 
			SMState<Message, State>::OnEnterDoNothing, 
			OnHWError, 
			SMState<Message, State>::OnExitDoNothing, 
			hwErrorTrans, 
			NELEMS(hwErrorTrans))
	};

	m_param = new SMParam(this, historyControl.getLastRecovery(), false);
	m_pSM = new StateMachine<Message, State>(states, NELEMS(states), m_param);

	AppConfig::getAppConfigChanged().addObserver(AppConfigChanged, this);
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

enum CheckConnectivityStages
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
#ifndef ESP32
		ping(MAX_SOCK_NUM, 1),
#else
		attempts(0),
#endif
		status(Message::M_Disconnected),
		stage(CheckLAN)
	{
	}

public:
#ifndef ESP32
	ICMPPing ping;
	ICMPEchoReply pingResult;
#else
	IPAddress address;
	int attempts;
#endif
	Message status;
	CheckConnectivityStages stage;
};

static bool TryGetHostAddress(IPAddress &address, String server)
{
	if (server.equals(""))
		return false;
#ifdef ESP32
	if (WiFi.hostByName(server.c_str(), address) != 1)
#else
	DNSClient dns;
	dns.begin(Config::gateway);

	if (dns.getHostByName(server.c_str(), address) != 1)
#endif
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Trace("Failed to get host address for ");
		Traceln(server.c_str());
#endif
		return false;
	}

#ifdef DEBUG_RECOVERY_CONTROL
	Trace("About to ping ");
	Traceln(server.c_str());
#endif

	return true;
}

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

	if (stateParam->stage == CheckLAN)
	{
		address = AppConfig::getLANAddr();
		if (IsZeroIPAddress(address))
		{
			stateParam->stage = CheckServer1;
			smParam->lanConnected = true;
		}
	}

	String server;

	if (stateParam->stage == CheckServer1)
	{
		server = AppConfig::getServer1();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = CheckServer2;
	}

	if (stateParam->stage == CheckServer2)
	{
		server = AppConfig::getServer2();
		if (!TryGetHostAddress(address, server))
			stateParam->stage = ChecksCompleted;
	}	

	if (stateParam->stage != ChecksCompleted)
	{
#ifdef DEBUG_RECOVERY_CONTROL
		Trace("Pinging address: ");
		Trace(address[0]);
		Trace(".");
		Trace(address[1]);
		Trace(".");
		Trace(address[2]);
		Trace(".");
		Traceln(address[3]);
#endif
#ifndef ESP32
		stateParam->ping.asyncStart(address, MAX_PING_ATTEMPTS, stateParam->pingResult);
#else
		stateParam->address = address;
		stateParam->attempts = 0;
#endif
	}
}

Message RecoveryControl::OnCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	CheckConnectivityStateParam *stateParam = (CheckConnectivityStateParam *)smParam->stateParam;
	Message status = Message::M_Disconnected;

	if (stateParam->stage != ChecksCompleted)
	{
#ifndef ESP32
		if (!stateParam->ping.asyncComplete(stateParam->pingResult))
			return Message::None;

		status = stateParam->pingResult.status == SUCCESS ? Message::M_Connected : Message::M_Disconnected;
#else
		if (stateParam->attempts < MAX_PING_ATTEMPTS)
		{
			if (ping_start(stateParam->address, 1, 0, 0, 1000))
				status = Message::M_Connected;
			else
			{
				stateParam->attempts++;
#ifdef DEBUG_RECOVERY_CONTROL
				Tracef("Ping attempt no. %d failed, address %s\n", stateParam->attempts, stateParam->address.toString().c_str());
#endif
				return Message::None;
			}
		}
#endif
#ifdef DEBUG_RECOVERY_CONTROL
		Trace("Ping result: ");
#ifndef ESP32
		Traceln((unsigned int)stateParam->pingResult.status);
#else
		Traceln(status == Message::M_Connected ? "OK" : "Failed");
#endif
#endif
	}

	stateParam->status = status;

	switch (stateParam->stage)
	{
	case CheckLAN:
		smParam->lanConnected = status == Message::M_Connected;
		if (smParam->lanConnected)
		{
			stateParam->stage = CheckServer1;
			status = Message::M_Done;
		}
		else
		{
			stateParam->stage = ChecksCompleted;
		}
		break;
	case CheckServer1:
		if (status == Message::M_Connected)
		{
			stateParam->stage = ChecksCompleted;
		}
		else
		{
			stateParam->stage = CheckServer2;
			status = Message::M_Done;
		}
		break;
	case CheckServer2:
		stateParam->stage = ChecksCompleted;
		break;
	case ChecksCompleted:
		break;
	}

	if (stateParam->stage == ChecksCompleted)
	{
		delete stateParam;
		smParam->stateParam = NULL;
	}

    return  status; 
}

Message RecoveryControl::UpdateRecoveryState(Message message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message == Message::M_Done)
		return message;

	if (message == Message::M_Connected)
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

Message RecoveryControl::DecideRecoveryPath(Message message, void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (message != Message::M_Done)
	{
		if (message != Message::M_Connected)
		{
			smParam->m_byUser = false;
            if (!AppConfig::getAutoRecovery() && !smParam->updateConnState)
            {
                smParam->lastRecovery = INT32_MAX;
                smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Disconnected, param);
                return message;
            }

			if (!smParam->lanConnected || smParam->lastRecovery == INT32_MAX || t_now - smParam->lastRecovery > 3600)
				message = Message::M_DisconnectRouter;
			else
				message = smParam->lastRecoveryType == RecoveryTypes::Router ? Message::M_DisconnectModem : Message::M_DisconnectRouter;
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

void RecoveryControl::OnEnterWaitConnectionPeriod(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->t0 = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Trace(__func__);
	Trace(": Waiting for ");
	Trace(AppConfig::getConnectionTestPeriod());
	Traceln(" Sec.");
#endif
}

Message RecoveryControl::OnWaitConnectionTestPeriod(void *param)
{
	SMParam *smParam = (SMParam *)param;
	Message requestedRecovery = Message::M_Done;
	if (smParam->requestedRecovery != Message::M_Done)
	{
		requestedRecovery = smParam->requestedRecovery;
		smParam->requestedRecovery = Message::M_Done;
		smParam->m_byUser = true;

		return requestedRecovery;
	}

	return t_now - smParam->t0 >= static_cast<time_t>(AppConfig::getConnectionTestPeriod()) ? Message::M_Done : Message::None;
}

Message RecoveryControl::OnStartCheckConnectivity(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::ConnectivityCheck, smParam->m_byUser);
	smParam->updateConnState = true;
	return Message::M_Done;
}

void RecoveryControl::OnEnterDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Router;
	smParam->lastRecovery = INT32_MAX;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Router, smParam->m_byUser);
	delay(500);
	smParam->recoveryStart = t_now;
	SetRouterPowerState(POWER_OFF);
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Router");
#endif
}

Message RecoveryControl::OnDisconnectRouter(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getRDisconnect()))
		return Message::None;

	SetRouterPowerState(POWER_ON);
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Router");
#endif
	return Message::M_Done;
}

void RecoveryControl::OnEnterWaitWhileRecovering(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->t0 = t_now;
}
Message RecoveryControl::OnWaitWhileRecovering(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->t0 >= 5 ? Message::M_Done : Message::None;
}

Message RecoveryControl::OnCheckRouterRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getRReconnect()) ? Message::M_Timeout : Message::M_NoTimeout;
}

void RecoveryControl::OnEnterDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->lastRecoveryType = RecoveryTypes::Modem;
	smParam->lastRecovery = INT32_MAX;
	SetModemPowerState(POWER_OFF);
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Modem, smParam->m_byUser);
	smParam->recoveryStart = t_now;
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Disconnecting Modem");
#endif
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(POWER_OFF));
}

Message RecoveryControl::OnDisconnectModem(void *param)
{
	SMParam *smParam = (SMParam *)param;

	if (t_now - smParam->recoveryStart < static_cast<time_t>(AppConfig::getMDisconnect()))
		return Message::None;

	SetModemPowerState(POWER_ON);
	smParam->m_recoveryControl->m_modemPowerStateChanged.callObservers(PowerStateChangedParams(POWER_ON));
#ifdef DEBUG_RECOVERY_CONTROL
	Traceln("Reconnecting Modem");
#endif
	return Message::M_Done;
}

Message RecoveryControl::OnCheckModemRecoveryTimeout(void *param)
{
	SMParam *smParam = (SMParam *)param;
	return t_now - smParam->recoveryStart > static_cast<time_t>(AppConfig::getMReconnect()) ? Message::M_Timeout : Message::M_NoTimeout;
}

Message RecoveryControl::OnCheckMaxCyclesExceeded(void *param)
{
	SMParam *smParam = (SMParam *)param;

	smParam->cycles++;
	if (!AppConfig::getLimitCycles() || smParam->cycles < AppConfig::getRecoveryCycles())
		return Message::M_NotExceeded;

	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::Failed, false);
	return Message::M_Exceeded;
}

void RecoveryControl::OnEnterHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	smParam->m_recoveryControl->RaiseRecoveryStateChanged(RecoveryTypes::HWFailure, false);
	smParam->t0 = time(NULL);
}

Message RecoveryControl::OnHWError(void *param)
{
	SMParam *smParam = (SMParam *)param;
	if (time(NULL) - smParam->t0 >= 10)
	    return Message::None;

    return Message::M_Done;
}

void RecoveryControl::StartRecoveryCycles(RecoveryTypes recoveryType)
{
	if (m_param->requestedRecovery != Message::M_Done)
		return;

	m_param->cycles = 0;
	switch(recoveryType)
	{
		case RecoveryTypes::Modem:
			m_param->requestedRecovery = Message::M_DisconnectModem;
			break;
		case RecoveryTypes::Router:
			m_param->requestedRecovery = Message::M_DisconnectRouter;
			break;
		case RecoveryTypes::ConnectivityCheck:
			m_param->requestedRecovery = Message::M_CheckConnectivity;
			break;
		default:
			break;
	}
}

RecoveryControl recoveryControl;