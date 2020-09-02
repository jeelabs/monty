#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

static uint8_t myMem [MEM_BYTES];

static void runInterp (Monty::Callable& init) {
    Monty::PyVM vm (init);

    while (vm.isAlive()) {
        vm.scheduler();
        archIdle();
    }

    // nothing left to do or wait for, at this point
}

int main () {
    archInit();

    Monty::fsBase = (uint8_t const*) 0x08010000; // 64 KB past flash begin

    Monty::setup(myMem, sizeof myMem);

    // load module from end of RAM, 4 KB below the stack top
    // this leaves 16 KB on a Blue Pill, and 192 KB on an F4 Discovery
    extern uint8_t _estack [];
    auto bcData = _estack - 0x1000;
    //printf("bytecode at 0x%p\n", bcData);

    auto init = Monty::loader("__main__", bcData);
    if (init == nullptr)
        return archDone("can't load module");

    runInterp(*init);

    //Monty::gcReport(true);
    return archDone();
}
