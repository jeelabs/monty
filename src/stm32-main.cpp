#include <cassert>
#include <cstdint>
#include <cstdlib>
#include "mrfs.h"
#include "monty.h"
#include "pyvm.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

int main () {
    arch::init();

    // STM32L432-specific
    //mrfs::init((void*) 0x08000000, 256*1024, 10*1024); // romend 0x2800
    mrfs::init((void*) 0x08000000, 256*1024, 44*1024); // romend 0xB000
    //mrfs::dump();
    auto pos = mrfs::find("hello");
    printf("hello @ %d\n", pos);
    assert(pos > 0);

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
