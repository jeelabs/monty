// Core segment, launched from the boot segment. Needs a non-std linker script.
// After some initialisations, init will locate and register the devs segment.

#include "monty.h"
#include "segment.h"

extern "C" void init () {
    printf("hello from %s\n", "core");

    extern uint8_t g_pfnVectors [], _eflash [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _eflash, _sdata, _ebss, _estack);

    auto hdr = SegmentHdr::next();
    if (hdr.isValid()) {
        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();
    }

    monty::setup((void*) 0x20002000, 48*1024);
    monty::gcReport();

    if (hdr.isValid()) {
        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("goodbye from %s\n", "core");
}
