#include "monty.h"
#include "arch.h"

using namespace monty;

auto monty::vmImport (char const* name) -> uint8_t const* {
    return arch::importer(name); // TODO get rid of this
}

#define SIZEOF(name) printf("%5d b  %s\n", (int) sizeof (struct name), #name);

int main () {
    arch::init();
#if !NATIVE
    for (int i = 0; i < 10000000; ++i) asm (""); // brief delay
#endif

    SIZEOF(Array)
    SIZEOF(Bool)
    SIZEOF(BoundMeth)
    SIZEOF(Buffer)
    SIZEOF(Bytes)
    SIZEOF(Cell)
    SIZEOF(Class)
    SIZEOF(Closure)
    SIZEOF(Dict)
    SIZEOF(DictView)
    SIZEOF(Event)
    SIZEOF(Exception)
    SIZEOF(Exception)
    SIZEOF(Inst)
    SIZEOF(Int)
    SIZEOF(Iterator)
    SIZEOF(List)
    SIZEOF(Method)
    SIZEOF(MethodBase)
    SIZEOF(Module)
    SIZEOF(None)
    SIZEOF(Obj)
    SIZEOF(Object)
    SIZEOF(Range)
    SIZEOF(Set)
    SIZEOF(Slice)
    SIZEOF(Stacklet)
    SIZEOF(Str)
    SIZEOF(Super)
    SIZEOF(Tuple)
    SIZEOF(Type)
    SIZEOF(Value)
    SIZEOF(VaryVec)
    SIZEOF(Vec)

    return 0;
}
