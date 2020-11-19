#include <HTTPServer.h>
#include <ViewUtil.h>
#include <IndexView.h>
#include <SettingsView.h>
#include <DummyView.h>
#include <DefaultView.h>
#include <HistoryView.h>

void InitViews()
{
    HTTPServer::AddView(&indexView);
    HTTPServer::AddView(&settingsView);
    HTTPServer::AddView(&dummyView);
    HTTPServer::AddView(&defaultView);
    HTTPServer::AddView(&historyView);
}

DummyView dummyView("/DUMMY", "");
DefaultView defaultView("/", "");
