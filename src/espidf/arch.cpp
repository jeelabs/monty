#include <cstdint>
#include <cstdlib>
#include <cstring>

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
