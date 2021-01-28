#include <HTTPServer.h>
#include <ViewUtil.h>
#include <IndexView.h>
#include <SettingsView.h>
#include <DummyView.h>
#include <DefaultView.h>
#include <HistoryView.h>
#include <Filesview.h>

void InitViews()
{
    HTTPServer::AddView(&indexView);
    HTTPServer::AddView(&settingsView);
    HTTPServer::AddView(&dummyView);
    HTTPServer::AddView(&defaultView);
    HTTPServer::AddView(&historyView);
#ifdef ESP32
    HTTPServer::AddView(&filesView);
#endif
}

DummyView dummyView("/DUMMY", "");
DefaultView defaultView("/", "");
