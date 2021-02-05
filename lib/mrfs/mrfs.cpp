#include <cstdint>
#include <cstring>
#include "mrfs.h"

namespace mrfs {
    Info* base;
    int skip;
    Info* next;
    Info* last;
}

#if TEST

#include <cassert>
#include <cstdlib>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

static auto mapFlashToFile (char const* name, size_t size) -> void* {
    auto fd = open(name, O_CREAT|O_RDWR, 0666); assert(fd > 0);
    auto e = ftruncate(fd, size); assert(e == 0);
    auto ptr = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);
    //printf("mmap %p\n", ptr);
    return ptr;
}

static void add (int ac, char const** av) {
    assert(ac == 4);
    char const* name = av[0];
    uint32_t time = strtoul(av[1], 0, 10);
    uint32_t size = strtoul(av[2], 0, 10);
    char const* data = av[3];

    auto buf = (uint8_t*) malloc(size); assert(buf != nullptr);
    int len = strlen(data); if (len == 0) ++len;

    for (uint32_t i = 0; i < size; ++i)
        buf[i] = data[i%len];

    auto pos = mrfs::add(name, time, buf, size);
    printf("%d\n", pos);

    free(buf);
}

static void find (int ac, char const** av) {
    assert(ac == 1);
    printf("%d\n", mrfs::find(av[0]));
}

static void saveToFlash (void* addr, mrfs::Info& info, void const* buf) {
    auto rounded = info.size + (-info.size & 31);
    auto p = (uint8_t*) addr;
    memcpy(p, &info, 8);
    memcpy(p+8, buf, info.size);
    memset(p+8+info.size, 0xFF, rounded-info.size);
    memcpy(p+8+rounded, info.name, 24);
}

int main (int argc, char const* argv[]) {
    if (argc < 2) {
        printf("usage: %s cmd args...\n", argv[0]);
        return 0;
    }

    constexpr auto flashSize = 64*1024;
    mrfs::init(mapFlashToFile("flash.img", flashSize), flashSize);

         if (strcmp(argv[1], "wipe") == 0) mrfs::wipe();
    else if (strcmp(argv[1], "dump") == 0) mrfs::dump();
    else if (strcmp(argv[1], "add")  == 0) add(argc-2, argv+2);
    else if (strcmp(argv[1], "find") == 0) find(argc-2, argv+2);
    else assert(false);
}

#else

#include <jee.h>

static void saveToFlash (uint8_t* addr, mrfs::Info& info, void const* buf) {
    // STM32L4-specific code
    auto start = (uint32_t*) addr;
    auto limit = (uint32_t*) (addr + (info.tail()+1-&info) * sizeof info);
    // store first 8 bytes from info, then buf, then last 24 bytes from info
    auto src = (uint32_t const*) &info;
    for (auto dst = start; dst < limit; dst += 2, src += 2) {
        if (dst == start + 2)
            src = (uint32_t const*) buf;
        if (dst == limit - 6)
            src = (uint32_t const*) info.name;
        if ((uint32_t) dst % 2048 == 0)
            Flash::erasePage(dst);
#if STM32L4
        Flash::write64(dst, src[0], src[1]);
#else
        Flash::write32(dst, src[0]);
        Flash::write32(dst+1, src[1]);
#endif
    }
    Flash::finish();
}

#endif

void mrfs::init (void* ptr, size_t len, size_t keep) {
    base = (Info*) ptr;
    skip = keep / sizeof (Info);
    last = base + len / sizeof (Info);

    next = base + skip;
    while (next < last && next->valid())
        next = next->tail() + 1;
}

void mrfs::wipe () {
    memset(base, 0xFF, (uint8_t*) last - (uint8_t*) base);
    next = base + skip;
}

void mrfs::dump () {
    auto p = base + skip;
    while (p < last && p->valid()) {
        printf("%04d:%6d  %6d.%04d  %s\n",
                (int) (p - base), p->size,
                p->tail()->time/10000, p->tail()->time%10000, p->tail()->name);
        p = p->tail() + 1;
    }
}

auto mrfs::add(char const* name, uint32_t time,
                void const* buf, uint32_t len) -> int {
    Info info = { MAGIC, len, 0, {}, 0, time, 0x41424344 };
    strncpy(info.name, name, sizeof info.name);

    saveToFlash((uint8_t*) next, info, buf);

    int pos = next - base;
    next = next->tail() + 1;
    return pos;
}

auto mrfs::find (char const* name) -> int {
    int n = -1;
    auto p = base + skip;
    while (p < last && p->valid()) {
        if (strcmp(name, p->tail()->name) == 0)
            n = p->time != 0 ? p - base : -1;
        p = p->tail() + 1;
    }
    return n;
}
