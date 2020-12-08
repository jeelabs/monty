#include "mkfs.h"

extern "C" int printf(char const* fmt, ...);

#if TEST
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

char const* flashImage = "flash.img";
constexpr auto flashSize = 64*1024;
uint8_t* flashBase;

int main (int argc, char const* argv[]) {
    assert(argc > 1);

    auto fd = open(flashImage, O_CREAT|O_RDWR, 0666); assert(fd > 0);
    auto e = ftruncate(fd, flashSize); assert(e == 0);
    flashBase = (uint8_t*)
        mmap(nullptr, flashSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    printf("flashBase %p\n", flashBase);

    mkfs::init();

    if (strcmp(argv[1], "wipe") == 0) {
        mkfs::wipe();
    } else
        assert(false);
}
#endif

void mkfs::init () {
    printf("hello from %s\n", "mkfs");
}

void mkfs::wipe () {
    memset(flashBase, 0xFF, flashSize);
}
