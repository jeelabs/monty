#include "monty.h"
#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;

void setUp () {
    setup(memory, sizeof memory);
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

void callTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Function));
    TEST_ASSERT_EQUAL(sizeof (void*), sizeof (MethodBase));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Method));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Module));
    TEST_ASSERT_EQUAL(4 * sizeof (void*) + 16, sizeof (Bytecode)); // TODO hack
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Callable));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (BoundMeth));
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Cell));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Closure));
    TEST_ASSERT_EQUAL(5 * sizeof (void*) + 8, sizeof (Context));
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(callTypeSizes);

    UNITY_END();
}
