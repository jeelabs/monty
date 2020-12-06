// This is for native use, see boot/core/devs for the segmented embedded builds.

#include <cstdio>
#include "monty.h"
#include "gc.h"

static uint8_t myMem [64*1024];

int main () {
    puts("NATIVE hello!");

    Monty::setup(myMem, sizeof myMem);
    Monty::gcReport();
}
