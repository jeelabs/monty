// Core layer, launched from the boot layer. Needs a non-std linker script.
// After some initialisations, init will locate and register the devs layer.

#include <cassert>
#include "monty.h"
#include "pyvm.h"
#include "layer.h"

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

extern "C" void init () {
    printf("hello from %s\n", "core");

    extern uint8_t g_pfnVectors [], _eflash [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _eflash, _sdata, _ebss, _estack);

    auto hdr = LayerHdr::next();
    if (hdr.isValid()) {
        extern uint8_t _estack [];
        constexpr auto stackBottom = _estack - 1024;
        printf("  gc pool %p..%p\n", hdr.ramEnd, stackBottom);

        monty::setup(hdr.ramEnd, stackBottom - hdr.ramEnd);
        //monty::gcReport();

        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();
        //monty::gcReport();

        monty::Bytecode* bc = nullptr;
        monty::Callable dummy (*bc);
        runInterp(dummy);

        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();

        monty::gcNow();
        monty::gcReport();
    }

    printf("goodbye from %s\n", "core");
}
