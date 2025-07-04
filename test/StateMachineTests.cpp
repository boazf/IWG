#include <unity.h>
#include "FakeLock.h"
#include "StateMachine.h"
#include "StateMachineTests.h"

#define VERBS \
    X(NONE) \
    X(GO) \
    X(STOP) \
    X(PAUSE)

#define X(a) a,
enum class Verb 
{ 
    VERBS
};

#define STATES \
    X(INIT) \
    X(RUNNING) \
    X(STOPPED) \
    X(IDLE)

enum class State
{ 
    STATES
};
#undef X

struct Context {
    Context() { init(); }
    int initEntryCount;
    int initExitCount;
    int initStateCount;
    int runningEntryCount;
    int runningExitCount;
    int runningStateCount;
    int stoppedEntryCount;
    int stoppedExitCount;
    int stoppedStateCount;
    int idleEntryCount;
    int idleExitCount;
    int idleStateCount;
    Verb nextVerb = Verb::NONE;

    void init() 
    {
        initEntryCount = 0;
        initExitCount = 0;
        initStateCount = 0;
        runningEntryCount = 0;
        runningExitCount = 0;
        runningStateCount = 0;
        stoppedEntryCount = 0;
        stoppedExitCount = 0;
        stoppedStateCount = 0;
        idleEntryCount = 0;
        idleExitCount = 0;
        idleStateCount = 0;
        nextVerb = Verb::NONE;
    }
};

#define X(a) {Verb::a, #a},
template<>
const std::map<Verb, std::string> StringableEnum<Verb>::strMap = 
{
    VERBS
};
#undef X

#define X(a) {State::a, #a},
template<>
const std::map<State, std::string> StringableEnum<State>::strMap = 
{
    STATES
};
#undef X

// Callback functions
void OnInitEntry(Context *ctx) {
    Traceln(__func__);
    ctx->initEntryCount++;
}

Verb OnInitState(Context *ctx) {
    Traceln(__func__);
    ctx->initStateCount++;
    return ctx->nextVerb;
}

Verb OnInitExit(Verb verb, Context *ctx) {
    Traceln(__func__);
    ctx->initExitCount++;
    return verb;
}

void OnRunningEntry(Context *ctx) {
    Traceln(__func__);
    ctx->runningEntryCount++;
}

Verb OnRunningState(Context *ctx) {
    Traceln(__func__);
    ctx->runningStateCount++;
    return ctx->nextVerb;
}

Verb OnRunningExit(Verb verb, Context *ctx) {
    Traceln(__func__);
    ctx->runningExitCount++;
    return verb;
}

void OnStoppedEntry(Context *ctx) {
    Traceln(__func__);
    ctx->stoppedEntryCount++;
}

Verb OnStoppedState(Context *ctx) {
    Traceln(__func__);
    ctx->stoppedStateCount++;
    return ctx->nextVerb;
}

Verb OnStoppedExit(Verb verb, Context *ctx) {
    Traceln(__func__);
    ctx->stoppedExitCount++;
    return verb;
}

void OnIdleEntry(Context *ctx) {
    Traceln(__func__);
    ctx->idleEntryCount++;
}

Verb OnIdleState(Context *ctx) {
    Traceln(__func__);
    ctx->idleStateCount++;
    return ctx->nextVerb;
}

Verb OnIdleExit(Verb verb, Context *ctx) {
    Traceln(__func__);
    ctx->idleExitCount++;
    return verb;
}

// Define transitions
Transition<Verb, State> transitionsInit[] = {
    { Verb::GO, State::RUNNING }
};

Transition<Verb, State> transitionsRunning[] = {
    { Verb::STOP, State::STOPPED },
    { Verb::GO, State::RUNNING },
    { Verb::PAUSE, State::IDLE }
};

Transition<Verb, State> transitionsStopped[] = {
    { Verb::GO, State::RUNNING }
};

Transition<Verb, State> transitionsIdle[] = {
    { Verb::GO, State::RUNNING }
};

// Define states
SMState<Verb, State, Context> stateInit(
    State::INIT, OnInitEntry, OnInitState, OnInitExit,
    transitionsInit, NELEMS(transitionsInit));

SMState<Verb, State, Context> stateRunning(
    State::RUNNING, OnRunningEntry, OnRunningState, OnRunningExit,
    transitionsRunning, NELEMS(transitionsRunning));

