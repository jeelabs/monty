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

struct TestInterp : Interp {
    using Context::vm; // expose access to protected member;
};

static void vmInstance () {
    TEST_ASSERT(TestInterp::vm == 0);
    {
        Interp vm;
        TEST_ASSERT(TestInterp::vm == &vm);
    }
    TEST_ASSERT(TestInterp::vm == 0);
}

static void vmGcTrigger () {
    Interp vm;
    TEST_ASSERT(!vm.gcCheck());
    vm.gcTrigger();
    TEST_ASSERT(true);
}
