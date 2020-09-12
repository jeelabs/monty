#include "monty.h"
#include <unity.h>

#include <cstdio>

using namespace Monty;

uint8_t memory [3*1024];
uint32_t memAvail;

static Value json;
static int ihexType;
static int ihexAddr;
static uint8_t const* ihexData;
static int ihexSize;

struct TestParser : InputParser {
    TestParser (char const* msg) {
        json = {};
        ihexType = -1;
        ihexAddr = -1;
        ihexData = nullptr;
        ihexSize = -1;

        while (*msg)
            feed(*msg++);
    }

    void onMsg (Value v) override {
        TEST_ASSERT(json.isNil());
        json = v;
    }
    void onBuf (uint8_t t, uint16_t a, uint8_t const* d, uint8_t n) override {
        //printf("type %d addr %04x data %p size %d\n", t, a, d, n);
        ihexType = t;
        ihexAddr = a;
        ihexData = d;
        ihexSize = n;
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
    TEST_ASSERT_EQUAL(6 * sizeof (void*) + 48, sizeof (InputParser));
}

static void ihexTests () {
    TestParser t1 {":0102030405F1\n"};
    TEST_ASSERT_EQUAL(4, ihexType);
    TEST_ASSERT_EQUAL(0x0203, ihexAddr);
    TEST_ASSERT_EQUAL(1, ihexSize);

    TestParser t2 {":0B0010006164647265737320676170A7\n"};
    TEST_ASSERT_EQUAL(0, ihexType);
    TEST_ASSERT_EQUAL(0x0010, ihexAddr);
    TEST_ASSERT_EQUAL(11, ihexSize);

    TestParser t3 {":00000001FF\n"};
    TEST_ASSERT_EQUAL(1, ihexType);
    TEST_ASSERT_EQUAL(0x0000, ihexAddr);
    TEST_ASSERT_EQUAL(0, ihexSize);

    TestParser t4 {":020000021200EA\n"};
    TEST_ASSERT_EQUAL(2, ihexType);
    TEST_ASSERT_EQUAL(0x0000, ihexAddr);
    TEST_ASSERT_EQUAL(2, ihexSize);

    TestParser t5 {":0400000300003800C1\n"};
    TEST_ASSERT_EQUAL(3, ihexType);
    TEST_ASSERT_EQUAL(0x0000, ihexAddr);
    TEST_ASSERT_EQUAL(4, ihexSize);

    TestParser t6 {":02000004FFFFFC\n"};
    TEST_ASSERT_EQUAL(4, ihexType);
    TEST_ASSERT_EQUAL(0x0000, ihexAddr);
    TEST_ASSERT_EQUAL(2, ihexSize);

    TestParser t7 {":04000005000000CD2A\n"};
    TEST_ASSERT_EQUAL(5, ihexType);
    TEST_ASSERT_EQUAL(0x0000, ihexAddr);
    TEST_ASSERT_EQUAL(4, ihexSize);
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

static void jsonList () {
    TestParser t1 {"[11,22,33]\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<List>());
    TEST_ASSERT_EQUAL(3, json.obj().len());
    TEST_ASSERT_EQUAL(11, json.obj().getAt(0));
    TEST_ASSERT_EQUAL(22, json.obj().getAt(1));
    TEST_ASSERT_EQUAL(33, json.obj().getAt(2));

    TestParser t2 {"[11,[22,33],44]\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<List>());
    TEST_ASSERT_EQUAL(3, json.obj().len());
    TEST_ASSERT_EQUAL(11, json.obj().getAt(0));
    TEST_ASSERT_NOT_NULL(json.obj().getAt(1).ifType<List>());
    TEST_ASSERT_EQUAL(2, json.obj().getAt(1).obj().len());
    TEST_ASSERT_EQUAL(22, json.obj().getAt(1).obj().getAt(0));
    TEST_ASSERT_EQUAL(33, json.obj().getAt(1).obj().getAt(1));
    TEST_ASSERT_EQUAL(44, json.obj().getAt(2));

    TestParser t3 {"[]\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<List>());
    TEST_ASSERT_EQUAL(0, json.obj().len());

    TestParser t4 {"[[]]\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<List>());
    TEST_ASSERT_EQUAL(1, json.obj().len());
    TEST_ASSERT_NOT_NULL(json.obj().getAt(0).ifType<List>());
    TEST_ASSERT_EQUAL(0, json.obj().getAt(0).obj().len());

    TestParser t5 {"[[42]]\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<List>());
    TEST_ASSERT_EQUAL(1, json.obj().len());
    TEST_ASSERT_EQUAL(1, json.obj().getAt(0).obj().len());
    TEST_ASSERT_EQUAL(42, json.obj().getAt(0).obj().getAt(0));
}

static void jsonTuple () {
    TestParser t1 {"(11,22,33)\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Tuple>());
    TEST_ASSERT_EQUAL(3, json.obj().len());
    TEST_ASSERT_EQUAL(11, json.obj().getAt(0));
    TEST_ASSERT_EQUAL(22, json.obj().getAt(1));
    TEST_ASSERT_EQUAL(33, json.obj().getAt(2));

    TestParser t2 {"(11,(22,33),44)\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Tuple>());
    TEST_ASSERT_EQUAL(3, json.obj().len());
    TEST_ASSERT_EQUAL(11, json.obj().getAt(0));
    TEST_ASSERT_NOT_NULL(json.obj().getAt(1).ifType<Tuple>());
    TEST_ASSERT_EQUAL(2, json.obj().getAt(1).obj().len());
    TEST_ASSERT_EQUAL(22, json.obj().getAt(1).obj().getAt(0));
    TEST_ASSERT_EQUAL(33, json.obj().getAt(1).obj().getAt(1));
    TEST_ASSERT_EQUAL(44, json.obj().getAt(2));

    TestParser t3 {"()\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Tuple>());
    TEST_ASSERT_EQUAL(0, json.obj().len());

    TestParser t4 {"(())\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Tuple>());
    TEST_ASSERT_EQUAL(1, json.obj().len());
    TEST_ASSERT_NOT_NULL(json.obj().getAt(0).ifType<Tuple>());
    TEST_ASSERT_EQUAL(0, json.obj().getAt(0).obj().len());

    TestParser t5 {"((42))\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Tuple>());
    TEST_ASSERT_EQUAL(1, json.obj().len());
    TEST_ASSERT_EQUAL(1, json.obj().getAt(0).obj().len());
    TEST_ASSERT_EQUAL(42, json.obj().getAt(0).obj().getAt(0));
}

static void jsonSet () {
    TestParser t1 {"{11,22,33}\n"};
    auto p = json.ifType<Set>();
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(3, p->size());
    TEST_ASSERT_FALSE(p->has(10));
    TEST_ASSERT_TRUE(p->has(11));
    TEST_ASSERT_TRUE(p->has(22));
    TEST_ASSERT_TRUE(p->has(33));
    TEST_ASSERT_FALSE(p->has(34));

    TestParser t2 {"{_}\n"};
    auto q = json.ifType<Set>();
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_EQUAL(0, q->size());
}

static void jsonDict () {
    TestParser t1 {"{11:22,33:44}\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Dict>());
    TEST_ASSERT_EQUAL(2, json.obj().len());
    TEST_ASSERT_EQUAL(22, json.obj().getAt(11));
    TEST_ASSERT_EQUAL(44, json.obj().getAt(33));

    TestParser t2 {"{}\n"};
    TEST_ASSERT_NOT_NULL(json.ifType<Dict>());
    TEST_ASSERT_EQUAL(0, json.obj().len());
}

static void jsonStr () {
    TestParser t1 {"\"abc\"\n"};
    TEST_ASSERT_EQUAL_STRING("abc", json);

    TestParser t2 {"\"a\\tb\\rc\\nd\\?e\\\\f\\\"g\"\n"};
    TEST_ASSERT_EQUAL_STRING("a\tb\rc\nd?e\\f\"g", json);

    TestParser t3 {"\"a\\x01b\\x10c\\x80d\"\n"};
    TEST_ASSERT_EQUAL_STRING("a\x01" "b\x10" "c\x80" "d", json);

    TestParser t4 {"\"a\\u0012b\\u0345c\\u6789d\"\n"};
    TEST_ASSERT_EQUAL_STRING("a\x12" "b\xCD\x85" "c\xE6\x9E\x89" "d", json);
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(jsonTypeSizes);
    RUN_TEST(ihexTests);
    RUN_TEST(jsonScalar);
    RUN_TEST(jsonList);
    RUN_TEST(jsonTuple);
    RUN_TEST(jsonSet);
    RUN_TEST(jsonDict);
    RUN_TEST(jsonStr);

    UNITY_END();
}
