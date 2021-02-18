#include "monty.h"
#include "arch.h"

using namespace monty;

#if NATIVE
uint8_t memPool [20*1024];
#else
uint8_t memPool [10*1024];
#endif

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO yuck, monty does not know about arch-*
}

int main (int argc, char const** argv) {
    arch::init();
#ifndef NDEBUG
    printf("main\n");
#endif

#if 1
    gcSetup(memPool, sizeof memPool);
//  libInstall();

    Event::triggers.append(0); // reserve 1st entry for VM TODO yuck

#if NATIVE
    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
#else
    auto task = arch::cliTask(vmLaunch);
#endif

    if (task == nullptr)
        printf("no task\n");
    else
        Stacklet::tasks.append(task);

    while (Stacklet::runLoop())
        arch::idle();
#endif

#ifndef NDEBUG
    printf("done\n");
#endif
    return arch::done();
}
