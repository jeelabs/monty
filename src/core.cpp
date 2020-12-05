#include "monty.h"
#include "gc.h"
#include "segment.h"

static uint8_t myMem [12*1024]; // tiny mem pool to stress the garbage collector

extern "C" void init () {
    printf("hello from %s\n", "core");

    extern uint8_t end [];
    printf("end %p\n", end);

    auto hdr = SegmentHdr::next();
    if (hdr.isValid()) {
        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();

        Monty::setup(myMem, sizeof myMem);
        Monty::gcReport();

        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("goodbye from %s\n", "core");
}
