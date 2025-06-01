#ifndef ManualControl_h
#define ManualControl_h

#include <CommonFsmDefs.h>
#include <RecoveryControl.h>
#include <Buttons.h>

namespace manualcontrol
{
    struct ButtonsStateChanged : tinyfsm::Event {};

    class ManualControl : public tinyfsm::Fsm<ManualControl>
    {
    public:
        /* default reaction for unhandled events */
        void react(tinyfsm::Event const &) { };

        virtual void react(RecoveryStateChanged const &) {};
        virtual void react(ButtonsStateChanged const &) {};

        virtual void entry(void) { };  /* entry actions in some states */
        virtual void exit(void)  { };  /* entry actions in some states */

    public:
        void init()
        {
            tinyfsm::FsmList<ManualControl>::start();
            recoveryControl.GetRecoveryStateChanged().addObserver(onRecoveryStateChanged, this);
            buttons.stateChanged.addObserver(onButtonsStateChanged, this);
        }

    protected:
        template<typename E>
        void send_event(E const &event)
        {
            tinyfsm::FsmList<ManualControl>::template dispatch<E>(event);
        }

    private:
        static void onRecoveryStateChanged(const RecoveryStateChangedParams &state, const void *context)
        {
            const_cast<ManualControl *>(reinterpret_cast<const ManualControl *>(context))->send_event(RecoveryStateChanged(state.m_recoveryType, state.m_source));
        }

        static void onButtonsStateChanged(const ButtonStateChangedParam &param, const void *context)
        {
            const_cast<ManualControl *>(reinterpret_cast<const ManualControl *>(context))->send_event(ButtonsStateChanged());
        }
    };
}

extern manualcontrol::ManualControl manualControl;

#endif // ManualControl_h