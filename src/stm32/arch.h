#include "config.h"

extern void archInit ();
extern void archIdle ();
extern int archDone ();

extern "C" int printf(const char*, ...);

extern Monty::Module const m_machine;
