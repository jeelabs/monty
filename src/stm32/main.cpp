#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "monty.h"
#include "arch.h"
//#include <cstdio>

int main () {
    archInit();
    printf("\xFF" // send out special marker for easier remote output capture
           "main\n");

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

#if STM32F1 // won't fit on a Blue Pill
    static uintptr_t myMem [3072];
#else
    static uintptr_t myMem [4096];
#endif
    Monty::setup(myMem);

    extern uint8_t _estack [];
    (void) Monty::loadModule(_estack - 0x1000);

    printf("done\n");
    return archDone();
}
