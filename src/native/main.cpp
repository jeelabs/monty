#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"
#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"
#include "util.h"

#include "xmonty.h"

static bool runInterp (const uint8_t* data) {
    Interp vm;

    ModuleObj* mainMod = 0;
    if (data[0] == 'M' && data[1] == 5) {
        Loader loader;
        mainMod = loader.load (data);
        vm.qPool = loader.qPool;
    }

    if (mainMod == 0)
        return false;

    vm.start(*mainMod, builtinDict);

    while (vm.isAlive()) {
        vm.run();
        INNER_HOOK // make sure this runs, even when there is no work to do
    }

    return true;
}

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
    printf("main qstr #%d %db\n", (int) qstrNext, (int) sizeof qstrData);

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    auto bcData = loadBytecode(argc == 2 ? argv[1] : "demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return 1;
    }

    auto ok = true; // XXX runInterp(bcData);

    static uintptr_t myMem [16*1024]; // TODO don't gc too soon ...
    Monty::setup(myMem, sizeof myMem);
    printf("new load %p\n", Monty::loadModule(bcData));

    free((void*) bcData);

    if (!ok) {
        printf("can't load module\n");
        return 2;
    }

    printf("done\n");
    return archDone();
}
