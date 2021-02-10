#include "monty.h"
#include "arch.h"

using namespace monty;

uint8_t memPool [10*1024];

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO yuck, monty does not know about arch-*
}

int main (int argc, char const** argv) {
    arch::init();

    gcSetup(memPool, sizeof memPool);

    Event::triggers.append(0); // reserve 1st entry for VM TODO yuck

    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
    if (task == nullptr)
        printf("no task\n");
    else
        Stacklet::tasks.append(task);

    while (Stacklet::runLoop())
        arch::idle();

    return arch::done();
}
