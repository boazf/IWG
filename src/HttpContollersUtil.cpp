#include <HTTPServer.h>
#include <HttpContollersUtil.h>
#include <IndexView.h>
#include <SettingsView.h>
#include <DummyView.h>
#include <DefaultView.h>
#include <HistoryView.h>
#include <Filesview.h>
#include <SSEController.h>
#include <FilesController.h>
#include <RecoveryController.h>
#include <SystemController.h>

void InitHttpControllers()
{
    HTTPServer::AddController("/INDEX", IndexView::getInstance);
    HTTPServer::AddController("/SETTINGS", SettingsView::getInstance);
    HTTPServer::AddController("/DUMMY", DummyView::getInstance);
    HTTPServer::AddController("/", DefaultView::getInstance);
    HTTPServer::AddController("/HISTORY", HistoryView::getInstance);
    HTTPServer::AddController("/FILES", FilesView::getInstance);
    HTTPServer::AddController("/API/SSE", SSEController::getInstance);
    HTTPServer::AddController("/API/FILES", FilesController::getInstance);
    HTTPServer::AddController("/API/RECOVERY", RecoveryController::getInstance);
    HTTPServer::AddController("/API/SYSTEM", SystemController::getInstance);
}
