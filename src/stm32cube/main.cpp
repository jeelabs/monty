#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"
#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"

#include <jee.h>

UartBufDev< PinA<9>, PinA<10>, 2, 99 > console;

int printf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

extern "C" int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

static bool runInterp (const uint8_t* data) {
    Interp vm;

    ModuleObj* mainMod = 0;
    if (data[0] == 'M' && data[1] == 5) {
        Loader loader;
        mainMod = loader.load (data);
        vm.qPool = loader.qPool;
    }

    if (mainMod == 0)
        return false;

    mainMod->chain = &builtinDict;
    mainMod->atKey("__name__", DictObj::Set) = "__main__";
    mainMod->call(0, 0);

    vm.run();

    // must be placed here, before the vm destructor is called
    Object::gcStats();
    Context::gcTrigger();
    return true;
}

#include "net.h"

int main () {
    console.init();
    console.baud(115200, fullSpeedClock());
    wait_ms(500);

    printf("\xFF" // send out special marker for easier remote output capture
           "main qstr #%d %db %s\n",
            (int) qstrNext, (int) sizeof qstrData, VERSION);

    mch_net_init();
    printf("Setup completed\n");
    while (1) {
        mch_net_poll();
        sys_check_timeouts();
    }

    auto bcData = (const uint8_t*) 0x20004000;
    if (!runInterp(bcData))
        printf("can't load bytecode\n");

    printf("done\n");
    while (true) {}
}
