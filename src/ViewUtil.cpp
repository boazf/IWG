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
    HTTPServer::AddView(&indexViewCreator);
    HTTPServer::AddView(&settingsViewCreator);
    HTTPServer::AddView(&dummyViewCreator);
    HTTPServer::AddView(&defaultViewCreator);
    HTTPServer::AddView(&historyViewCreator);
    HTTPServer::AddView(&filesViewCreator);
}

DummyViewCreator dummyViewCreator("/DUMMY", "");
DefaultViewCreator defaultViewCreator("/", "");
