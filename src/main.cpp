#include <monty.h>
#include <arch.h>

using namespace monty;

int main (int argc, char const** argv) {
    arch::init(12*1024);
#ifndef NDEBUG
    printf("main\n");
#endif

//  libInstall();

#if NATIVE
    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
#else
    (void) argc; (void) argv;
    auto task = arch::cliTask();
#endif
    if (task != nullptr)
        Stacklet::tasks.append(task);
    else
        printf("no task\n");

    while (Stacklet::runLoop())
        arch::idle();

#ifndef NDEBUG
    printf("done\n");
#endif
    return arch::done();
}
