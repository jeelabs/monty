#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

extern auto vmTest (uint8_t const*) -> Stacklet*;

auto shell (char const* cmd) -> bool {
    if (cmd[0] == 'M' && cmd[1] == 0x05)
        vmTest((uint8_t const*) cmd);
    else
        printf("cmd <%s>\n", cmd);
    return *cmd != '!';
}

int main (int argc, char const** argv) {
    arch::init();
#ifndef NDEBUG
    printf("main\n");
#endif

    gcSetup(memPool, sizeof memPool);
//  libInstall();

    if (handlers.size() == 0) // no events may have been registered yet
        handlers.append(None::nullObj); // reserve 1st entry for VM TODO yuck

#if NATIVE
    uint8_t const* data = argc > 1 ? arch::loadFile(argv[1]) : nullptr;
    if (data == nullptr || vmTest(data) == nullptr)
        printf("no bytecode\n");
#else
    arch::cliTask(shell);
#endif

    while (Stacklet::runLoop())
        arch::idle();

#ifndef NDEBUG
    printf("done\n");
#endif
    return arch::done();
}
