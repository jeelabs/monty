extern "C" int printf(const char* fmt, ...);

namespace arch {
    auto cliTask (auto(*)(char const*)->bool) -> monty::Stacklet*;

    void init ();
    void idle ();
    auto done () -> int;
}
