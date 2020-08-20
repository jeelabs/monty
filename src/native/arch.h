#include "config.h"

extern void archInit ();
extern void archIdle ();
extern auto archDone (char const* =nullptr) -> int;

enum class RunMode { Run, GC, Idle, Sleep, Done, };
extern void archMode (RunMode); // show status on LEDs, if available

#define INNER_HOOK  { timerHook(); }
extern void timerHook ();

extern Monty::Module const m_machine;
