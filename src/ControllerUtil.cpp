#include <HTTPServer.h>
#include <SSEController.h>
#include <RecoveryController.h>
#include <HistoryControl.h>
#include <FilesController.h>
#include <ManualControl.h>
#include <SystemController.h>

void InitControllers()
{
    tinyfsm::Fsm<ManualControl>::current_state_ptr->init();
    historyControl.Init();
    recoveryControl.Init();
    sseController.Init();
}

void PerformControllersCycles()
{
    historyControl.PerformCycle();
}