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
        asm("wfi");
    }

    return true;
}

int main () {
    archInit();
    printf("\xFF" // send out special marker for easier remote output capture
           "main qstr #%d %db\n", (int) qstrNext, (int) sizeof qstrData);

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    extern uint8_t _estack [];
    if (!runInterp(_estack - 0x1000)) // 4 KB below end of RAM
        printf("can't load bytecode\n");

#ifndef STM32F1 // won't fit on a Blue Pill
    // TODO load using the new code ...
    static uintptr_t myMem [4096];
    Monty::setup(myMem, sizeof myMem);
    printf("new load %p\n", Monty::loadModule(_estack - 0x1000));
#endif

    printf("done\n");
    return archDone();
}
