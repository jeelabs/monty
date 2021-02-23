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

static int blah;
printf("11 %p %p\n", &task, &blah);
Object::dumpAll();
Vec::dumpAll();
//Vec::compact();
//Vec::dumpAll();
printf("22\n");
    while (Stacklet::runLoop())
{
printf("12 %p %p\n", &task, &blah);
Vec::compact();
printf("23\n");
        arch::idle();
}

    gcReport();
    printf("111\n");
    //Stacklet::gcAll();
    Vec::compact();
    printf("222\n");
    gcReport();
#ifndef NDEBUG
    printf("done\n");
#endif
    return arch::done();
}
