#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include <assert.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"
#include "arch.h"

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

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    auto bcData = loadBytecode(argc == 2 ? argv[1] : "demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return 1;
    }

    static uintptr_t myMem [32*1024]; // TODO don't gc too soon ...
    Monty::setup(myMem, sizeof myMem);

    if (Monty::loadModule(bcData) == nullptr) {
        printf("can't load module\n");
        return 2;
    }

    printf("done\n");
    return archDone();
}
