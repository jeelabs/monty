#include "monty.h"
#include "jet.h"
#include "arch.h"

using namespace monty;
using namespace jet;

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO get rid of this
}

int main (int argc, char const** argv) {
    static uint8_t memPool [10*1024];
    gcSetup(memPool, sizeof memPool);

    Event::triggers.append(0); // TODO get rid of this

    auto task = new Flow;
    Stacklet::tasks.append(task);

    while (Stacklet::runLoop()) {}

    return 0;
}
