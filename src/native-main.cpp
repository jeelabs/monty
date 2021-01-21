#include <cassert>
#include <cstdio>
#include "monty.h"
#include "pyvm.h"

using namespace monty;

static void runInterp (monty::Callable& init) {
    monty::PyVM vm (init);

    printf("Running ...\n");
    while (vm.isAlive()) {
        vm.scheduler();
        //XXX archIdle();
    }
    printf("Stopped.\n");
}

uint8_t memPool [64*1024];

int main () {
    setbuf(stdout, nullptr);
    puts("NATIVE hello!");

    monty::setup(memPool, sizeof memPool);

    monty::Bytecode* bc = nullptr;
    auto mod = new monty::Module (monty::builtins);
    monty::Callable dummy (*bc, mod);

    runInterp(dummy);

    monty::gcReport();
}
