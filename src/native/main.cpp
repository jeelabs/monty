#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include <cstdio>

static uint8_t myMem [12*1024]; // tiny mem pool to stress the garbage collector

static const uint8_t* loadFile (const char* fname) {
    FILE* fp = fopen(fname, "rb");
    if (fp == 0)
        return 0;
    fseek(fp, 0, SEEK_END);
    size_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //printf("%s: %db\n", fname, (int) bytes);
    auto buf = (uint8_t*) malloc(bytes + 64);
    auto len = fread(buf, 1, bytes, fp);
    memset(buf + bytes, 0xFF, 64); // simulate empty flash at end
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

    // load simulated "rom" from "file"
    Monty::fsBase = loadFile(argc >= 3 ? argv[2] : "rom.mrfs");

    // name of the bytecode to run
    auto bcData = loadFile(argc >= 2 ? argv[1] : "demo.mpy");

    auto bc = bcData;
    if (bc == nullptr && argc >= 3) // no such file, try mrfs
        bc = Monty::fsLookup(argv[1]);
    if (bc == nullptr)
        return archDone("can't load bytecode");

    // the loader uses garbage-collected memory
    Monty::setup(myMem, sizeof myMem);

    // construct in-memory bytecode objects and sub-objects
    auto init = Monty::loader("__main__", bc);
    if (init == nullptr)
        return archDone("can't load module");

    // orignal bytecode file is no longer needed
    free((void*) bcData);

    // go!
    runInterp(*init);

    // simulated rom data no longer needed
    free((void*) Monty::fsBase);
    return archDone();
}
