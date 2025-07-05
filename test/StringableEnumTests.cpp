#include <unity.h>
#include "StringableEnumTests.h"
#include <StringableEnum.h>

#define TEST_ENUM_VALUES \
    X(Value1) \
    X(Value2) \
    X(Value3)

#define X(a) a,
enum class TestEnum
{
    TEST_ENUM_VALUES
};
#undef X

#define X(a) {TestEnum::a, #a},
template<>
const std::map<TestEnum, std::string> StringableEnum<TestEnum>::strMap = 
{
    TEST_ENUM_VALUES
};
#undef X

void test_stringable_enum()
{
    StringableEnum<TestEnum> enumValue(TestEnum::Value1);
    TEST_ASSERT_EQUAL_STRING("Value1", enumValue.ToString().c_str());

    enumValue = StringableEnum<TestEnum>(TestEnum::Value2);
    TEST_ASSERT_EQUAL_STRING("Value2", enumValue.ToString().c_str());

    enumValue = StringableEnum<TestEnum>(TestEnum::Value3);
    TEST_ASSERT_EQUAL_STRING("Value3", enumValue.ToString().c_str());
}

void test_stringable_enum_invalid()
{
    StringableEnum<TestEnum> enumValue(static_cast<TestEnum>(999)); // Invalid value
    TEST_ASSERT_EQUAL_STRING(StringableEnum<TestEnum>::unknown().c_str(), enumValue.ToString().c_str());
}

void test_StringableEnum()
{
    UNITY_BEGIN();
    RUN_TEST(test_stringable_enum);
    RUN_TEST(test_stringable_enum_invalid);
    UNITY_END();
}