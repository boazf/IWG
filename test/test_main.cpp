#include <unity.h>
#include <Arduino.h>
#include "StateMachineTests.h"
#include "StringableEnumTests.h"
#include "HtmlFillerViewReaderTests.h"
#include "HistoryStorageTests.h"
#include "FakeLock.h"
#include <FakeEEPROMEx.h>
#include <Trace.h>

void setUp(void) {
  Traceln("Setting up test environment...\n");
}

void tearDown(void) {
  Traceln("Tearing down test environment...\n");
}

int main(void) {
  Traceln("Starting tests...");
  UNITY_BEGIN();
  test_StateMachine();
  test_StringableEnum();
  test_HtmlFillerViewReader();
  test_HistoryStorage();
  return UNITY_END();
}

/// @brief Global instance of the EEPROMClassEx.
EEPROMClassEx EEPROMEx;