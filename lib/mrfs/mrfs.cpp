#include <cstdlib>
#include "mrfs.h"

static_assert(sizeof (mrfs::Info) == 32, "incorrect header size");

extern "C" int printf(char const* fmt, ...);

#if TEST
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

auto mapFlashToFile (char const* name, size_t size) -> mrfs::Info* {
    auto fd = open(name, O_CREAT|O_RDWR, 0666); assert(fd > 0);
    auto e = ftruncate(fd, size); assert(e == 0);
    auto ptr = mmap(nullptr, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != MAP_FAILED);
    //printf("mmap %p\n", ptr);
    return (mrfs::Info*) ptr;
}

constexpr auto flashSize = 64*1024;

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

int main (int argc, char const* argv[]) {
    if (argc < 2) {
        printf("usage: %s cmd args...\n", argv[0]);
        return 0;
    }

    mrfs::start = mrfs::next = mapFlashToFile("flash.img", flashSize);
    mrfs::limit = mrfs::start + flashSize / sizeof (mrfs::Info);

    while (mrfs::next->isValid())
        mrfs::next += 1 + (mrfs::next->size+31)/32;

    mrfs::init();

         if (strcmp(argv[1], "wipe") == 0) mrfs::wipe();
    else if (strcmp(argv[1], "dump") == 0) mrfs::dump();
    else if (strcmp(argv[1], "add")  == 0) add(argc-2, argv+2);
    else assert(false);
}
#endif

void mrfs::init () {
    //printf("hello from %s\n", "mrfs");
}

void mrfs::wipe () {
    memset(start, 0xFF, (uint8_t*) limit - (uint8_t*) start);
    next = start;
}

void mrfs::dump () {
    auto p = start;
    while (p->isValid()) {
        auto tail = p + (p->size+31)/32;
        printf("%05d:%6d  20%10u  %s\n",
                (int) (p - start), p->size, tail->time, tail->name);
        p = tail + 1;
    }
}

auto mrfs::add(char const* name, uint32_t time,
                void const* buf, uint32_t len) -> int {
    Info info {
        .magic = MAGIC,
        .time = time,
        .size = len,
        .flags = 0,
        .crc = 0x41424344,
    };
    auto n = sizeof info.name - 1;
    strncpy(info.name, name, n);
    info.name[n] = 0;
    auto rounded = len + (-len & 31);

    auto p = (uint8_t*) next;
    memcpy(p, &info, 8);
    memcpy(p+8, buf, len);
    memset(p+8+len, 0xFF, rounded-len);
    memcpy(p+8+rounded, info.name, 24);

    int pos = next - start;
    next += 1 + rounded / sizeof info;
    return pos;
}
