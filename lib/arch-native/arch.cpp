#include <monty.h>
#include "arch.h"

//CG1 if dir extend
#include <extend.h>

//CG2 if dir pyvm
#define HAS_PYVM 1
#include <pyvm.h>

#include <cassert>
#include <cstdio>
#include <ctime>

using namespace monty;

static void* pool;

#if HAS_PYVM

static auto loadFile (char const* name) -> uint8_t const* {
    auto fp = fopen(name, "rb");
    if (fp == nullptr)
        return nullptr;
    fseek(fp, 0, SEEK_END);
    uint32_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto data = (uint8_t*) malloc(bytes + 64);
    fread(data, 1, bytes, fp);
    fclose(fp);
    return data;
}

auto monty::vmImport (char const* name) -> uint8_t const* {
    auto data = loadFile(name);
    if (data != nullptr)
        return data;
    assert(strlen(name) < 25);
    char buf [40];
    sprintf(buf, "test/py/%s.mpy", name);
    return loadFile(buf);
}

#else
auto monty::vmLaunch (void const*) -> Stacklet* {
    return nullptr;
}
#endif

void arch::init (int size) {
    setbuf(stdout, nullptr);
    if (size <= 0)
        size = 1024*1024; // allocate a whopping 1 Mb
    pool = malloc(size);
    gcSetup(pool, size);
#if HAS_PYVM
    Event::triggers.append(0); // TODO yuck, reserve 1st entry for VM
#endif
}

void arch::idle () {
    timespec ts { 0, 100000 };
    nanosleep(&ts, &ts); // 100 µs, i.e. 10% of ticks' 1 ms resolution
}

static void cleanup () {
    Event::triggers.clear();
    Stacklet::ready.clear();
    Module::builtins.clear();
    Module::loaded.clear();
    qstrCleanup();
}

auto arch::done () -> int {
    cleanup();
    Stacklet::gcAll();
    //gcReport();
    //Object::dumpAll();
    //Vec::dumpAll();

    // TODO unfortunately, free() will fail if *any* vector still has non-zero
    // size, because then its destructor will try to clean up free'd memory
    //free(pool);

    return 0;
}
