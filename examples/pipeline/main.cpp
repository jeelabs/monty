#include <monty.h>
#include <arch.h>

#include <cassert>

using namespace monty;

struct Pipeline : Stacklet {
    Value feed;

    Pipeline (Value src) : feed (src) {}

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

    void marker () const override { feed.marker(); }
};

struct Runner : Object {
    static Type info;
    auto type () const -> Type const& override { return info; }

    using Func = auto (*)(Runner*) -> Value;
    Func func;
    Runner* feed;

    Runner (Func f =nullptr, Runner* in =nullptr) : func (f), feed (in) {}

    auto next  () -> Value override {
        return func(feed);
    }

    void marker () const override { mark(feed); }
};

Type Runner::info ("<runner>");

struct ArgRunner : Runner {
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

static auto fileRunner (Runner* feed) -> Value {
    assert(feed != nullptr);
    Value v = feed->next();
    if (v.isStr()) {
        auto data = vmImport(v);
        if (data != nullptr)
            v = new Bytes (data, 2000); // FIXME, length is wrong
    }
    return v;
}

static auto pyRunner (Runner* feed) -> Value {
    assert(feed != nullptr);
    Value v = feed->next();
    if (v.ifType<Bytes>() != nullptr) {
        auto task = vmLaunch(v.asType<Bytes>().begin());
        if (task != nullptr) {
            assert(Stacklet::current != nullptr);
            task->next();
            return 0;
        }
    }
    return v;
}

struct Command {
    char const* desc;
    void (*proc)();
};

static void printBuildVer () {
    printf("Monty %s (" __DATE__ ", " __TIME__ ")\n", VERSION);
}

static void helpCmd ();

static Command const commands [] = {
    { "bv    show build version"         , printBuildVer },
    { "gc    trigger garbage collection" , Stacklet::gcAll },
    { "gr    generate a GC report"       , gcReport },
    { "od    object dump"                , Obj::dumpAll },
    { "vd    vector dump"                , Vec::dumpAll },
    { "-h    this help"                  , helpCmd },
};

void helpCmd () {
    for (auto& cmd : commands)
        printf("  %s\n", cmd.desc);
}

static auto cmdRunner (Runner* feed) -> Value {
    assert(feed != nullptr);
    Value v = feed->next();
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

int main (int argc, char const** argv) {
    arch::init(10*1024);

    auto args = new ArgRunner (argc-1, argv+1);
    auto file = new Runner (fileRunner, args);
    auto py   = new Runner (pyRunner, file);
    auto cmd  = new Runner (cmdRunner, py);
    Pipeline pipe (cmd);

    Stacklet::ready.append(&pipe);

    while (Stacklet::runLoop()) {}

    arch::done();
}
