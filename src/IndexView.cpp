#include <Common.h>
#include <IndexView.h>
#include <AppConfig.h>
#include <RecoveryControl.h>
#include <HistoryControl.h>
#include <SSEController.h>
#include <TimeUtil.h>
#include <Config.h>
#include <HttpHeaders.h>
#include <HttpHeaders.h>

using namespace historycontrol;

extern const uint8_t _binary_sd_wwwroot_index_htm_start[];
extern const uint8_t _binary_sd_wwwroot_index_htm_end[];
  
IndexView::IndexView() : 
   HtmlFillerView(
    _binary_sd_wwwroot_index_htm_start, 
    _binary_sd_wwwroot_index_htm_end - _binary_sd_wwwroot_index_htm_start, 
    CONTENT_TYPE::HTML, 
    getFillers)
{
}

ViewFiller IndexView::fillers[] = 
{
    /*  0 */ [](String &fill) { fill = AppConfig::getAutoRecovery() ? "none" : "block"; fill += "\""; },
    /*  1 */ [](String &fill) { fill = GetRouterPowerState() == PowerState::POWER_ON ? "checked" : ""; },
    /*  2 */ [](String &fill) { fill = GetModemPowerState() == PowerState::POWER_ON ? "checked" : ""; },
    /*  3 */ [](String &fill) { fill = recoveryControl.GetRecoveryState() != RecoveryTypes::NoRecovery ? "visible" : "hidden"; fill += "'"; },
    /*  4 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", (t_now - historyControl.getLastRecovery()) / 3600 / 24); fill = buff;  },
    /*  5 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", ((t_now - historyControl.getLastRecovery()) / 3600) % 24); fill = buff;  },
    /*  6 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", ((t_now - historyControl.getLastRecovery()) / 60) % 60); fill = buff;  },
    /*  7 */ [](String &fill) { char buff[8]; sprintf(buff, "%ld", (t_now - historyControl.getLastRecovery()) % 60); fill = buff; },
    /*  8 */ [](String &fill) { fill = historyControl.getLastRecovery() == 0 ? "false" : "true"; },
    /*  9 */ [](String &fill) { fill = (int)recoveryControl.GetRecoveryState(); },
    /* 10 */ [](String &fill) { fill = appBase(); },
    /* 11 */ [](String &fill) { fill = Config::deviceName; },
    /* 12 */ [](String &fill) { fill = Config::singleDevice ? "none" : "visible"; }
};

int IndexView::getFillers(const ViewFiller *&_fillers)
{
    _fillers = fillers;
    return NELEMS(fillers);
}

std::atomic<int> IndexView::id(0);

bool IndexView::redirect(EthClient &client, const String &_id)
{
    if (_id.equals("") || !sseController.IsValidId(_id))
    {
        HttpHeaders::Header additionalHeaders[] = {{"Location", String("/index/") + ++id}};
        HttpHeaders headers(client);
        headers.sendHeaderSection(302, true, additionalHeaders, NELEMS(additionalHeaders));

        sseController.AddClient(String(id));

        return true;
    }

    return false;
}
