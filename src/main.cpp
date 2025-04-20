#include <Arduino.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <HTTPServer.h>
#include <NTPClient.h>
#include <HttpContollersUtil.h>
#include <ControllerUtil.h>
#include <SSEController.h>
#include <Relays.h>
#include <esp_task_wdt.h>
#include <Indicators.h>
#include <GWConnTest.h>
#include <PwrCntl.h>

void initProgress(bool last = false)
{
  static int state = 0;
  static Indicator indicators[] = { opi, rri, uli, mri };

  if (state > 0)
    indicators[state - 1].set(ledState::LED_ON);

  if (!last && state < NELEMS(indicators))
  {
    indicators[state++].set(ledState::LED_BLINK);
  }
}

void setup() {
  initProgress();
  esp_task_wdt_init(30, true);
  InitSerialTrace();
  InitPowerControl();
  InitSD();
  InitConfig();
  InitAppConfig();
  InitRelays();
#ifndef USE_WIFI
  // Wait for router initialization time.
  // Some routers dors not function properly soon after startup.
  delay(Config::routerInitTimeSec * 1000);
#endif
  initProgress();
  InitEthernet();
  initProgress();
  WaitForDNS();
  initProgress();
  InitTime();
  initProgress();
  InitFileTrace();
  InitControllers();
  InitHttpControllers();
  InitHTTPServer();
  initProgress(true);
}

void loop() 
{
  MaintainEthernet();
  if (gwConnTest.IsConnected())
    DoHTTPService();
  PerformControllersCycles();
  delay(1);
}

CriticalSection csSpi;
