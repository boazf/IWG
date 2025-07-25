#include <unity.h>
#include <Arduino.h>
#include "StateMachineTests.h"
#include "StringableEnumTests.h"
#include "HtmlFillerViewReaderTests.h"
#include "HistoryStorageTests.h"
#include <HistoryControlTests.h>
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
	RUN_TEST(stateMachineBasicTransitionsTests);
	RUN_TEST(stateMachIneInvalidVerbTransitionTests);
	RUN_TEST(stateMachineInvalidStateTransitionsTests);
	RUN_TEST(stateMachineNoEntryExitActionsTests);
	RUN_TEST(stateMachineUnvalidStatesTests);
	RUN_TEST(stateMachineChangeVerbAtExitTests);
	RUN_TEST(stringableEnumTests);
	RUN_TEST(stringableEnumInvalidValueTests);
	RUN_TEST(htmlFillerViewReaderBasicTests);
	RUN_TEST(htmlFillerViewReaderWithVariousBuffLenTests);
	RUN_TEST(htmlFillerViewReaderWithNonExistingFillerTests);
	RUN_TEST(htmlFillerViewReaderWithNotEnoughSpaceForFillerTests);
	RUN_TEST(historyStorageBasicTests);
	RUN_TEST(historyStorageBasicResizeTests);
	RUN_TEST(historyStorageInitTests);
	RUN_TEST(historyStorageShrinkTests);
	RUN_TEST(historyStorageEnlargeTests);
	RUN_TEST(historyStorageLastRecoveryTests);
	RUN_TEST(historyStorageModemAndRouterRecoveryCountsTests);
	RUN_TEST(historyControlBasicTests);
	RUN_TEST(historyControlResizeTests);
	RUN_TEST(historyControlRouterRecoveryTests);
	RUN_TEST(historyControlModemRecoveryTests);
	RUN_TEST(historyControlPeriodicRestartTests);
	RUN_TEST(historyControlConnectivityCheckWhileInFailureTests);
  return UNITY_END();
}

/// @brief Global instance of the EEPROMClassEx.
EEPROMClassEx EEPROMEx;