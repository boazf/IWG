#include "Common.h"
#include <InfoView.h>
#include "Version.h"
#include "EthernetUtil.h"
#include "TimeUtil.h"
#include "AppConfig.h"

InfoView::InfoView(const char *_viewFile) :
    HtmlFillerView(_viewFile, getFillers)
{
}

ViewFiller InfoView::fillers[] =
{
    /*  0 */ [](String &fill){ fill = Version::getCurrentVersion(); },
    /*  1 */ [](String &fill){ fill = Version::getBuild(); },
    /*  2 */ [](String &fill){ fill = Eth.localIP().toString(); },
    /*  3 */ [](String &fill)
    {
        byte macAddress[6]; 
#ifdef USE_WIFI
        WiFi.macAddress(macAddress); 
#else
        Eth.MACAddress(macAddress); 
#endif
        char macStr[18]; 
        sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]); 
        fill = macStr; 
    },
    /*  4 */ [](String &fill){ fill = String(t_now); },
    /* 5 */ [](String &fill)
    { 
        fill = 
#ifdef USE_WIFI
            String("date.toLocaleString(navigator.language, {") +
            "weekday: 'short'," +
            "day: '2-digit'," +
            "month: 'short'," +
            "year: 'numeric'," +
            "hour: '2-digit'," +
            "minute: '2-digit'," +
            "second: '2-digit'," +
            "hour12: false})";
#else
            "date.toUTCString().replace(\" GMT\", \"\")";
#endif
    }
};

int InfoView::getFillers(const ViewFiller *&_fillers)
{
    _fillers = fillers;
    return NELEMS(fillers);
}

