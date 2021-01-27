extern "C" int printf(const char* fmt, ...);

namespace arch {
    auto loadFile (char const*) -> uint8_t const*;
    void cliTask (void(*)(char const*));

    void init ();
    void idle ();
    auto done () -> int;
}
