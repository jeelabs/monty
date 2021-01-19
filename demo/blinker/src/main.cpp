#include <jee.h>
#include <cstdint>
#include <cstring>
#include <setjmp.h>

UartBufDev< PinA<2>, PinA<15> > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

PinB<3> led;

PinA<6> led1;
PinA<5> led2;
PinA<4> led3;
PinA<3> led4;
PinA<1> led5;
PinA<0> led6;

void (*toggler[])() = {
    led.toggle,
    led1.toggle,
    led2.toggle,
    led3.toggle,
    led4.toggle,
    led5.toggle,
    led6.toggle,
};

struct {
    int stack [100];
    jmp_buf top;
    void (*task)(int);
} stacks [7], *current;

jmp_buf* caller;

void suspend () {
    int callee [1];
    if (setjmp(current->top) == 0) {
        // suspending: copy stack out and jump to caller
        memcpy(current->stack, callee, (uintptr_t) caller - (uintptr_t) callee);
        longjmp(*caller, 1);
    } else {
        // resuming: copy stack in and prevent re-resuming
        memcpy(callee, current->stack, (uintptr_t) caller - (uintptr_t) callee);
        memset(current->top, 0, sizeof current->top);
#if 1
        asm ("nop"); // FIXME without this, the code doesn't work (???)
#endif
    }
}

void resume (int num) {
    current = &stacks[num];
    jmp_buf bottom;
    caller = &bottom;
    if (setjmp(bottom) == 0) {
        // either call for the first time, or resume saved state
        if (current->task != nullptr)
            current->task(num);
        else
            longjmp(current->top, 1);
    }
    // state has been saved, return to caller
    current->task = nullptr;
}

void delay_ms (uint16_t ms) {
    auto limit = ticks + ms;
    while (ticks < limit)
        suspend();
}

void toggleTask (int n) {
    while (true) {
        toggler[n]();
        delay_ms(100);
        toggler[n]();
        delay_ms(100 * (n+1));
    }
}

void demoPinToggle () {
    // set up 7 independent blink tasks
    for (int i = 0; i < 7; ++i)
        stacks[i].task = toggleTask;

    // run them for 100s
    for (int i = 0; i < 100000; ++i) {
        resume(i % 7);
        asm("wfi");
    }
}

int main() {
    console.init();
    console.baud(115200, fullSpeedClock());
    led.mode(Pinmode::out);
    Port<'A'>::modeMap(0b1111011, Pinmode::out);

    demoPinToggle();

    while (true) {}
}
