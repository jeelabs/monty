#include "config.h"

extern void archInit ();
extern void archIdle ();
extern auto archDone (char const* =nullptr) -> int;

#define INNER_HOOK  { timerHook(); }
extern void timerHook ();

extern Monty::Module const m_machine;
