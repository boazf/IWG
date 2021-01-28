#include <HTTPServer.h>
#include <SSEController.h>
#include <RecoveryController.h>
#include <HistoryControl.h>
#include <FilesController.h>

void InitControllers()
{
    historyControl.Init();
    recoveryControl.Init();
    sseController.Init();
    HTTPServer::AddController(&sseController);
    HTTPServer::AddController(&recoveryController);
#ifdef ESP32
    HTTPServer::AddController(&filesController);
#endif
}