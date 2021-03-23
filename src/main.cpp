#include <Arduino.h>
#include <Trace.h>
#include <Config.h>
#include <TimeUtil.h>
#include <SDUtil.h>
#include <EthernetUtil.h>
#include <SFT.h>
#include <HTTPServer.h>
#include <RecoveryControl.h>
#include <NTPClient.h>
#include <ViewUtil.h>
#include <ControllerUtil.h>
#include <SSEController.h>
#include <HistoryControl.h>
#include <Relays.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
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
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  MaintainEthernet();
  DoSFTService();
  DoHTTPService();
  recoveryControl.PerformCycle();
  historyControl.PerformCycle();
  delay(1);
}

CriticalSection csSpi;
