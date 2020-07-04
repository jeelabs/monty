// ToyPy main application, this is a test shell for the Monty VM.

#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#if NATIVE
#include <stdio.h>
static void initBoard () { setbuf(stdout, 0); }
static int deinitBoard (bool ok) { return ok ? 0 : 1; }
#else
#include <jee.h>

#if STM32F1
UartBufDev< PinA<9>, PinA<10>, 2, 99 > console; // Blue Pill
#elif STM32F4
UartBufDev< PinA<2>, PinA<3>, 2 > console;      // F4 Discovery
#else
UartBufDev< PinA<2>, PinA<3> > console;         // ESP32 TinyPico
#endif

int printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    veprintf(console.putc, fmt, ap); va_end(ap);
    return 0;
}

static void initBoard () {
    console.init();
#if STM32F1
    console.baud(115200, fullSpeedClock());     // Blue Pill
    (void) powerDown; (void) enableClkAt8MHz;   // suppress warnings
#else
    console.baud(115200, fullSpeedClock()/4);   // F4 Discovery
#endif
}

static int deinitBoard (bool ok) {
    while (true) {
#ifndef ESP_PLATFORM
        asm("wfi");
#endif
    }
}
#endif

#include "monty.h"

#include <assert.h>
#include <string.h>

#if NATIVE
#define INNER_HOOK  { timerHook(); }
#endif

#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"
#include "util.h"

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

#if !RAMLOAD
static const uint8_t* loadBytecode (const char* fname) {
    FILE* fp = fopen(fname, "rb");
    if (fp == 0)
        return 0;
    fseek(fp, 0, SEEK_END);
    size_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //printf("bytecode size %db\n", (int) bytes);
    auto buf = (uint8_t*) malloc(bytes);
    auto len = fread(buf, 1, bytes, fp);
    fclose(fp);
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}
#endif

#if ESP_PLATFORM
int main () {
    int argc = 1;
    const char* argv [] = { "" };
    vTaskDelay(3000/10); // 3s delay, enough time to attach serial
#else
int main (int argc, const char* argv []) {
#endif
    initBoard();
    printf(
#if !NATIVE
            "\xFF" // send out special marker for easier remote output capture
#endif
            "main qstr #%d %db\n", (int) qstrNext, (int) sizeof qstrData);

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

#if RAMLOAD
    auto bcData = (const uint8_t*) RAMLOAD;
#else
    auto bcData = loadBytecode(argc == 2 ? argv[1] : "demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return deinitBoard (false);
    }
#endif

    if (!runInterp(bcData)) {
        printf("can't load module\n");
        return deinitBoard (false);
    }

#if NATIVE
    free((void*) bcData);
#endif

    printf("done\n");
    Object::gcStats();
    return deinitBoard(true);
}
