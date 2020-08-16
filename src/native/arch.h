#include "config.h"
#include "../version.h"

#include <stdio.h>

extern void archInit ();
extern int archDone ();

extern "C" int debugf (const char*, ...);

#define INNER_HOOK  { extern void timerHook (); timerHook(); }

extern const Monty::Module m_machine;
