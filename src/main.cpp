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

void setup() {
  InitSerialTrace();
  InitSD();
  InitConfig();
  InitAppConfig();
  InitEthernet();
  InitTime();
  InitFileTrace();
  InitSFT();
  InitRelays();
  InitControllers();
  InitViews();
  InitHTTPServer();
}

void loop() {
  MaintainEthernet();
  DoSFTService();
  DoHTTPService();
  PerformControllersCycles();
  delay(1);
}

CriticalSection csSpi;
