// Some generated utility functions, only used by the main ToyPy application.

void showAlignment () {
    static char a[1] = {'a'};
    static char b[1] = {'b'};
    printf("a %p b %p b-a %d\n", a, b, (int) ((uintptr_t) b -(uintptr_t) a));
    static const char c[1] = {'c'};
    static const char d[1] = {'d'};
    printf("c %p d %p c-d %d\n", c, d, (int) ((uintptr_t) d -(uintptr_t) c));
}

void showAllocInfo () {
    for (int i = 0; i < 25; ++i) {
        auto a = (char*) malloc(i);
        auto b = (char*) malloc(i);
        auto c = (char*) malloc(i);
        auto d = (char*) malloc(i);
        auto e = (char*) malloc(i);
        uint8_t f = (intptr_t)a|(intptr_t)b|(intptr_t)c|(intptr_t)d|(intptr_t)e;
        printf("malloc %2d: %6d %6d %6d %6d    ...%02x\n",
                i, (int)(b-a), (int)(c-b), (int)(d-c), (int)(e-d), f);
    }
}

void showObjSizes () {
    printf("%4d = sizeof int"          "\n", (int) sizeof (int));
    printf("%4d = sizeof long"         "\n", (int) sizeof (long));
    printf("%4d = sizeof void*"        "\n", (int) sizeof (void*));
    printf("%4d = sizeof Value"        "\n", (int) sizeof (Value));
    printf("%4d = sizeof Object"       "\n", (int) sizeof (Object));
    printf("%4d = sizeof Vector"       "\n", (int) sizeof (Vector));
    printf("%4d = sizeof IntObj"       "\n", (int) sizeof (IntObj));
    printf("%4d = sizeof BytesObj"     "\n", (int) sizeof (BytesObj));
    printf("%4d = sizeof StrObj"       "\n", (int) sizeof (StrObj));
    printf("%4d = sizeof ForceObj"     "\n", (int) sizeof (ForceObj));
    printf("%4d = sizeof TypeObj"      "\n", (int) sizeof (TypeObj));
    printf("%4d = sizeof FunObj"       "\n", (int) sizeof (FunObj));
    printf("%4d = sizeof MethObj"      "\n", (int) sizeof (MethObj));
    printf("%4d = sizeof MethodBase"   "\n", (int) sizeof (MethodBase));
    printf("%4d = sizeof BoundMethObj" "\n", (int) sizeof (BoundMethObj));
    printf("%4d = sizeof SeqObj"       "\n", (int) sizeof (SeqObj));
    printf("%4d = sizeof TupleObj"     "\n", (int) sizeof (TupleObj));
    printf("%4d = sizeof LookupObj"    "\n", (int) sizeof (LookupObj));
    printf("%4d = sizeof MutSeqObj"    "\n", (int) sizeof (MutSeqObj));
    printf("%4d = sizeof ArrayObj"     "\n", (int) sizeof (ArrayObj));
    printf("%4d = sizeof ListObj"      "\n", (int) sizeof (ListObj));
    printf("%4d = sizeof DictObj"      "\n", (int) sizeof (DictObj));
    printf("%4d = sizeof ClassObj"     "\n", (int) sizeof (ClassObj));
    printf("%4d = sizeof InstanceObj"  "\n", (int) sizeof (InstanceObj));
    printf("%4d = sizeof ModuleObj"    "\n", (int) sizeof (ModuleObj));
    printf("%4d = sizeof BytecodeObj"  "\n", (int) sizeof (BytecodeObj));
    printf("%4d = sizeof ResumableObj" "\n", (int) sizeof (ResumableObj));
    printf("%4d = sizeof FrameObj"     "\n", (int) sizeof (FrameObj));
    printf("%4d = sizeof Context"      "\n", (int) sizeof (Context));
    printf("%4d = sizeof Interp"       "\n", (int) sizeof (Interp));
}
