#include "monty.h"
#include "jet.h"
#include "arch.h"

using namespace monty;
using namespace jet;

extern Module m_jet;

static uint8_t memPool [20*1024];

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO get rid of this
}

int main (int argc, char const** argv) {
    arch::init();
    printf("main\n");

    gcSetup(memPool, sizeof memPool);

    Event::triggers.append(0); // TODO get rid of this

    m_jet.setAt("__name__", "jet");
    Module::loaded.at("jet") = m_jet;

    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
    if (task == nullptr)
        printf("no task\n");
    else
        Stacklet::tasks.append(task);

    while (Stacklet::runLoop())
        arch::idle();

    printf("done\n");
    return arch::done();
}
