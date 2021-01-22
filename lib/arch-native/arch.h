extern "C" int printf(const char* fmt, ...);

namespace arch {
    void init ();
    void idle ();
    int done ();
}
