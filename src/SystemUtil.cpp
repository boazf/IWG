#include "SystemUtil.h"
#include "AppConfig.h"
#include "HistoryControl.h"

void factoryReset()
{
    // Implementation of the factory reset logic goes here.
    // This should reset the system to its default state, erasing all configurations and settings.
    AppConfig::factoryReset();
    historyControl.reset();
    Serial.println("Factory reset performed.");
}

