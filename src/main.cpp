#include <Arduino.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <SFT.h>
#include <HTTPServer.h>
#include <NTPClient.h>
#include <ViewUtil.h>
#include <ControllerUtil.h>
#include <SSEController.h>
#include <Relays.h>
#include <esp_task_wdt.h>
#include <Indicators.h>

void initProgress(bool last = false)
{
  static int state = 0;
  static Indicator indicators[] = { opi, mri, uli, rri };

  if (state > 0)
    indicators[state - 1].set(LED_ON);

  if (!last && state < NELEMS(indicators))
  {
    indicators[state++].set(LED_BLINK);
  }
}

void setup() {
  initProgress();
  esp_task_wdt_init(30, false);
  InitSerialTrace();
  InitSD();
  InitConfig();
  InitAppConfig();
  InitRelays();
#ifndef USE_WIFI
  // Wait for router initialization time.
  // Some routers dors not function properly soon after startup.
  while(millis() < Config::routerInitTimeSec * 1000);
#endif
  initProgress();
  InitEthernet();
  initProgress();
#ifndef USE_WIFI
  WaitForDNS();
  initProgress();
#endif
  InitTime();
  initProgress();
  InitFileTrace();
  InitSFT();
  InitControllers();
  InitViews();
  InitHTTPServer();
  initProgress(true);
}

void loop() {
  MaintainEthernet();
  DoSFTService();
  DoHTTPService();
  PerformControllersCycles();
  delay(1);
}

CriticalSection csSpi;
