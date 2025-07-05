#include <unity.h>
#include <Arduino.h>
#include "StateMachineTests.h"
#include "StringableEnumTests.h"
#include "HtmlFillerViewReaderTests.h"
#include "FakeLock.h"
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
  return UNITY_END();
}
