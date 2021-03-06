// qstr.cpp - set of "quick strings" from MicroPython 1.14 plus own entries

#include "monty.h"

using namespace monty;

//CG: mod-list d

static Lookup::Item const mod_map [] = {
//CG: mod-list a
};

static Lookup const mod_attrs (mod_map);
Dict Module::loaded (&mod_attrs);

extern char const monty::qstrBase [] =
//CG: qstr-emit
;

int const monty::qstrBaseLen = sizeof qstrBase;