SMState<Verb, State, Context> stateStopped(
    State::STOPPED, OnStoppedEntry, OnStoppedState, OnStoppedExit,
    transitionsStopped, NELEMS(transitionsStopped));

SMState<Verb, State, Context> stateIdle(
    State::IDLE, OnIdleEntry, OnIdleState, OnIdleExit,
    transitionsIdle, NELEMS(transitionsIdle));

void testCounters(Context &exp, Context &ctx) {
    TEST_ASSERT_EQUAL_MESSAGE(exp.initEntryCount, ctx.initEntryCount, "initEntryCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.initStateCount, ctx.initStateCount, "initStateCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.initExitCount, ctx.initExitCount, "initExitCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.runningEntryCount, ctx.runningEntryCount, "runningEntryCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.runningStateCount, ctx.runningStateCount, "runningStateCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.runningExitCount, ctx.runningExitCount, "runningExitCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.stoppedEntryCount, ctx.stoppedEntryCount, "stoppedEntryCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.stoppedStateCount, ctx.stoppedStateCount, "stoppedStateCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.stoppedExitCount, ctx.stoppedExitCount, "stoppedExitCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.idleEntryCount, ctx.idleEntryCount, "idleEntryCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.idleStateCount, ctx.idleStateCount, "idleStateCount");
    TEST_ASSERT_EQUAL_MESSAGE(exp.idleExitCount, ctx.idleExitCount, "idleExitCount");
}

void sm_test_basic_transitions() {
    Traceln("Starting basic transitions test");
    Context ctx, exp;
    SMState<Verb, State, Context> states[] = {
        stateInit, stateRunning, stateStopped, stateIdle
    };

    Traceln("Step 1");
    StateMachine<Verb, State, Context> sm(states, NELEMS(states), &ctx, "BasicTransitionsTest");
    testCounters(exp, ctx);
    sm.Start(); // INIT
    exp.initEntryCount++;
    testCounters(exp, ctx);

    Traceln("Step 2");
    ctx.nextVerb = Verb::NONE;
    sm.HandleState();
    exp.initStateCount++;
    testCounters(exp, ctx);

    Traceln("Step 3");
    ctx.nextVerb = Verb::GO; // Set next verb to GO
    sm.HandleState();
    exp.initStateCount++;
    exp.initExitCount++;
    exp.runningEntryCount++;
    testCounters(exp, ctx);

    Traceln("Step 4");
    sm.HandleState();
    exp.runningEntryCount++;
    exp.runningStateCount++;
    exp.runningExitCount++;
    testCounters(exp, ctx);

    Traceln("Step 5");
    ctx.nextVerb = Verb::NONE;
    sm.HandleState();
    exp.runningStateCount++;
    testCounters(exp, ctx);

    Traceln("Step 6");
    ctx.nextVerb = Verb::PAUSE; // Set next verb to IDLE
    sm.HandleState();
    exp.runningStateCount++;
    exp.runningExitCount++;
    exp.idleEntryCount++;
    testCounters(exp, ctx);

    Traceln("Step 7");
    ctx.nextVerb = Verb::GO;
    sm.HandleState();
    exp.idleStateCount++;
    exp.idleExitCount++;
    exp.runningEntryCount++;
    testCounters(exp, ctx);

    Traceln("Step 8");
    ctx.nextVerb = Verb::STOP;
    sm.HandleState();
    exp.runningStateCount++;
    exp.runningExitCount++;
    exp.stoppedEntryCount++;
    testCounters(exp, ctx);

    Traceln("Step 9");
    ctx.nextVerb = Verb::NONE;
    sm.HandleState();
    exp.stoppedStateCount++;
    testCounters(exp, ctx);
}

void sm_test_invalid_verb_transition()
{
    Traceln("Starting invalid verb transition test");
    Context exp, ctx;
    SMState<Verb, State, Context> states[] = {stateInit};

    Traceln("Step 1");
    StateMachine<Verb, State, Context> sm(states, NELEMS(states), &ctx, "VerbErrorTest");
    testCounters(exp, ctx);
    sm.Start();
    exp.initEntryCount++;
    testCounters(exp, ctx);

    // Try invalid transition
    Traceln("Step 2");
    bool exceptionCaught = false;
    ctx.nextVerb = Verb::STOP; // This verb does not exist in the INIT state
    try {
        sm.HandleState();
    } catch (...) {
        exceptionCaught = true;
    }

    TEST_ASSERT(exceptionCaught); // Expected exception for invalid verb transition
    exp.initStateCount++;
    exp.initExitCount++;
    testCounters(exp, ctx);
}

