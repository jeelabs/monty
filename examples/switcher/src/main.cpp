#include <jee.h>
#include <monty.h>

using namespace monty;

PinB<3> led;

struct Switcher : Stacklet {
    auto run () -> bool override {
        led.toggle();
        suspend(ready);
        return true;
    }
};

char mem [10000];

int main () {
    led.mode(Pinmode::out);

    gcSetup(mem, sizeof mem); // set up GC memory pool

    Switcher s;

    while (Stacklet::runLoop()) {}
}
