#include <cstdint>
#include "mrfs.h"

extern "C" int printf(char const* fmt, ...);

#if TEST
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

auto mapFlashToFile (char const* name, size_t size) -> uint8_t* {
    auto fd = open(name, O_CREAT|O_RDWR, 0666); assert(fd > 0);
    auto e = ftruncate(fd, size); assert(e == 0);
    auto ptr = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);
    //printf("flashBase %p\n", flashBase);
    return (uint8_t*) ptr;
}

constexpr auto flashSize = 64*1024;
uint8_t* flashBase;

int main (int argc, char const* argv[]) {
    assert(argc > 1);

    mrfs::base = mapFlashToFile("flash.img", flashSize);

    mrfs::init();

    if (strcmp(argv[1], "wipe") == 0)
        mrfs::wipe();
    else if (strcmp(argv[1], "dump") == 0)
        mrfs::dump();
    else
        assert(false);
}
#endif

void mrfs::init () {
    //printf("hello from %s\n", "mrfs");
}

void mrfs::wipe () {
    memset(base, 0xFF, flashSize);
}

void mrfs::dump () {
    printf("mrfs entries:\n");
}
