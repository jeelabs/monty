#include "monty.h"
#include "arch.h"

#include <stdarg.h>

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

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);
