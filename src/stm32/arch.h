#include "config.h"
#include "../version.h"

extern void archInit ();
extern void archIdle ();
extern int archDone ();

extern "C" int printf(const char*, ...);
extern "C" int debugf (const char*, ...);

extern const Monty::Module m_machine;