void sm_test_invalid_state_transition()
{
    Traceln("Starting invalid state transition test");
    Context exp, ctx;
    SMState<Verb, State, Context> states[] = {stateInit};

    Traceln("Step 1");
    StateMachine<Verb, State, Context> sm(states, NELEMS(states), &ctx, "StateErrorTest");
    testCounters(exp, ctx);
    sm.Start();
    exp.initEntryCount++;
    testCounters(exp, ctx);

    // Try invalid state
    Traceln("Step 2");
    bool exceptionCaught = false;
    ctx.nextVerb = Verb::GO; // This state does not exist in the state machine
    try {
        sm.HandleState();
    } catch (...) {
        exceptionCaught = true;
    }

    TEST_ASSERT(exceptionCaught); // Expected exception for invalid state transition
    exp.initStateCount++;
    exp.initExitCount++;
    testCounters(exp, ctx);
}

void sm_test_no_entry_exit_actions()
{
    Traceln("Starting test with no entry/exit actions");
    Context exp, ctx;
    SMState<Verb, State, Context> states[] = {
        SMState<Verb, State, Context>(State::INIT, NULL, OnInitState, NULL, transitionsInit, NELEMS(transitionsInit)),
        SMState<Verb, State, Context>(State::RUNNING, NULL, OnRunningState, NULL, transitionsRunning, NELEMS(transitionsRunning))};
    StateMachine<Verb, State, Context> sm(states, NELEMS(states), &ctx, "NoEntryExitTest");

    testCounters(exp, ctx);
    sm.Start();
    testCounters(exp, ctx);

    ctx.nextVerb = Verb::GO; // Transition to RUNNING
    sm.HandleState();
    exp.initStateCount++;
    testCounters(exp, ctx);

    ctx.nextVerb = Verb::NONE; // Stay in RUNNING
    sm.HandleState(); // Handle RUNNING state
    exp.runningStateCount++;
    testCounters(exp, ctx);
}

void sm_test_invalid_states()
{
    Traceln("Starting test with invalid states");
    bool exceptionCaught = false;

    try
    {
        StateMachine<Verb, State, Context> sm(NULL, 2, NULL, "InvalidStatesTest");
    }
    catch(...)
    {
        exceptionCaught = true;
    }
    TEST_ASSERT(exceptionCaught); // Expected exception for NULL states

    exceptionCaught = false; // Reset for next test
    try
    {
        SMState<Verb, State, Context> states[] = {stateInit};
        StateMachine<Verb, State, Context> sm(states, 0, NULL, "InvalidStatesTest");
    }
    catch(...)
    {
        exceptionCaught = true;
    }
    TEST_ASSERT(exceptionCaught); // Expected exception for zero states
    
    exceptionCaught = false; // Reset for next test
    try
    {
        SMState<Verb, State, Context> states[] = {stateInit};
        StateMachine<Verb, State, Context> sm(states, -1, NULL, "InvalidStatesTest");
    }
    catch(...)
    {
        exceptionCaught = true;
    }
    TEST_ASSERT(exceptionCaught); // Expected exception for negative states
}

void sm_test_change_verb_at_exit()
{
    Traceln("Starting test with changing verb at exit");
    Context exp, ctx;
    SMState<Verb, State, Context> states[] = {
        SMState<Verb, State, Context>(
            State::INIT, 
            OnInitEntry, 
            OnInitState, 
            [](Verb verb, Context *ctx) -> Verb 
            {
                TEST_ASSERT_EQUAL_MESSAGE(Verb::STOP, verb, "Expected STOP verb at exit");
                ctx->initExitCount++;
                return Verb::GO; // Change verb at exit
            },
            transitionsInit,
            NELEMS(transitionsInit)),
        stateRunning};
    StateMachine<Verb, State, Context> sm(states, NELEMS(states), &ctx, "ChangeVerbAtExitTest");

    testCounters(exp, ctx);
    sm.Start();
    exp.initEntryCount++;
    testCounters(exp, ctx);

    ctx.nextVerb = Verb::STOP; // Transition to RUNNING
    sm.HandleState();
    exp.initStateCount++;
    exp.initExitCount++;
    exp.runningEntryCount++;
    testCounters(exp, ctx);
}
