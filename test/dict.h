static void dictInstance () {
    DictObj obj;
    TEST_ASSERT_EQUAL(0, obj.len());
}

static void dictLookupGrow () {
    DictObj obj;

    TEST_ASSERT_TRUE(obj.atKey(123).isNil());

    Value& p1 = obj.atKey(11, obj.Set);
    TEST_ASSERT_TRUE(p1.isNil());
    p1 = 22;

    Value p2 = obj.atKey(11);
    TEST_ASSERT_FALSE(p2.isNil());
    TEST_ASSERT_EQUAL(22, (int) p2);
    TEST_ASSERT_TRUE(obj.atKey(123).isNil());
}
