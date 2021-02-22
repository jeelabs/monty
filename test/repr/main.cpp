#include <monty.h>
#include <unity.h>

using namespace monty;

uint8_t memory [3*1024];
uint32_t memAvail;

static char buf [250];
static char* bufEnd;

struct TestBuffer : Buffer {
    TestBuffer () { bufEnd = buf; }
    ~TestBuffer () override {
        for (uint32_t i = 0; i < _fill && bufEnd < buf + sizeof buf - 1; ++i)
            *bufEnd++ = begin()[i];
        _fill = 0;
        *bufEnd = 0;
    }
};

void setUp () {
    gcSetup(memory, sizeof memory);
    memAvail = gcMax();
}

void tearDown () {
    Obj::sweep();
    Vec::compact();
    TEST_ASSERT_EQUAL(memAvail, gcMax());
}

void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

void reprTypeSizes () {
    TEST_ASSERT_EQUAL(2*sizeof (uint32_t) + 2*sizeof (void*), sizeof (Buffer));
}

static void reprBasics () {
    { TestBuffer tb; TEST_ASSERT_EQUAL(buf, bufEnd); }
    TEST_ASSERT_EQUAL_STRING("", buf);

    { TestBuffer tb; tb.print("<%d>", 42); }
    TEST_ASSERT_EQUAL_STRING("<42>", buf);

    TestBuffer {} << Value () << ' ' << 123 << " abc " << (Value) "def";
    TEST_ASSERT_EQUAL_STRING("_ 123 abc \"def\"", buf);

    TestBuffer {} << Null << ' ' << True << ' ' << False;
    TEST_ASSERT_EQUAL_STRING("null true false", buf);

    TestBuffer {} << Object {};
    TEST_ASSERT_EQUAL_STRING_LEN("<<object> at ", buf, 13);

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
