#include "ObserversTests.h"
#include <FakeLock.h>
#include <Observers.h>
#include <unity.h>


void observersBasicTests()
{
    Observers<int> observers;
    struct Param
    {
        int data;
        int count;
    } param = {5, 0};

    static int expectedData;
    static void (*verifyObserverData)(int, int, const void *) = [](int expectedParam, int data, const void *context)
    {
        Param *param = (Param *)context;
        TEST_ASSERT_EQUAL(expectedData, data);
        TEST_ASSERT_EQUAL(expectedParam, param->data);
        param->count++;
    };

    int token1 = observers.addObserver([](const int &data, void *context)
    {
        verifyObserverData(5, data, context);
    }, &param);
    TEST_ASSERT_EQUAL(0, param.count);

    expectedData = 42;
    observers.callObservers(expectedData);
    TEST_ASSERT_EQUAL(1, param.count);

    Param param2 = {6, 0};
    int token2 = observers.addObserver([](const int &data, void *context)
    {
        verifyObserverData(6, data, context);
    }, &param2,
    [](void *ctx)
    {
        // Free function for context cleanup if needed
        Param *p = (Param *)ctx;
        // For this test, nothing to free, but could add logic here
        p->count = -1;
    });
    TEST_ASSERT_EQUAL(1, param.count);
    TEST_ASSERT_EQUAL(0, param2.count);

    expectedData = 10;
    observers.callObservers(expectedData);
    TEST_ASSERT_EQUAL(2, param.count);
    TEST_ASSERT_EQUAL(1, param2.count);

    expectedData = 200;
    observers.callObservers(expectedData);
    TEST_ASSERT_EQUAL(3, param.count);
    TEST_ASSERT_EQUAL(2, param2.count);

    observers.removeObserver(token2);
    TEST_ASSERT_EQUAL(3, param.count);
    TEST_ASSERT_EQUAL(-1, param2.count);

    expectedData = 100;
    observers.callObservers(expectedData);
    TEST_ASSERT_EQUAL(4, param.count);
    TEST_ASSERT_EQUAL(-1, param2.count);

    observers.removeObserver(token1);
    observers.callObservers(-1);
    TEST_ASSERT_EQUAL(4, param.count);
    TEST_ASSERT_EQUAL(-1, param2.count);

    Param param3 = {-1, 0};
    observers.addObserver([](const int &data, void *context)
    {
        verifyObserverData(-1, data, context);
    }, &param3);
    TEST_ASSERT_EQUAL(4, param.count);
    TEST_ASSERT_EQUAL(-1, param2.count);
    TEST_ASSERT_EQUAL(0, param3.count);
    
    expectedData = 7;
    observers.callObservers(expectedData);
    TEST_ASSERT_EQUAL(4, param.count);
    TEST_ASSERT_EQUAL(-1, param2.count);
    TEST_ASSERT_EQUAL(1, param3.count);

}
