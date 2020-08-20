#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include <cstdio>

static uint8_t myMem [12*1024]; // tiny mem pool to stress the garbage collector

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
    Monty::PyVM vm (init);

    while (vm.isAlive()) {
        vm.runner();
        archIdle();
    }

    // nothing left to do or wait for, at this point
}

int main (int argc, const char* argv []) {
    archInit();

    auto bcData = loadBytecode(argc == 2 ? argv[1] : "demo.mpy");
    if (bcData == 0)
        return archDone("can't load bytecode");

    Monty::setup(myMem, sizeof myMem);

    auto init = Monty::loadModule("__main__", bcData);
    if (init == nullptr)
        return archDone("can't load module");

    free((void*) bcData);

    runInterp(*init);

    return archDone();
}
