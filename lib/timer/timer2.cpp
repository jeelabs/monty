// Monty add-on to implement a periodic timer (uses SysTick on ARM µCs).

#include <monty.h>
#include "timer.h"

static int ms, id;
static uint32_t start, begin, last = ~0;

#if NATIVE
#include <time.h>

// need to #define INNER_HOOK in the VM, as there are no h/w systicks, e.g.
//  #define INNER_HOOK  { timerHook(); }
static void hookTimerIRQ () {}

static uint32_t getTime () {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

#elif ESP_PLATFORM
static void hookTimerIRQ () {} // TODO
static uint32_t getTime () { return 0; }

#elif ESP8266
#include <Arduino.h>
static void hookTimerIRQ () {} // TODO
static uint32_t getTime () { return millis(); }

#else // embedded version, currently based on JeeH
#include <jee.h>

static void hookTimerIRQ () { VTableRam().systick = timerHook; }
static uint32_t getTime () { return ticks; }

#endif

void timerHook () {
    uint32_t t = getTime();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Context::raise(id);
    }
}

// interface exposed to the VM

Value xSetTimer (int argc, Value argv []) {
    if (argc != 2 || !argv[0].isInt())
        return -1;
    ms = argv[0];
    id = Context::setHandler(ms > 0 ? argv[1] : (Value) id);
    start = getTime(); // set first timeout relative to now
    last = 0;
    hookTimerIRQ();
    return id;
}

Value xGetTime (int argc, Value argv []) {
    uint32_t t = getTime();
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}
