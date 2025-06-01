#include <HTTPServer.h>
#include <SSEController.h>
#include <RecoveryController.h>
#include <HistoryControl.h>
#include <FilesController.h>
#include <ManualControl.h>
#include <SystemController.h>

void InitControllers()
{
    manualControl.init();
    historyControl.init();
    recoveryControl.Init();
    sseController.Init();
}

void PerformControllersCycles()
{
}