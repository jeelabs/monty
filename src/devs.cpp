#include <jee.h>

extern "C" void init () {
    printf("cheers from %s\n", "devs");

    extern uint8_t end [];
    printf("end %p\n", end);
}
