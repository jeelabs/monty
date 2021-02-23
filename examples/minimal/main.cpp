#include <monty.h>
#include <arch.h>

using namespace monty;

int main (int argc, char const** argv) {
    arch::init(10*1024);

    auto task = argc > 1 ? vmLaunch(argv[1]) : nullptr;
    if (task != nullptr)
        Stacklet::tasks.append(task);
    else
        printf("no bytecode\n");

    while (Stacklet::runLoop()) {}

    return arch::done();
}
