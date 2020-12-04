#include <jee.h>
#include "segment.h"

extern "C" void init () {
    printf("hello from %s\n", "core");

    auto hdr = nextSegment();
    if (hdr.magic == 0x12345678) {
        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();
        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("goodbye from %s\n", "core");
}
