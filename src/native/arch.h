#include <stdio.h>

#include "../version.h"

#define INNER_HOOK  { timerHook(); }

extern void archInit ();
extern int archDone ();

extern "C" int debugf (const char*, ...);

extern const ModuleObj m_machine;
