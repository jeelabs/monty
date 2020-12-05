// This is for native use, see boot/core/devs for the segmented embedded builds.

#include <cstdio>
#include "monty.h"
#include "gc.h"

static uint8_t myMem [12*1024]; // tiny mem pool to stress the garbage collector

int main () {
    puts("NATIVE hello!");

    Monty::setup(myMem, sizeof myMem);
    Monty::gcReport();
}
