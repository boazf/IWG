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
    HTTPServer::AddController(IndexView::getPath(), IndexView::getInstance);
    HTTPServer::AddController(SettingsView::getPath(), SettingsView::getInstance);
    HTTPServer::AddController(DummyView::getPath(), DummyView::getInstance);
    HTTPServer::AddController(DefaultView::getPath(), DefaultView::getInstance);
    HTTPServer::AddController(HistoryView::getPath(), HistoryView::getInstance);
    HTTPServer::AddController(FilesView::getPath(), FilesView::getInstance);
    HTTPServer::AddController(SSEController::getPath(), SSEController::getInstance);
    HTTPServer::AddController(FilesController::getPath(), FilesController::getInstance);
    HTTPServer::AddController(RecoveryController::getPath(), RecoveryController::getInstance);
    HTTPServer::AddController(SystemController::getPath(), SystemController::getInstance);
}
