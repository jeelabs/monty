extern "C" int printf(const char* fmt, ...);

namespace arch {
    auto loadFile (char const*) -> uint8_t const*;
    auto cliTask (void(*)(char const*)) -> monty::Stacklet*;

    void init ();
    void idle ();
    auto done () -> int;
}
