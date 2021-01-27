extern "C" int printf(const char* fmt, ...);

namespace arch {
    void cliTask (void(*)(char const*));

    void init ();
    void idle ();
    auto done () -> int;
}
