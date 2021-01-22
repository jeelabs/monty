#include <cstdio>
#include <cassert>
#include "monty.h"
#include "pyvm.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [64*1024];

int main () {
    arch::init();
    setup(memPool, sizeof memPool);

    Bytecode* bc = nullptr;
    auto mod = new Module (builtins);
    Callable init (*bc, mod);

    {
        PyVM vm (init);

        printf("Running ...\n");
        while (vm.isAlive()) {
            vm.scheduler();
            arch::idle();
        }
        printf("Stopped.\n");
    }

    gcReport();
    return arch::done();
}
