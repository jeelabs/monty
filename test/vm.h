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
