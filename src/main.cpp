#include <cassert>
#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

uint8_t const boot [] = {
#include "boot.h"
};

int main () {
    arch::init();
    setup(memPool, sizeof memPool);

    extern auto vmTest (uint8_t const*) -> Stacklet*;
    auto vm = vmTest(boot);
    printf("vmTest %p\n", vm);

    Stacklet::runLoop();

    gcReport();
    return arch::done();
}
