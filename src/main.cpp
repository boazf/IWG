#include <Arduino.h>
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
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  InitSD();
  InitConfig();
  InitAppConfig();
  InitEthernet();
  InitTime();
  InitSFT();
  InitViews();
  InitRelays();
  InitControllers();
  InitHTTPServer();
}

void loop() {
  MaintainEthernet();
  DoSFTService();
  DoHTTPService();
  recoveryControl.PerformCycle();
  historyControl.PerformCycle();
  //sseController.Maintain();  
}
