#include "config.h"

extern void archInit ();
extern auto archTime () -> uint32_t;
extern void archIdle ();
extern auto archDone (char const* =nullptr) -> int;

enum class RunMode { // mapped to blue, red, orange, green LEDs
    Run = 0b0001, GC = 0b0010, Idle = 0b1000, Sleep = 0b0000, Done = 0b0100,
};
extern void archMode (RunMode); // show status on LEDs, if available

extern "C" int printf (char const*, ...);

extern Monty::Module const m_machine;
