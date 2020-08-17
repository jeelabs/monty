#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include <cstdio>

static uint8_t myMem [512*1024]; // TODO don't gc too soon ...

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

int main (int argc, const char* argv []) {
    archInit();
    printf("main\n");

    auto bcData = loadBytecode(argc == 2 ? argv[1] : "demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return 1;
    }

    Monty::setup(myMem, sizeof myMem);

    auto init = Monty::loadModule("__main__", bcData);
    if (init == nullptr)
        return 2;

    Monty::Context ctx;
    ctx.enter(*init);
    ctx.frame().locals = &init->mo;
    Monty::Interp::context = &ctx;

    Monty::PyVM vm;

    while (vm.isAlive()) {
        vm.runner();
        archIdle();
    }
    // nothing to do and nothing to wait for at this point

    printf("done\n");
    return archDone();
}
