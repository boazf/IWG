#include <Common.h>
#include <IndexView.h>
#include <AppConfig.h>
#include <RecoveryControl.h>
#include <HistoryControl.h>
#include <SSEController.h>
#include <TimeUtil.h>
#include <Config.h>

IndexView::IndexView(const char *_viewName, const char *_viewFile) : 
   HtmlFillerView(_viewName, _viewFile)
{
}

ViewFiller IndexView::fillers[] = 
{
    /*  0 */ [](String &fill) { fill = AppConfig::getAutoRecovery() ? "none" : "block"; fill += "\""; },
    /*  1 */ [](String &fill) { fill = GetRouterPowerState() == POWER_ON ? "checked" : ""; },
    /*  2 */ [](String &fill) { fill = GetModemPowerState() == POWER_ON ? "checked" : ""; },
    /*  3 */ [](String &fill) { fill = recoveryControl.GetRecoveryState() != NoRecovery ? "visible" : "hidden"; fill += "'"; },
    /*  4 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", (t_now - historyControl.getLastRecovery()) / 3600 / 24); fill = buff;  },
    /*  5 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", ((t_now - historyControl.getLastRecovery()) / 3600) % 24); fill = buff;  },
    /*  6 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", ((t_now - historyControl.getLastRecovery()) / 60) % 60); fill = buff;  },
    /*  7 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", (t_now - historyControl.getLastRecovery()) % 60); fill = buff; },
    /*  8 */ [](String &fill) { fill = historyControl.getLastRecovery() == 0 ? "false" : "true"; },
    /*  9 */ [](String &fill) { fill = recoveryControl.GetRecoveryState(); },
    /* 10 */ [](String &fill) { IPAddress myIP = Eth.localIP(); fill = String("'http://") + myIP[0] + "." + myIP[1]+ "." + myIP[2] + "." + myIP[3] + "/'"; }
};

int IndexView::getFillers(const ViewFiller *&_fillers)
{
    _fillers = fillers;
    return NELEMS(fillers);
}

int IndexView::id = 0;

bool IndexView::redirect(EthClient &client, const String &_id)
{
    if (_id.equals("") || !sseController.IsValidId(_id))
    {
        client.println("HTTP/1.1 302 Found");
        client.print("Location: /index/");
        client.println(++id);
        client.println("Content-Length: 0");
        client.println("Connection: close");
        client.println();
#ifdef USE_WIFI
        client.flush();
#endif
        sseController.AddClient(String(id));
        return true;
    }

    return false;
}

IndexView indexView("/INDEX", "/INDEX.HTM");