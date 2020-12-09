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
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // Open serial communications and wait for port to open:
#ifndef ESP32
  Serial.begin(9600);
#else
  Serial.begin(115200);
#endif
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  InitSD();
  InitConfig();
  InitAppConfig();
  InitEthernet();
  InitTime();
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
  //sseController.Maintain();  
}
