#include <monty.h>
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

char mem [10000];

int main () {
    led.mode(Pinmode::out);

    gcSetup(mem, sizeof mem); // set up GC memory pool

    tasks.append(new Switcher);

    while (Stacklet::runLoop()) {}
}
