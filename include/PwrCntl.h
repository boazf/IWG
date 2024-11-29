#ifndef PwrCntl_h
#define PwrCntl_h
#include <Arduino.h>

void InitPowerControl();
void HardReset(int timeout = portMAX_DELAY);

#endif // PwrCntl_h