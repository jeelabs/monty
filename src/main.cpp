// This is for native use, see boot/core/devs for the layered embedded builds.

#include <cstdio>
#include "monty.h"

static uint8_t myMem [64*1024];

int main () {
    puts("NATIVE hello!");

    monty::setup(myMem, sizeof myMem);
    monty::gcReport();
}
