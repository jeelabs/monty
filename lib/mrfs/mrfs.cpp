#include <cstdlib>
#include "mrfs.h"

extern "C" int printf(char const* fmt, ...);

#if TEST
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

auto mapFlashToFile (char const* name, size_t size) -> void* {
    auto fd = open(name, O_CREAT|O_RDWR, 0666); assert(fd > 0);
    auto e = ftruncate(fd, size); assert(e == 0);
    auto ptr = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);
    //printf("mmap %p\n", ptr);
    return ptr;
}

void add (int ac, char const** av) {
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

void find (int ac, char const** av) {
    assert(ac == 1);
    printf("%d\n", mrfs::find(av[0]));
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
        printf("%05d:%6d  20%06u.%04u  %s\n",
                (int) (p - base), p->size,
                p->tail()->time/10000, p->tail()->time%10000, p->tail()->name);
        p = p->tail() + 1;
    }
}

auto mrfs::add(char const* name, uint32_t time,
                void const* buf, uint32_t len) -> int {
    Info info {
        .magic = MAGIC,
        .time = time,
        .zero = 0,
        .size = len,
        .flags = 0,
        .crc = 0x41424344,
    };
    strncpy(info.name, name, sizeof info.name);
    auto rounded = len + (-len & 31);

    auto p = (uint8_t*) next;
    memcpy(p, &info, 8);
    memcpy(p+8, buf, len);
    memset(p+8+len, 0xFF, rounded-len);
    memcpy(p+8+rounded, info.name, 24);

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
