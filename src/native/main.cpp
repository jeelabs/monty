#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include <cstdio>

//static uint8_t myMem [12*1024]; // tiny mem pool to stress the garbage collector
static uint8_t myMem [128*1024];

static auto compileFile (const char* fname) -> char const* {
    auto buf = (char*) malloc(20 + strlen(fname));
    if (system(strcat(strcpy(buf, "mpy-cross "), fname)) != 0)
        return nullptr;
    strcpy(strcpy(buf, fname) + strlen(fname) - 3, ".mpy");
    return buf;
}

static auto loadFile (const char* fname) -> const uint8_t* {
    FILE* fp = fopen(fname, "rb");
    if (fp == 0)
        return 0;
    fseek(fp, 0, SEEK_END);
    uint32_t bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //printf("%s: %db\n", fname, (int) bytes);
    auto buf = (uint8_t*) malloc(bytes + 64);
    auto len = fread(buf, 1, bytes, fp);
    memset(buf + bytes, 0xFF, 64); // simulate empty flash at end
    fclose(fp);
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}

static void runInterp (Monty::Callable& init) {
    Monty::PyVM vm (init);

    while (vm.isAlive()) {
        vm.scheduler();
        archIdle();
    }

    // nothing left to do or wait for, at this point
}

static auto mpy2mty (char const* fn) -> char const* {
    auto buf = strdup(fn);
    auto dot = strrchr(buf, '.');
    if (dot == nullptr || strcmp(dot, ".mpy") != 0) {
        printf("%s: ", fn);
        return "not a .mpy file";
    }

    auto bcData = loadFile(buf);
    if (bcData == nullptr)
        return "can't load bytecode";

    auto vvCode = Monty::converter(bcData);
    if (vvCode == nullptr)
        return "can't load module";

    strcpy(dot, ".mty");
    printf("%s: ", buf);
    FILE* fp = fopen(buf, "wb");
    if (fp == nullptr)
        return "can't write .mty file";
    fwrite(vvCode->first(), 1, vvCode->limit() - vvCode->first(), fp);
    printf("%db\n", (int) ftell(fp));
    fclose(fp);

    free((void*) bcData);
    free(buf);
    return nullptr;
}

int main (int argc, const char* argv []) {
    archInit();

    // the loader uses garbage-collected memory
    Monty::setup(myMem, sizeof myMem);

    // special convert mode to generate new-format .mty files
    if (argc >= 3 && strcmp(argv[1], "-c") == 0) {
        for (int i = 2; i < argc; ++i) {
            auto msg = mpy2mty (argv[i]);
            if (msg != nullptr)
                archDone(msg);
        }
        return archDone();
    }

    // load simulated "rom" from "file"
    Monty::fsBase = loadFile(argc >= 3 ? argv[2] : "rom.mrfs");

    // name of the bytecode to run
    auto fname = argc >= 2 ? argv[1] : "demo.py";
    if (strlen(fname) > 3 && strcmp(fname + strlen(fname) - 3, ".py") == 0) {
        fname = compileFile(fname);
        if (fname == nullptr)
            return archDone("can't compile source file");
    }
    auto bcData = loadFile(fname);

    auto bc = bcData;
    if (bc == nullptr && argc >= 3) // no such file, try mrfs
        bc = Monty::fsLookup(argv[1]);
    if (bc == nullptr) {
        perror(fname);
        return archDone("can't load bytecode");
    }

    // construct in-memory bytecode objects and sub-objects
    auto init = Monty::loader("__main__", bc);
    if (init == nullptr)
        return archDone("can't load module");

    // orignal bytecode file is no longer needed
    free((void*) bcData);

    // go!
    runInterp(*init);

    // simulated rom data no longer needed
    free((void*) Monty::fsBase);
    //Monty::gcReport(true);
    return archDone();
}
