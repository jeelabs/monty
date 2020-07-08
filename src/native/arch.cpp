#include "monty.h"
#include "arch.h"

#include <stdarg.h>
#include <time.h>

int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    return 0;
}

void archInit () {
    setbuf(stdout, 0);    
}

int archDone () {
    Object::gcStats();
    return 0;
}

static Value bi_blah (int argc, Value argv []) {
    return argc;
}

static const FunObj f_blah (bi_blah);

static int ms, id;
static uint32_t start, begin, last = ~0;

static uint32_t getTime () {
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return tv.tv_sec * 1000 + tv.tv_nsec / 1000000; // ms resolution
}

void timerHook () {
    uint32_t t = getTime();
    if (ms > 0 && (t - start) / ms != last) {
        last = (t - start) / ms;
        if (id > 0)
            Context::raise(id);
    }
}

// interface exposed to the VM

static Value f_timer (int argc, Value argv []) {
    if (argc != 3 || !argv[1].isInt())
        return -1;
    ms = argv[1];
    id = Context::setHandler(ms > 0 ? argv[2] : (Value) id);
    start = getTime(); // set first timeout relative to now
    last = 0;
    return id;
}

static Value f_ticks (int argc, Value argv []) {
    uint32_t t = getTime();
    if (begin == 0)
        begin = t;
    return t - begin; // make all runs start out the same way
}

static const FunObj fo_timer (f_timer);
static const FunObj fo_ticks (f_ticks);

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
    { "timer", &fo_timer },
    { "ticks", &fo_ticks },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);
