#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"

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

void stateTypeSizes () {
    TEST_ASSERT_EQUAL(2 * sizeof (void*), sizeof (Function));
    TEST_ASSERT_EQUAL(3 * sizeof (void*), sizeof (BoundMeth));
    TEST_ASSERT_EQUAL(6 * sizeof (void*), sizeof (Context));
}

int main () {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);
    RUN_TEST(stateTypeSizes);

    UNITY_END();
}
