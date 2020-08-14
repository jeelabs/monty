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

extern "C" uint32_t __atomic_fetch_or_4 (void volatile* p, uint32_t v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (uint32_t volatile*) p;
    // atomic start
    auto t = *q;
    *q |= v;
    // atomic end
    return t;
}

extern "C" uint32_t __atomic_fetch_nand_4 (void volatile* p, uint32_t v, int o) {
    // see https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    // FIXME this version is not atomic!
    auto q = (uint32_t volatile*) p;
    // atomic start
    auto t = *q;
    *q &= ~v;
    // atomic end
    return t;
}
