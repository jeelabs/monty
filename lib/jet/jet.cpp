#include "monty.h"
#include "jet.h"
#include <cassert>

using namespace monty;
using namespace jet;

void Gadget::marker () const {
    // ...
}

Flow::Flow () : _fanout ('N'), _wires ('H'), _index ('H') {
    printf("Flow %d b\n", (int) sizeof *this);
}

void Flow::marker () const {
    mark(_fanout);
    mark(_wires);
    mark(_index);
    markVec(_state);
}

Type Flow::info ("jet.flow");

static auto f_flow (ArgVec const& args) -> Value {
    assert(args.num == 0);
    return new Flow;
}

static Lookup::Item const lo_jet [] = {
    { "flow", f_flow },
};

static Lookup const ma_jet (lo_jet, sizeof lo_jet);
extern Module m_jet (ma_jet);
