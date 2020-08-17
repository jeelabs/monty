#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

static uint8_t myMem [MEM_BYTES];

int main () {
    archInit();
    printf("\xFF" "main\n"); // insert marker for serial capture by dog.c

    Monty::setup(myMem, sizeof myMem);

    // load module from end of RAM, 4 KB below the stack top
    // this leaves 16 KB on a Blue Pill, and 192 KB on an F4 Discovery
    extern uint8_t _estack [];
    auto bcData = _estack - 0x1000;

    auto init = Monty::loadModule("__main__", bcData);
    if (init == nullptr) {
        printf("can't load module\n");
        return archDone();
    }

    Monty::Context ctx;
    ctx.enter(*init);
    ctx.frame().locals = &init->mo;
    Monty::Interp::context = &ctx;

    Monty::PyVM vm;

    while (vm.isAlive()) {
        vm.runner();
        archIdle();
    }
    // nothing to do and nothing to wait for at this point

    return archDone();
}
