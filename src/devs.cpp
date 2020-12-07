// Devs segment, launched from the code segment. Needs a non-std linker script.
// The purpose of this segment is to extend core with custom functionality.

#include <jee.h>

extern "C" void init () {
    printf("cheers from %s\n", "devs");

    extern uint8_t g_pfnVectors [], _eflash [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _eflash, _sdata, _ebss, _estack);
}
