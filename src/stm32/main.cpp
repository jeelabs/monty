#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "monty.h"
#include "arch.h"

int main () {
    archInit();
    printf("\xFF" // send out special marker for easier remote output capture
           "main\n");

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    static uint8_t myMem [MEM_BYTES];
    Monty::setup(myMem, sizeof myMem);

    // load module from end of RAM, 4 KB below the stack top
    // this leaves 16 KB on a Blue Pill, and 192 KB on an F4 Discovery
    extern uint8_t _estack [];
    auto bcData = _estack - 0x1000;

    if (Monty::loadModule(bcData) == nullptr)
        printf("can't load module\n");
    else
        printf("done\n");

    return archDone();
}
