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
	RUN_TEST(sm_test_basic_transitions);
	RUN_TEST(sm_test_invalid_verb_transition);
	RUN_TEST(sm_test_basic_transitions);
	RUN_TEST(sm_test_invalid_verb_transition);
	RUN_TEST(sm_test_invalid_state_transition);
	RUN_TEST(sm_test_no_entry_exit_actions);
	RUN_TEST(sm_test_invalid_states);
	RUN_TEST(sm_test_change_verb_at_exit);
	RUN_TEST(test_stringable_enum);
	RUN_TEST(test_stringable_enum_invalid);
	RUN_TEST(HtmlFillerViewReaderBasicTests);
	RUN_TEST(HtmlFillerViewReaderWithVariousBuffLen);
	RUN_TEST(HtmlFillerViewReaderWithNonExistingFiller);
	RUN_TEST(HtmlFillerViewReaderWithNotEnoughSpaceForFiller);
	RUN_TEST(HistoryStorageBasicTests);
	RUN_TEST(BasicResizeHistoryStorageTests);
	RUN_TEST(InitHistoryStorageTests);
	RUN_TEST(TestShrinkHistoryStorage);
	RUN_TEST(EnlargeHistoryStorageTests);
	RUN_TEST(TestLastRecovery);
	RUN_TEST(TestModemAndRouterRecoveryCounts);
	RUN_TEST(HistoryControlBasicTests);
	RUN_TEST(HistoryControlResizeTests);
	RUN_TEST(HistoryControlRouterRecoveryTests);
	RUN_TEST(HistoryControlModemRecoveryTests);
	RUN_TEST(HistoryControlPeriodicRestartTests);
	RUN_TEST(HistoryControlConnectivityCheckWhileInFailureTests);
  return UNITY_END();
}

/// @brief Global instance of the EEPROMClassEx.
EEPROMClassEx EEPROMEx;