#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

uint8_t const boot [] = {
#include "boot.h"
};

extern auto vmTest (uint8_t const*) -> Stacklet*;

auto shell (char const* cmd) -> bool {
    printf("cmd <%s>\n", cmd);
    if (cmd[0] == 'M' && cmd[1] == 0x05)
        vmTest((uint8_t const*) cmd);
    return true; //XXX *cmd != 0;
}

int main (int argc, char const** argv) {
    arch::init();

    gcSetup(memPool, sizeof memPool);
    libInstall();

    auto data = boot;
#if NATIVE
    if (argc > 1)
        data = arch::loadFile(argv[1]);
#endif

    if (vmTest(data) == nullptr)
        printf("no VM\n");

    arch::cliTask(shell);

    while (Stacklet::runLoop())
        arch::idle();

    return arch::done();
}
