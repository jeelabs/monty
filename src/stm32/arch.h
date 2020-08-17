#include "config.h"

extern void archInit ();
extern void archIdle ();
extern auto archDone (char const* =nullptr) -> int;

extern "C" int printf (char const*, ...);

extern Monty::Module const m_machine;
