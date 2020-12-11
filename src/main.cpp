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
    setbuf(stdout, nullptr);
    puts("NATIVE hello!");

    monty::setup(myMem, sizeof myMem);

    monty::Bytecode* bc = nullptr;
    auto mod = new monty::Module (monty::builtins);
    monty::Callable dummy (*bc, mod);
    runInterp(dummy);

    monty::gcReport();
}
