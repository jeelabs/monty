// Devs segment, launched from the code segment. Needs a non-std linker script.
// The purpose of this segment is to extend core with custom functionality.

#include <jee.h>

extern "C" void init () {
    printf("cheers from %s\n", "devs");
}
