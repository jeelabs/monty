#include <monty.h>
#include <jee.h>

using namespace monty;

PinB<3> led;
PinA<6> led1;
PinA<5> led2;
PinA<4> led3;
PinA<3> led4;
PinA<1> led5;
PinA<0> led6;

void onOff (int n, bool f) {
    switch (n) {
        case 0: led.write(f); break;
        case 1: led1.write(f); break;
        case 2: led2.write(f); break;
        case 3: led3.write(f); break;
        case 4: led4.write(f); break;
        case 5: led5.write(f); break;
        case 6: led6.write(f); break;
    }
}

// silly delay mechanism: keep putting ourselves (the current stacklet)
// at the end of the ready queue until the time limit has been reached

void delay_ms (uint16_t ms) {
    auto now = ticks;
    while (ticks - now < ms) {
        ready.append(Stacklet::current);
        Stacklet::suspend();
    }
}

struct Toggler : Stacklet {
    int num;

    Toggler (int n) : num (n) {}

    auto run () -> bool override {
        onOff(num, true);
        delay_ms(100);
        onOff(num, false);
        delay_ms(100 * (num+1));
        return true;
    }
};

char mem [10000];

int main() {
    enableSysTick();
    led.mode(Pinmode::out);
    Port<'A'>::modeMap(0b1111011, Pinmode::out);

    setup(mem, sizeof mem);

    // create 7 stacklets, with different delays
    for (int i = 0; i < 7; ++i)
        new Toggler (i);

    // minimal loop to keep stacklets going
    while (ready.size() > 0)
        ready.pull(0).asType<Stacklet>().resume();
}