#include <jee.h>
#include "segment.h"

extern "C" void init () {
    printf("hello from %s\n", "core");

    auto hdr = SegmentHdr::next();
    if (hdr.isValid()) {
        printf("  core -> regFun %p\n", hdr.regFun);
        hdr.regFun();
        printf("  core -> deregFun %p\n", hdr.deregFun);
        hdr.deregFun();
    }

    printf("goodbye from %s\n", "core");
}
