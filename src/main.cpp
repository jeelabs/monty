#include <cassert>
#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

uint8_t const boot [] = {
#include "boot.h"
};

int main (int argc, char const** argv) {
    arch::init();
#ifndef NDEBUG
    printf("build " __DATE__ ", " __TIME__ "\n"
           "using " __VERSION__ "\n"
           "<------------------------------------------------------------->\n");
#endif

    setup(memPool, sizeof memPool);

    auto data = boot;
#if NATIVE
    if (argc > 1) {
        data = arch::loadFile(argv[1]);
        if (data == nullptr)
            return 1; // exit with error code
    }
#endif

    extern auto vmTest (uint8_t const*) -> Stacklet*;
    vmTest(data);

    Stacklet::runLoop();

#ifndef NDEBUG
    printf("</>\n");
    gcReport();
#endif
    return arch::done();
}
