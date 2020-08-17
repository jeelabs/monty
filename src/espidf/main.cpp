// ToyPy main application, this is a test shell for the Monty VM.

#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

static uintptr_t myMem [4096];

static const uint8_t* loadBytecode (const char* fname) {
    FILE* fp = fopen(fname, "rb");
    if (fp == 0)
        return 0;
    fseek(fp, 0, SEEK_END);
    size_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //printf("bytecode size %db\n", (int) bytes);
    auto buf = (uint8_t*) malloc(bytes);
    auto len = fread(buf, 1, bytes, fp);
    fclose(fp);
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}

static void runInterp (Monty::Callable& init) {
    Monty::Context ctx;
    ctx.enter(init);
    ctx.frame().locals = &init.mo;
    Monty::Interp::context = &ctx;

    Monty::PyVM vm;

    while (vm.isAlive()) {
        vm.runner();
        //archIdle();
    }
    // nothing to do and nothing left to wait for at this point
}

extern "C" int app_main () {
    vTaskDelay(3000/10); // 3s delay, enough time to attach serial
    printf("\xFF" "main\n"); // insert marker for serial capture by dog.c

    Monty::setup(myMem, sizeof myMem);

    auto bcData = loadBytecode("demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return 1;
    }

    auto init = Monty::loadModule("__main__", bcData);
    if (init == nullptr) {
        printf("can't load module\n");
        return 2;
    }

    runInterp(*init);

    printf("done\n");
    return 0;
}
