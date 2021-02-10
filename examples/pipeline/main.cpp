#include "monty.h"
#include "arch.h"

using namespace monty;

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::loadFile(name);
}

struct Drain : Stacklet {
    Value feed;

    Drain (Value src) : feed (src) {}

    auto run () -> bool override {
        auto v = feed.asObj().next();
        if (v.isNil()) {
            current = nullptr;
            return false;
        }
        if (v.truthy())
            v.dump("pipe?");
        return true;
    }
};

struct ArgRunner : Object {
    int i = 0;
    int ac;
    char const** av;

    ArgRunner (int argc, char const** argv) : ac (argc), av (argv) {}

    auto next  () -> Value override {
        if (i < ac)
            return av[i++];
        return {};
    }
};

struct FileRunner : Object {
    Value feed;

    FileRunner (Value src) : feed (src) {}

    auto next  () -> Value override {
        Value v = feed.asObj().next();
        if (v.isStr()) {
            auto data = arch::loadFile(v);
            if (data != nullptr)
                v = new Bytes (data, 2000); // FIXME, length is wrong
        }
        return v;
    }
};

struct PyRunner : Object {
    Value feed;

    PyRunner (Value src) : feed (src) {}

    auto next  () -> Value override {
        Value v = feed.asObj().next();
        if (v.ifType<Bytes>() != nullptr) {
            auto task = vmLaunch(v.asType<Bytes>().begin());
            if (task != nullptr) {
                Stacklet::tasks.append(task);
                return 0;
            }
        }
        return v;
    }
};

struct Command {
    char const* desc;
    void (*proc)();
};

static void printBuildVer () {
    printf("Monty " VERSION " (" __DATE__ ", " __TIME__ ")\n");
}

static void helpCmd ();

static Command const commands [] = {
    { "bv    show build version"          , printBuildVer },
    { "gc    trigger garbage collection"  , Stacklet::gcAll },
    { "gr    generate a GC report"        , gcReport },
    { "od    object dump"                 , gcObjDump },
    { "vd    vector dump"                 , gcVecDump },
    { "-h    this help"                   , helpCmd },
};

void helpCmd () {
    for (auto& cmd : commands)
        printf("  %s\n", cmd.desc);
}

struct CmdRunner : Object {
    Value feed;

    CmdRunner (Value src) : feed (src) {}

    auto next  () -> Value override {
        Value v = feed.asObj().next();
        if (v.isStr()) {
            char const* buf = v;
            for (auto& cmd : commands)
                if (memcmp(buf, cmd.desc, 2) == 0) {
                    cmd.proc();
                    return 0;
                }
        }
        return v;
    }
};

int main (int argc, char const** argv) {
    static uint8_t memPool [10*1024];
    gcSetup(memPool, sizeof memPool);

    Event::triggers.append(0); // TODO get rid of this

    ArgRunner  args (argc-1, argv+1);
    FileRunner file (args);
    PyRunner   py   (file);
    CmdRunner  cmd  (py);
    Drain      pipe (cmd);

    Stacklet::tasks.append(&pipe);

    while (Stacklet::runLoop()) {}

    return 0;
}
