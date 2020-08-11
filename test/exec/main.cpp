#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

#include <unity.h>

using namespace Monty;

uintptr_t memory [1024];
size_t memAvail;

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

void execTypeSizes () {
    TEST_ASSERT_EQUAL(6 * sizeof (void*), sizeof (Module));
    TEST_ASSERT_EQUAL(5 * sizeof (void*), sizeof (Callable));
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(execTypeSizes);

    UNITY_END();
}
