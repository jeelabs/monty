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

    auto info = mrfs::add(name, time, buf, size);
    printf("%p\n", info);

    free(buf);
}

int main (int argc, char const* argv[]) {
    if (argc < 2) {
        printf("usage: %s cmd args...\n", argv[0]);
        return 0;
    }

    mrfs::start = mapFlashToFile("flash.img", flashSize);
    mrfs::limit = mrfs::start + flashSize / sizeof (mrfs::Info);

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
}

void mrfs::dump () {
    printf("mrfs entries:\n");
    if (start->isValid())
        printf("0x%06x:%6d  20%10u  %s\n",
                0, start->size, start->time, start->name);
}

auto mrfs::add(char const* name, uint32_t time,
                void const* buf, uint32_t len) -> Info const* {
    Info info {
        .magic = MAGIC,
        .time = time,
        .size = len,
        .flags = 0,
    };
    strncpy(info.name, name, sizeof info.name);

    auto p = (uint8_t*) start;
    memcpy(p, &info, 8);
    memcpy(p+8, buf, len);
    memcpy(p+len+(-len&31)-24, info.name, 24);
    return start;
}
