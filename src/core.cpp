// Core segment, launched from the boot segment. Needs a non-std linker script.
// After some initialisations, init will locate and register the devs segment.

#include "monty.h"
#include "segment.h"

using namespace monty;

extern "C" void init () {
    printf("hello from %s\n", "core");

    extern uint8_t g_pfnVectors [], _eflash [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _eflash, _sdata, _ebss, _estack);

    auto hdr = SegmentHdr::next();
    if (hdr.isValid()) {
        extern uint8_t _estack [];
        constexpr auto stackBottom = _estack - 1024;
        printf("  gc pool %p..%p\n", hdr.ramEnd, stackBottom);

        monty::setup(hdr.ramEnd, stackBottom - hdr.ramEnd);
        monty::gcReport();

        {
            Vec vec;
            printf("  vec %p ok %d ptr %p\n", &vec, vec.adj(100), vec.ptr());

            monty::gcReport();

            printf("  core -> regFun %p\n", hdr.regFun);
            hdr.regFun();

            monty::gcReport();
        }

        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();

        monty::gcNow();
        monty::gcReport();
    }

    printf("goodbye from %s\n", "core");
}
