#include "monty.h"
#include "arch.h"

static Value bi_blah (int argc, Value argv []) {
    return argc;
}

static const FunObj f_blah (bi_blah);

static const LookupObj::Item lo_machine [] = {
    { "blah", &f_blah },
};

static const LookupObj ma_machine (lo_machine, sizeof lo_machine / sizeof *lo_machine);
const ModuleObj m_machine (&ma_machine);
