// The actual tests are in header files, grouped by functionality.

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "monty.h"

// stubs for arch.h and arch.cpp
#include "../src/version.h"
const ModuleObj m_machine;

#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"

#include <unity.h>

// void setUp () {}
// void tearDown () {}

#include "vec.h"
#include "dict.h"
#include "vm.h"

static void smokeTest () {
    TEST_ASSERT_EQUAL(42, 40 + 2);
}

int main (int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(smokeTest);

    RUN_TEST(vecInstance);
    RUN_TEST(vecSetAndGrow);
    RUN_TEST(vecOfValue);

    RUN_TEST(dictInstance);
    RUN_TEST(dictLookupGrow);

    RUN_TEST(vmInstance);
    RUN_TEST(vmGcTrigger);

    UNITY_END();
    return 0;
}
