#ifndef HistoryControl_h
#define HistoryControl_h

#include <CommonFsmDefs.h>
#include <HistoryStorage.h>
namespace historycontrol
{
    class HistoryControl : public tinyfsm::Fsm<HistoryControl>
    {
    public:
        HistoryControl() {}

    public:
        /* default reaction for unhandled events */
        void react(tinyfsm::Event const &) { };

        virtual void react(RecoveryStateChanged const &) {};

        virtual void entry(void) { };  /* entry actions in some states */
        virtual void exit(void)  { };  /* entry actions in some states */

        void init();
        int Available();
        const HistoryStorageItem GetHistoryItem(int index);
        time_t getLastRecovery();
        time_t getLastUpdate();

    protected:
        static int maxHistory;
        static RecoveryTypes recoveryType;
        static RecoverySource recoverySource;
        static HistoryStorage storage;
        static HistoryStorageItem *currStorageItem;
        static time_t lastUpdate;
        
    protected:
        void AddToHistory();
        void AddHistoryItem(RecoverySource recoverySource);
        bool CreateHistoryItem(RecoverySource recoverySource);
        void AddToHistoryStorage(RecoveryStatus status, bool withEndTime = true);

    protected:
        template<typename E>
        void send_event(E const &event)
        {
            tinyfsm::FsmList<HistoryControl>::template dispatch<E>(event);
        }

    private:
        static void onRecoveryStateChanged(const RecoveryStateChangedParams &params, const void* context);
        static void onMaxHistoryChanged(const MaxHistoryRecordChangedParams &params, const void* context);
    };
}

extern historycontrol::HistoryControl historyControl;

#endif // HistoryControl_h