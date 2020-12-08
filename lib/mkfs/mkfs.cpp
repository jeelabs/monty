#include "mkfs.h"

extern "C" int printf(char const* fmt, ...);

using namespace mkfs;

void mkfs::init () {
    printf("hello from %s\n", "mkfs");
}

#if MAIN
int main (int argc, char const* argv[]) {
    mkfs::init();
}
#endif
