#include <monty.h>
#include <arch.h>
#include <jee.h>

using namespace monty;

PinB<3> led;

struct Switcher : Stacklet {
    auto run () -> bool override {
        led.toggle();
        yield();
        return true;
    }
};

int main () {
    led.mode(Pinmode::out);

    char mem [2000];
    gcSetup(mem, sizeof mem); // set up a small GC memory pool

    Stacklet::ready.append(new Switcher);

    while (Stacklet::runLoop()) {}

    return 0;
}
