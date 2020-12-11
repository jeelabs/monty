// This is for native use, see boot/core/devs for the layered embedded builds.

#include <cassert>
#include <cstdio>
#include "monty.h"
#include "pyvm.h"

static uint8_t myMem [64*1024];

static void runInterp (monty::Callable& init) {
    monty::PyVM vm (init);

    printf("Running ...\n");
    while (vm.isAlive()) {
        vm.scheduler();
        //XXX archIdle();
    }
    printf("Stopped.\n");
}

int main () {
    puts("NATIVE hello!");

    monty::setup(myMem, sizeof myMem);

    //extern auto loadModule (char const*) -> monty::Bytecode const&;
    //monty::Callable dummy (loadModule("hello"));
    monty::Bytecode* bc = nullptr;
    monty::Callable dummy (*bc);
    runInterp(dummy);

    monty::gcReport();
}
