extern "C" int printf(const char* fmt, ...);

namespace arch {
    using Loader = auto (*)(uint8_t const*)->monty::Stacklet*;
    auto cliTask (Loader) -> monty::Stacklet*;

    void init ();
    void idle ();
    auto done () -> int;
}
