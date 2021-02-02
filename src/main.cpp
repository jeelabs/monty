#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

extern auto vmLaunch (uint8_t const*) -> Stacklet*;

auto shell (char const* cmd) -> bool {
    if (cmd[0] == 'M' && cmd[1] == 0x05) {
        auto vm = vmLaunch((uint8_t const*) cmd);
        if (vm != nullptr)
            tasks.append(vm);
        return false;
    }
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

    handlers.append(0); // reserve 1st entry for VM TODO yuck

#if NATIVE
    Stacklet* task = nullptr;
    if (argc > 1) {
        auto data = arch::loadFile(argv[1]);
        if (data != nullptr)
            task = vmLaunch(data);
    }
#else
    auto task = arch::cliTask(shell);
#endif

    if (task == nullptr)
        printf("no task\n");
    else
        tasks.append(task);

    while (Stacklet::runLoop())
        arch::idle();

#ifndef NDEBUG
    printf("done\n");
#endif
    return arch::done();
}
