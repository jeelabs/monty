#include <monty.h>
#include "arch.h"

#include <cstdio>

using namespace monty;

void arch::init () {
    setbuf(stdout, nullptr);
}

void arch::idle () {
    // nothing ...
}

int arch::done () {
    return 0;
}
