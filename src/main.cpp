// This is for native use, see boot/core/devs for the layered embedded builds.

#include <cassert>
#include <cstdio>
#include "monty.h"
#include "pyvm.h"

static uint8_t myMem [64*1024];

static void runInterp (monty::Callable& init) {
    monty::PyVM vm (init);

    while (vm.isAlive()) {
        vm.scheduler();
        //XXX archIdle();
    }

    // nothing left to do or wait for, at this point
}

int main () {
    puts("NATIVE hello!");

    monty::setup(myMem, sizeof myMem);

    extern auto loadModule (char const*) -> monty::Bytecode const&;
    monty::Callable dummy (loadModule("hello"));
    runInterp(dummy);

    monty::gcReport();
}
