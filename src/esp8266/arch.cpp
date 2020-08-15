#include "monty.h"
#include "arch.h"

using namespace Monty;

static Value bi_blah (Vector const& vec, int argc, int args) {
    return argc;
}

static const Function f_blah (bi_blah);

static const Lookup::Item lo_machine [] = {
    { "blah", &f_blah },
};

static const Lookup ma_machine (lo_machine, sizeof lo_machine);
const Module m_machine (&ma_machine);

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
