#include <unity.h>
#include <Arduino.h>
#include "StateMachineTests.h"
#include "StringableEnumTests.h"
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
  RUN_TEST(sm_test_basic_transitions);
  RUN_TEST(sm_test_invalid_verb_transition);
  RUN_TEST(sm_test_invalid_state_transition);
  RUN_TEST(sm_test_no_entry_exit_actions);
  RUN_TEST(sm_test_invalid_states);
  RUN_TEST(sm_test_change_verb_at_exit);
  RUN_TEST(test_stringable_enum);
  RUN_TEST(test_stringable_enum_invalid);
  return UNITY_END();
}
