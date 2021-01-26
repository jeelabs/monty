extern "C" int printf(const char* fmt, ...);

namespace arch {
    auto loadFile (char const*) -> uint8_t const*;

    void init ();
    void idle ();
    int done ();
}
