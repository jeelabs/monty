#include "monty.h"
#include "jet.h"

using namespace monty;
using namespace jet;

void Gadget::marker () const {
    // ...
}

Flow::Flow () : _fanout ('N'), _wires ('H'), _index ('H') {
    //printf("Flow %d b\n", (int) sizeof *this);
}

void Flow::marker () const {
    mark(_fanout);
    mark(_wires);
    mark(_index);
    markVec(_state);
    Stacklet::marker();
}
