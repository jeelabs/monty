#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO yuck, monty does not know about arch-*
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
    auto task = argc > 1 ? vmLaunch(arch::loadFile(argv[1])) : nullptr;
#else
    auto task = arch::cliTask(vmLaunch);
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
