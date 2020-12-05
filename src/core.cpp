// Core segment, launched from the boot segment. Needs a non-std linker script.
// After some initialisations, init will locate and register the devs segment.

#include "monty.h"
#include "gc.h"
#include "segment.h"

extern "C" void init () {
    printf("hello from %s\n", "core");

    auto hdr = SegmentHdr::next();
    if (hdr.isValid()) {
        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();

        Monty::setup((void*) 0x20002000, 56*1024);
        Monty::gcReport();

        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("goodbye from %s\n", "core");
}
