#include <ManualControl.h>
#include <Indicators.h>
#include <AppConfig.h>
#include <PwrCntl.h>

namespace manualcontrol
{
    class ConnectivityCheck; 
    class Disconnected;
    class RecoveryFailure;
    class HWFailure;
    class ModemRecovery;
    class Connected;
    class RouterRecovery;
    class PeriodicRestart;
    class Unlock;

    class CommonManualControlState : public ManualControl
    {
    public:
        CommonManualControlState(bool isRecovery = false) : isRecovery(isRecovery) {}

        void entry() override
        {
            if (!isRecovery)
            {
                opi.set(ledState::LED_IDLE);
                uli.set(ledState::LED_IDLE);
                mri.set(ledState::LED_IDLE);
                rri.set(ledState::LED_IDLE);
            }
            else
            {
                opi.set(ledState::LED_OFF);
                uli.set(ledState::LED_OFF);
                mri.set(ledState::LED_OFF);
                rri.set(ledState::LED_OFF);
            }
        }

        void react(RecoveryStateChanged const &event) override
        {
            switch(event.m_recoveryType)
            {
            case RecoveryTypes::ConnectivityCheck:
                transit<ConnectivityCheck>();

            case RecoveryTypes::Disconnected:
                transit<Disconnected>();
                break;

            case RecoveryTypes::Failed:
                transit<RecoveryFailure>();
                break;

            case RecoveryTypes::HWFailure:
                transit<HWFailure>();
                break;

            case RecoveryTypes::Modem:
                transit<ModemRecovery>();
                break;

            case RecoveryTypes::NoRecovery:
                transit<Connected>();
                break;

            case RecoveryTypes::Router:
            case RecoveryTypes::RouterSingleDevice:
                transit<RouterRecovery>();
                break;

            case RecoveryTypes::Periodic:
                transit<PeriodicRestart>();
                break;
            }
        }

        void doButtons(bool isConnected)
        {
            CommonManualControlState::isConnected = isConnected;
            
            if (ul.state() == ButtonState::PRESSED && rr.state() == ButtonState::UNPRESSED && mr.state() == ButtonState::UNPRESSED && cc.state() == ButtonState::UNPRESSED)
            {
                transit<Unlock>();
            }

            if (cc.state() == ButtonState::PRESSED)
            {
                recoveryControl.StartRecoveryCycles(RecoveryTypes::ConnectivityCheck);
            }
        }

    protected:
        bool isRecovery;
        static bool isConnected;
    };

    bool CommonManualControlState::isConnected;

    class Connected : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_ON);
        }

        void react(ButtonsStateChanged const &event) override
        {
            doButtons(true);
        }
    };

    class PeriodicRestart : public CommonManualControlState
    {
    public:
        PeriodicRestart() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            if (AppConfig::getPeriodicallyRestartRouter())
                rri.set(ledState::LED_BLINK);
            if (AppConfig::getPeriodicallyRestartModem())
                mri.set(ledState::LED_BLINK);
        }
    };

    class RouterRecovery : public CommonManualControlState
    {
    public:
        RouterRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            rri.set(ledState::LED_BLINK);
        }
    };

    class ModemRecovery : public CommonManualControlState
    {
    public:
        ModemRecovery() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            mri.set(ledState::LED_BLINK);
        }
    };

    class HWFailure : public CommonManualControlState
    {
    public:
        HWFailure() : CommonManualControlState(true) {}
        
        void entry() override
        {
            CommonManualControlState::entry();
            mri.set(ledState::LED_BLINK);
            rri.set(ledState::LED_BLINK);
        }
    };

    class RecoveryFailure : public CommonManualControlState
    {
    public:
        RecoveryFailure() : CommonManualControlState(true) {}

        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_OFF);
        }
    };

    class Disconnected : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_OFF);
        }
    
        void react(ButtonsStateChanged const &event) override
        {
            doButtons(false);
        }
};

    class ConnectivityCheck : public CommonManualControlState
    {
    public:
        void entry() override
        {
            CommonManualControlState::entry();
            opi.set(ledState::LED_BLINK);
        }
    };

    class Unlock : public CommonManualControlState
    {
    public:
        struct UnlockDelay : tinyfsm::Event {};

    public:
        void entry() override
        {
            unlocked = false;
            ledState opiState = opi.get();
            CommonManualControlState::entry();
            opi.set(opiState);
            uli.set(ledState::LED_OFF);
            esp_timer_create_args_t args = {onTimer, this, ESP_TIMER_TASK, "UnlockTimer"};
            esp_timer_create(&args, &hTimer);
            esp_timer_start_once(hTimer, 1000000);
        }

        void react(const UnlockDelay &event)
        {
            if (!unlocked)
            {
                unlocked = true;
                mri.set(ledState::LED_ON);
                rri.set(ledState::LED_ON);
                uli.set(ledState::LED_ON);
                esp_timer_start_once(hTimer, 3000000);
            }
            else
            {
                if (isConnected)
                    transit<Connected>();
                else
                    transit<Disconnected>();
            }
        }

        void react(ButtonsStateChanged const &event) override
        {
            if (!unlocked)
            {
                if (ul.state() == ButtonState::UNPRESSED || mr.state() == ButtonState::PRESSED || rr.state() == ButtonState::PRESSED)
                    transit<Connected>();
            }
            else
            {
                if (ul.state() == ButtonState::UNPRESSED && mr.state() == ButtonState::UNPRESSED && rr.state() == ButtonState::PRESSED)
                {
                    recoveryControl.StartRecoveryCycles(RecoveryTypes::Router);
                }
                else if (ul.state() == ButtonState::UNPRESSED && mr.state() == ButtonState::PRESSED && rr.state() == ButtonState::UNPRESSED)
                {
                    recoveryControl.StartRecoveryCycles(RecoveryTypes::Modem);        
                }
                else if (cc.state() == ButtonState::PRESSED)
                {
                    esp_timer_stop(hTimer);
                    opi.set(ledState::LED_BLINK);
                    uli.set(ledState::LED_OFF);
                    rri.set(ledState::LED_OFF);
                    mri.set(ledState::LED_OFF);
                    HardReset(3000);
                }
            }
        }

        void exit() override
        {
            esp_timer_stop(hTimer);
            esp_timer_delete(hTimer);
        }

    private:
        esp_timer_handle_t hTimer;
        bool unlocked;

    private:
        static void onTimer(void *arg)
        {
            reinterpret_cast<Unlock *>(arg)->react(UnlockDelay());
        }
    };

    class Init : public CommonManualControlState
    {
        void entry() override
        {
            CommonManualControlState::entry();
            if (recoveryControl.GetRecoveryState() != RecoveryTypes::ConnectivityCheck)
                transit<Connected>();
        }

        void react(RecoveryStateChanged const &event) override
        {
            if (event.m_recoveryType !=  RecoveryTypes::ConnectivityCheck)
                transit<Connected>();
        }
    };
}

manualcontrol::ManualControl manualControl;

FSM_INITIAL_STATE(manualcontrol::ManualControl, manualcontrol::Init)
