#include "monty.h"
#include <unity.h>

#include <cstdio>

using namespace Monty;

uint8_t memory [3*1024];
uint32_t memAvail;

static Value json;
static int ihexType;
static int ihexAddr;

struct TestParser : InputParser {
    TestParser (char const* msg) {
        json = {};
        ihexType = -1;
        ihexAddr = -1;

        while (*msg)
            feed(*msg++);
    }

    void onMsg (Value v) override {
        TEST_ASSERT(json.isNil());
        json = v;
    }
    void onBuf (uint8_t t, uint16_t a, uint8_t const* d, uint8_t n) override {
        printf("type %d addr %04x data %p size %d\n", t, a, d, n);
    }
};

void setUp () {
    setup(memory, sizeof memory);
    memAvail = gcAvail();
}

void tearDown () {
    sweep();
    compact();
    TEST_ASSERT_EQUAL(memAvail, gcAvail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void jsonTypeSizes () {
    TEST_ASSERT_EQUAL(12 * sizeof (void*), sizeof (InputParser));
}

static void ihexData () {
    TestParser t1 {":0102030405F1\n"};
    TestParser t2 {":0B0010006164647265737320676170A7\n"};
    TestParser t3 {":00000001FF\n"};
    TestParser t4 {":020000021200EA\n"};
    TestParser t5 {":0400000300003800C1\n"};
    TestParser t6 {":02000004FFFFFC\n"};
    TestParser t7 {":04000005000000CD2A\n"};
}

static void jsonScalar () {
    TestParser t1 {"null\n"};
    TEST_ASSERT(json.isNull());

    TestParser t2 {"false\n"};
    TEST_ASSERT(json.isFalse());

    TestParser t3 {"true\n"};
    TEST_ASSERT(json.isTrue());

    TestParser t4 {"0\n"};
    TEST_ASSERT_EQUAL(0, json.asInt());

    TestParser t5 {"-0\n"};
    TEST_ASSERT_EQUAL(0, json.asInt());

    TestParser t6 {"123\n"};
    TEST_ASSERT_EQUAL(123, json.asInt());

    TestParser t7 {"-123\n"};
    TEST_ASSERT_EQUAL(-123, json.asInt());

    TestParser t8 {"1234567890123456789\n"};
    TEST_ASSERT_EQUAL(1234567890123456789, json.asInt());

    TestParser t9 {"-1234567890123456789\n"};
    TEST_ASSERT_EQUAL(-1234567890123456789, json.asInt());
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(jsonTypeSizes);
    RUN_TEST(ihexData);
    RUN_TEST(jsonScalar);

    UNITY_END();
}
