#include <jee.h>

UartBufDev< PinA<9>, PinA<10>, 2, 99 > console;

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"

#include "mod_monty.h"

#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"

void Context::print (Value v) {
    switch (v.tag()) {
        case Value::Nil: printf("<nil>"); break;
        case Value::Int: printf("<Int %d>", (int) v); break;
        case Value::Str: printf("<Str '%s' at %p>",
                                 (const char*) v, (const char*) v); break;
        case Value::Obj: printf("<Obj %s at %p>",
                                 v.obj().type().name, &v.obj()); break;
    }
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

#if !UNIT_TEST
int main () {
    console.init();
    console.baud(115200, fullSpeedClock());
    wait_ms(500);
    printf("main qstr #%d %db\n", (int) qstrNext, (int) sizeof qstrData);

    auto bcData = (const uint8_t*) 0x20004000;
    if (!runInterp(bcData))
        printf("can't load bytecode\n");
    else
        printf("done\n");
    while (true) {}
}
#endif
