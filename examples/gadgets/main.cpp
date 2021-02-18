#include "monty.h"
#include "arch.h"

using namespace monty;

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO get rid of this
}

int main (int argc, char const** argv) {
    static uint8_t memPool [10*1024];
    gcSetup(memPool, sizeof memPool);

    Event::triggers.append(0); // TODO get rid of this

    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
    if (task != nullptr)
        Stacklet::tasks.append(task);
    else
        printf("no bytecode\n");

    while (Stacklet::runLoop()) {}

    return 0;
}
