#include <monty.h>
#include "arch.h"

#include <cstdio>

using namespace monty;

auto arch::loadFile (char const* name) -> uint8_t const* {
    auto fp = fopen(name, "rb");
    if (fp == nullptr) {
        perror(name);
        return nullptr;
    }
    fseek(fp, 0, SEEK_END);
    uint32_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto data = (uint8_t*) malloc(bytes + 64);
    fread(data, 1, bytes, fp);
    fclose(fp);
    return data;
}

void arch::init () {
    setbuf(stdout, nullptr);
}

void arch::idle () {
    // nothing ...
}

int arch::done () {
    return 0;
}
