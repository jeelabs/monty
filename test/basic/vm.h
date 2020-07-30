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
    vm.gcCheck();
    TEST_ASSERT(vm.nextPending().isNil());
    vm.gcTrigger();
    TEST_ASSERT(true);
}
