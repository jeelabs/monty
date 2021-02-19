#include "mrfs.h" // this include is needed so PIO will find lib/mrfs/

namespace arch {
    auto cliTask () -> monty::Stacklet*;

    void init (int =0);
    void idle ();
    auto done () -> int;
}
