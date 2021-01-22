#include <cstdio>
#include <cassert>
#include "monty.h"
#include "pyvm.h"
#include "arch.h"

using namespace monty;

static void runInterp (Callable& init) {
    PyVM vm (init);

    printf("Running ...\n");
    while (vm.isAlive()) {
        vm.scheduler();
        arch::idle();
    }
    printf("Stopped.\n");
}

uint8_t memPool [64*1024];

int main () {
    arch::init();
    setup(memPool, sizeof memPool);

    Bytecode* bc = nullptr;
    auto mod = new Module (builtins);
    Callable dummy (*bc, mod);

    runInterp(dummy);

    gcReport();
    return arch::done();
}
