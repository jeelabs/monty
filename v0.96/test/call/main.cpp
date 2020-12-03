#include "monty.h"
#include <unity.h>

using namespace Monty;

uint8_t memory [3*1024];
uint32_t memAvail;

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

void callTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Function));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (MethodBase));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Method));
    TEST_ASSERT_EQUAL(3 * sizeof (void*) + 8, sizeof (Module));
    TEST_ASSERT_EQUAL(2 * sizeof (void*) + 24, sizeof (Bytecode));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Callable));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (BoundMeth));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Cell));
    TEST_ASSERT_EQUAL(3 * sizeof (void*) + 8, sizeof (Closure));
    TEST_ASSERT_EQUAL(3 * sizeof (void*) + 16, sizeof (Context));
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(callTypeSizes);

    UNITY_END();
}
