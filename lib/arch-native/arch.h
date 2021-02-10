extern "C" int printf(const char* fmt, ...);

namespace arch {
    auto loadFile (char const* name) -> uint8_t const*;
    auto importer (char const* name) -> uint8_t const*;

    void init ();
    void idle ();
    auto done () -> int;
}
