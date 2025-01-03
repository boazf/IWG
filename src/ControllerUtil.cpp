#include <HTTPServer.h>
#include <SSEController.h>
#include <RecoveryController.h>
#include <HistoryControl.h>
#include <FilesController.h>
#include <ManualControl.h>
#include <VersionController.h>

void InitControllers()
{
    manualControl.Init();
    historyControl.Init();
    recoveryControl.Init();
    sseController.Init();
    HTTPServer::AddController(&sseController);
    HTTPServer::AddController(&recoveryController);
    HTTPServer::AddController(&filesController);
    HTTPServer::AddController(&versionController);
}

void PerformControllersCycles()
{
    manualControl.PerformCycle();
    historyControl.PerformCycle();
}