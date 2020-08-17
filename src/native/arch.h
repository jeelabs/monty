#include "config.h"
#include "../version.h"

#include <stdio.h>

extern void archInit ();
extern void archIdle ();
extern int archDone ();

extern "C" int debugf (const char*, ...);

#define INNER_HOOK  { timerHook(); }
extern void timerHook ();

extern const Monty::Module m_machine;
