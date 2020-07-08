#include <lwip/sys.h>

extern "C" {
#include "enchw.h"
}

#include <jee.h>

SpiGpio< PinA<7>, PinA<6>, PinA<5>, PinA<4> > spi;

void enchw_setup (enchw_device_t*) {
    spi.init();

    PinA<3> reset; reset.mode(Pinmode::out);
    reset = 0; wait_ms(2); reset = 1; wait_ms(10);
}

void enchw_select (enchw_device_t*) {
    spi.enable();
}

void enchw_unselect (enchw_device_t*) {
    spi.disable();
}

uint8_t enchw_exchangebyte (enchw_device_t*, uint8_t b) {
    return spi.transfer(b);
}

uint32_t sys_now () {
    return ticks;
}
