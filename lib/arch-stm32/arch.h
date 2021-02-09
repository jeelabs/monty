#include "mrfs.h" // this include is needed so PIO will find lib/mrfs/

extern "C" int printf(const char* fmt, ...);

namespace arch {
    using Loader = auto (*)(void const*)->monty::Stacklet*;
    auto cliTask (Loader) -> monty::Stacklet*;
    auto importer (char const* name) -> uint8_t const*;

    void init ();
    void idle ();
    auto done () -> int;
}
