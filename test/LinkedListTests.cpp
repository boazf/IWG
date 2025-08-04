#include "LinkedListTests.h"
#include "FakeLock.h"
#include <LinkedList.h>
#include <unity.h>

void verifyLinkedList(const std::vector<int> &expected, LinkedList<int> &list)
{
    struct Param
    {
        const int *expected;
        int size;
        int index;
    } param = {expected.data(), static_cast<int>(expected.size()), 0};
    
    list.ScanNodes([](const int &value, const void *vparam) -> bool {
        Param *param = (Param *)vparam;
        TEST_ASSERT_LESS_THAN(param->size, param->index);
        TEST_ASSERT_EQUAL(param->expected[param->index++], value);
        return true;
    }, &param);
    TEST_ASSERT_EQUAL(param.size, param.index);
}

void linkedListInsertTests()
{
    LinkedList<int> list;
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    verifyLinkedList({1}, list);
    list.Insert(2);
    verifyLinkedList({1, 2}, list);
    list.Insert(3);
    verifyLinkedList({1, 2, 3}, list);
}

void linkedListDeleteTests()
{
    LinkedList<int> list;
    list.Insert(1);
    list.Insert(2);
    list.Insert(3);
    TEST_ASSERT_TRUE(list.Delete(2));
    verifyLinkedList({1, 3}, list);
    TEST_ASSERT_TRUE(list.Delete(3));
    verifyLinkedList({1}, list);
    TEST_ASSERT_TRUE(list.Delete(1));
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    list.Insert(3);
    TEST_ASSERT_TRUE(list.Delete(1));
    verifyLinkedList({2, 3}, list);
    TEST_ASSERT_TRUE(list.Delete(3));
    verifyLinkedList({2}, list);
    TEST_ASSERT_TRUE(list.Delete(2));
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    list.Insert(3);
    TEST_ASSERT_TRUE(list.Delete(3));
    verifyLinkedList({1, 2}, list);
    TEST_ASSERT_TRUE(list.Delete(1));
    verifyLinkedList({2}, list);
    TEST_ASSERT_TRUE(list.Delete(2));
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    TEST_ASSERT_TRUE(list.Delete(1));
    verifyLinkedList({2}, list);
    TEST_ASSERT_TRUE(list.Delete(2));
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    TEST_ASSERT_TRUE(list.Delete(2));
    verifyLinkedList({1}, list);
    TEST_ASSERT_TRUE(list.Delete(1));
    TEST_ASSERT_TRUE(list.IsEmpty());
    TEST_ASSERT_FALSE(list.Delete(1)); // Deleting from an empty list should return false
    list.Insert(1);
    TEST_ASSERT_FALSE(list.Delete(2)); // Trying to delete a non-existing element should return false
    verifyLinkedList({1}, list);
}

void linkedListClearAllTests()
{
    LinkedList<int> list;
    list.ClearAll();
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.ClearAll();
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    list.ClearAll();
    TEST_ASSERT_TRUE(list.IsEmpty());
    list.Insert(1);
    list.Insert(2);
    list.Insert(3);
    list.ClearAll();
    TEST_ASSERT_TRUE(list.IsEmpty());
}

void linkedListScanNodesTests()
{
    LinkedList<int> list;
    list.Insert(1);
    list.Insert(2);
    list.Insert(3);
    LinkedList<int> list2;
    list.ScanNodes([](const int &value, const void *param) -> bool 
    {
        LinkedList<int> *list = (LinkedList<int> *)param;
        list->Insert(value);
        return value != 2;
    }, &list2);
    verifyLinkedList({1, 2}, list2);
}
