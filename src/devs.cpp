// Devs segment, launched from the code segment. Needs a non-std linker script.
// The purpose of this segment is to extend core with custom functionality.

#include "monty.h"
#include <jee.h>

using namespace monty;

static Vec vec;

extern "C" void init () {
    printf("cheers from %s\n", "devs");

    extern uint8_t g_pfnVectors [], _eflash [], _sdata [], _ebss [], _estack [];
    printf("  flash %p..%p, ram %p..%p, stack top %p\n",
            g_pfnVectors, _eflash, _sdata, _ebss, _estack);

    printf("  vec %p ok %d ptr %p\n", &vec, vec.adj(1000), vec.ptr());
}
