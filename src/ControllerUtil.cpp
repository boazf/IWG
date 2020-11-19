#include <HTTPServer.h>
#include <SSEController.h>
#include <RecoveryController.h>
#include <HistoryControl.h>

void InitControllers()
{
    historyControl.Init();
    recoveryControl.Init();
    sseController.Init();
    HTTPServer::AddController(&sseController);
    HTTPServer::AddController(&recoveryController);
}