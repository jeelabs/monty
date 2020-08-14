#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include <assert.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "xmonty.h"
#include "arch.h"

int main () {
    archInit();
    printf("\xFF" // send out special marker for easier remote output capture
           "main\n");

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    static uintptr_t myMem [4096];
    Monty::setup(myMem, sizeof myMem);

    extern uint8_t _estack [];
    (void) Monty::loadModule(_estack - 0x1000);

    printf("done\n");
    return archDone();
}
