#include <monty.h>
#include <arch.h>

using namespace monty;

int main ([[maybe_unused]] int argc, [[maybe_unused]] char const** argv) {
    arch::init(12*1024);
#ifndef NDEBUG
    printf("main\n");
#endif

    initMyLib(); // defined in my-lib.h

#if NATIVE
    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
#else
    auto task = arch::cliTask();
#endif
    if (task != nullptr)
        Stacklet::ready.append(task);
    else
        printf("no task\n");

    while (Stacklet::runLoop())
        arch::idle();

#ifndef NDEBUG
    printf("done\n");
#endif
    arch::done();
}
