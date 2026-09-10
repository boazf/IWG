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
    /*  3 */ [](String &fill){ byte macAddress[6]; Eth.MACAddress(macAddress); char macStr[18]; sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]); fill = macStr; },
    /*  4 */ [](String &fill){ fill = String(t_now); },
};

int InfoView::getFillers(const ViewFiller *&_fillers)
{
    _fillers = fillers;
    return NELEMS(fillers);
}

