#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include <LittleFS.h>

static uintptr_t myMem [4096];

static const uint8_t* loadBytecode (const char* fname) {
    File f = LittleFS.open(fname, "r");
    if (!f)
        return 0;
    size_t bytes = f.size();
    printf("bytecode size %db\n", (int) bytes);
    auto buf = (uint8_t*) malloc(bytes);
    auto len = f.read(buf, bytes);
    f.close();
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}

static void runInterp (Monty::Callable& init) {
    Monty::PyVM vm (init);

    while (vm.isAlive()) {
        vm.runner();
        //archIdle();
    }

    // nothing left to do or wait for, at this point
}

void setup () {
    Serial.begin(115200);
    printf("main\n");

    if (LittleFS.begin())
        printf("LittleFS mounted\n");

    const char* fname = "/demo.mpy";
    auto bcData = loadBytecode(fname);
    if (bcData == 0) {
        printf("can't open %s\n", fname);
        perror(fname);
        return;
    }

    Monty::setup(myMem, sizeof myMem);

    auto init = Monty::importer("__main__", bcData);
    if (init == nullptr) {
        printf("can't load module\n");
        return;
    }

    runInterp(*init);

    printf("done\n");
}

void loop () {}
