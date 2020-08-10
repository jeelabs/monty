#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;

static char buf [250];
static char* fill;

struct TestBuffer : Buffer {
    TestBuffer () { fill = buf; }
    ~TestBuffer () override { *fill = 0; }

    void write (uint8_t const* ptr, size_t len) const override {
        while (len-- > 0 && fill < buf + sizeof buf - 1)
            *fill++ = *ptr++;
    }
};

void setUp () {
    setup(memory);
    memAvail = avail();
}

void tearDown () {
    sweep();
    compact();
    TEST_ASSERT_EQUAL(memAvail, avail());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void reprTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Buffer));
}

static void reprBasics () {
    { TestBuffer tb; TEST_ASSERT_EQUAL(buf, fill); }
    TEST_ASSERT_EQUAL_STRING("", buf);

    { TestBuffer tb; tb.print("<%d>", 42); }
    TEST_ASSERT_EQUAL_STRING("<42>", buf);

    TestBuffer {} << Value () << 123 << "abc";
    TEST_ASSERT_EQUAL_STRING("Nil 123 \"abc\"", buf);

    TestBuffer {} << Null << True << False;
    TEST_ASSERT_EQUAL_STRING("null true false", buf);

    TestBuffer {} << Object {};
    TEST_ASSERT_EQUAL_STRING_LEN("<<object> at ", buf, 13);

    TestBuffer {} << Context {};
    TEST_ASSERT_EQUAL_STRING_LEN("<<context> at ", buf, 14);

    TestBuffer {} << Buffer::info;
    TEST_ASSERT_EQUAL_STRING("<type <buffer>>", buf);

    { TestBuffer tb; tb << tb; }
    TEST_ASSERT_EQUAL_STRING_LEN("<<buffer> at ", buf, 13);
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(reprTypeSizes);
    RUN_TEST(reprBasics);

    UNITY_END();
}
