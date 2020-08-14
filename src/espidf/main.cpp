// ToyPy main application, this is a test shell for the Monty VM.

#define SHOW_INSTR_PTR  0 // show instr ptr each time through loop (interp.h)
#define VERBOSE_LOAD    0 // show .mpy load progress with detailed file info

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "monty.h"
#include "arch.h"

#include <cstdarg>
#include <cstdio>

extern "C" int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    return 0;
}

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

extern "C" int app_main () {
    vTaskDelay(3000/10); // 3s delay, enough time to attach serial
    printf("\xFF" // send out special marker for easier remote output capture
           "main\n");

    //showAlignment();      // show string address details in flash and ram
    //showAllocInfo();      // show mem allocator behaviour for small allocs
    //showObjSizes();       // show sizeof information for the main classes

    auto bcData = loadBytecode("demo.mpy");
    if (bcData == 0) {
        printf("can't load bytecode\n");
        return 1;
    }

    static uintptr_t myMem [4096];
    Monty::setup(myMem);
    (void) Monty::loadModule(bcData);

    free((void*) bcData);

    printf("done\n");
    //Object::gcStats();
    return 0;
}
