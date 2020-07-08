#include <stdio.h>

#include "../version.h"

extern void archInit ();
extern int archDone ();

extern "C" int debugf (const char*, ...);

#define INNER_HOOK  { timerHook(); }
extern void timerHook ();

extern const ModuleObj m_machine;
