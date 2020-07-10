#include "config.h"
#include "../version.h"

extern void archInit ();
extern int archDone ();

extern int printf(const char*, ...);
extern "C" int debugf (const char*, ...);

extern const ModuleObj m_machine;
